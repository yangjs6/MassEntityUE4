// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassSmartObjectProcessor.h"
#include "MassCommandBuffer.h"
#include "MassCommonTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "MassSmartObjectSettings.h"
#include "SmartObjectZoneAnnotations.h"
#include "Misc/ScopeExit.h"
#include "SmartObjectOctree.h"
#include "SmartObjectSubsystem.h"
#include "VisualLogger/VisualLogger.h"
#include "ZoneGraphAnnotationSubsystem.h"
#include "ZoneGraphSubsystem.h"

//----------------------------------------------------------------------//
// UMassProcessor_SmartObjectBase
//----------------------------------------------------------------------//
void UMassProcessor_SmartObjectBase::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(GetWorld());
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
}

//----------------------------------------------------------------------//
// UMassProcessor_SmartObjectCandidatesFinder
//----------------------------------------------------------------------//
void UMassProcessor_SmartObjectCandidatesFinder::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	AnnotationSubsystem = UWorld::GetSubsystem<UZoneGraphAnnotationSubsystem>(GetWorld());
	ZoneGraphSubsystem = UWorld::GetSubsystem<UZoneGraphSubsystem>(GetWorld());
}

void UMassProcessor_SmartObjectCandidatesFinder::ConfigureQueries()
{
	WorldRequestQuery.AddRequirement<FMassSmartObjectWorldLocationRequestFragment>(EMassFragmentAccess::ReadOnly);
	WorldRequestQuery.AddRequirement<FMassSmartObjectRequestResultFragment>(EMassFragmentAccess::ReadWrite);
	WorldRequestQuery.AddTagRequirement<FMassSmartObjectCompletedRequestTag>(EMassFragmentPresence::None);

	LaneRequestQuery.AddRequirement<FMassSmartObjectLaneLocationRequestFragment>(EMassFragmentAccess::ReadOnly);
	LaneRequestQuery.AddRequirement<FMassSmartObjectRequestResultFragment>(EMassFragmentAccess::ReadWrite);
	LaneRequestQuery.AddTagRequirement<FMassSmartObjectCompletedRequestTag>(EMassFragmentPresence::None);
}

UMassProcessor_SmartObjectCandidatesFinder::UMassProcessor_SmartObjectCandidatesFinder()
{
	// 1. Frame T Behavior create a request(deferred entity creation)
	// 2. Frame T+1: Processor execute the request might mark it as done(deferred add tag flushed at the end of the frame)
	// 3. Frame T+1: Behavior could cancel request(deferred destroy entity)
	// If the processor does not run before the behaviors, step 2 and 3 are flipped and it will crash while flushing the deferred commands
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UMassProcessor_SmartObjectCandidatesFinder::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	checkf(SmartObjectSubsystem != nullptr, TEXT("SmartObjectSubsystem should exist when executing processors."));
	checkf(SignalSubsystem != nullptr, TEXT("MassSignalSubsystem should exist when executing processors."));
	checkf(ZoneGraphSubsystem != nullptr, TEXT("ZoneGraphSubsystem should exist when executing processors."));
	checkf(AnnotationSubsystem != nullptr, TEXT("ZoneGraphAnnotationSubsystem should exist when executing processors."));

	// Create filter
	FSmartObjectRequestFilter Filter;
	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();

	// Build list of request owner entities to send a completion signal
	TArray<FMassEntityHandle> EntitiesToSignal;

	auto BeginRequestProcessing = [](const FMassEntityHandle Entity, FMassExecutionContext& Context, FMassSmartObjectRequestResult& Result)
	{
		Context.Defer().AddTag<FMassSmartObjectCompletedRequestTag>(Entity);
		Result.NumCandidates = 0;
	};

	auto EndRequestProcessing = [](const UObject* LogOwner, const FMassEntityHandle Entity, FMassSmartObjectRequestResult& Result)
	{
		if (Result.NumCandidates > 0)
		{
			TArrayView<FSmartObjectCandidate> View = MakeArrayView(Result.Candidates.GetData(), Result.NumCandidates);
			Algo::Sort(View, [](const FSmartObjectCandidate& LHS, const FSmartObjectCandidate& RHS) { return LHS.Cost < RHS.Cost; });
		}
		Result.bProcessed = true;

#if WITH_MASSGAMEPLAY_DEBUG
		UE_VLOG(LogOwner, LogSmartObject, Verbose, TEXT("[%s] search completed: found %d"), *Entity.DebugGetDescription(), Result.NumCandidates);
#endif // WITH_MASSGAMEPLAY_DEBUG
	};

	// Process world location based requests
	WorldRequestQuery.ForEachEntityChunk(EntitySubsystem, Context, [this, &Filter, &EntitiesToSignal, &BeginRequestProcessing, &EndRequestProcessing](FMassExecutionContext& Context)
	{
		const FSmartObjectOctree& Octree = SmartObjectSubsystem->GetOctree();

		const int32 NumEntities = Context.GetNumEntities();
		EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);

		const TConstArrayView<FMassSmartObjectWorldLocationRequestFragment> RequestList = Context.GetFragmentView<FMassSmartObjectWorldLocationRequestFragment>();
		const TArrayView<FMassSmartObjectRequestResultFragment> ResultList = Context.GetMutableFragmentView<FMassSmartObjectRequestResultFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			const FMassSmartObjectWorldLocationRequestFragment& RequestFragment = RequestList[i];
			FMassSmartObjectRequestResult& Result = ResultList[i].Result;
			EntitiesToSignal.Add(RequestFragment.RequestingEntity);

			const FVector& SearchOrigin = RequestFragment.SearchOrigin;
			const FBox& SearchBounds = FBox::BuildAABB(SearchOrigin, FVector(SearchExtents));

			const FMassEntityHandle Entity = Context.GetEntity(i);
			bool bDisplayDebug = false;
			FColor DebugColor(FColor::White);

#if WITH_MASSGAMEPLAY_DEBUG
			bDisplayDebug = UE::Mass::Debug::IsDebuggingEntity(Entity, &DebugColor);
#endif // WITH_MASSGAMEPLAY_DEBUG

			BeginRequestProcessing(Entity, Context, Result);
			ON_SCOPE_EXIT{ EndRequestProcessing(SmartObjectSubsystem, Entity, Result);	};

			Octree.FindElementsWithBoundsTest(SearchBounds, [this, &Filter, &Entity, &Result, &SearchOrigin, &bDisplayDebug, &DebugColor](const FSmartObjectOctreeElement& Element)
			{
				if (Result.NumCandidates < FMassSmartObjectRequestResult::MaxNumCandidates)
				{
					const FSmartObjectID ID = Element.SmartObjectID;

					// Make sure that we can use a slot in that object (availability with supported definitions, etc.)
					if (SmartObjectSubsystem->FindSlot(ID, Filter).IsValid())
					{
						const FVector ObjectLocation = Element.Bounds.Center;
						Result.Candidates[Result.NumCandidates++] = FSmartObjectCandidate(ID, FVector::DistSquared(SearchOrigin, ObjectLocation));

#if WITH_MASSGAMEPLAY_DEBUG
						if (bDisplayDebug)
						{
							constexpr float DebugRadius = 10.f;
							UE_VLOG_LOCATION(SmartObjectSubsystem, LogSmartObject, Display, ObjectLocation, DebugRadius, DebugColor, TEXT("%s"), *ID.Describe());
							UE_VLOG_SEGMENT(SmartObjectSubsystem, LogSmartObject, Display, SearchOrigin, ObjectLocation, DebugColor, TEXT(""));
						}
#endif // WITH_MASSGAMEPLAY_DEBUG
					}
				}
			});
		}
	});

	// Process lane based requests
	const FZoneGraphTag SmartObjectTag = GetDefault<UMassSmartObjectSettings>()->SmartObjectTag;
	USmartObjectZoneAnnotations* Annotations = Cast<USmartObjectZoneAnnotations>(AnnotationSubsystem->GetFirstAnnotationForTag(SmartObjectTag));

	LaneRequestQuery.ForEachEntityChunk(EntitySubsystem, Context,
		[this, Annotations, &Filter, SmartObjectTag, &EntitiesToSignal, &BeginRequestProcessing, &EndRequestProcessing](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			EntitiesToSignal.Reserve(EntitiesToSignal.Num() + NumEntities);

			TConstArrayView<FMassSmartObjectLaneLocationRequestFragment> RequestList = Context.GetFragmentView<FMassSmartObjectLaneLocationRequestFragment>();
			TArrayView<FMassSmartObjectRequestResultFragment> ResultList = Context.GetMutableFragmentView<FMassSmartObjectRequestResultFragment>();

			// Cache latest used data since request are most of the time on the same zone graph
			FZoneGraphDataHandle LastUsedDataHandle;
			const FSmartObjectAnnotationData* GraphData = nullptr;

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FMassSmartObjectLaneLocationRequestFragment& RequestFragment = RequestList[i];
				FMassSmartObjectRequestResult& Result = ResultList[i].Result;
				EntitiesToSignal.Add(RequestFragment.RequestingEntity);

				const FZoneGraphCompactLaneLocation RequestLocation = RequestFragment.CompactLaneLocation;
				const FZoneGraphLaneHandle RequestLaneHandle = RequestLocation.LaneHandle;

				const FMassEntityHandle Entity = Context.GetEntity(i);
				bool bDisplayDebug = false;
#if WITH_MASSGAMEPLAY_DEBUG
				FColor DebugColor(FColor::White);
				bDisplayDebug = UE::Mass::Debug::IsDebuggingEntity(Entity, &DebugColor);
#endif // WITH_MASSGAMEPLAY_DEBUG

				BeginRequestProcessing(Entity, Context, Result);
				ON_SCOPE_EXIT{ EndRequestProcessing(SmartObjectSubsystem, Entity, Result); };

				if (!ensureMsgf(RequestLaneHandle.IsValid(), TEXT("Requesting smart objects using an invalid handle")))
				{
					continue;
				}

				if (Annotations == nullptr)
				{
					UE_VLOG(SmartObjectSubsystem, LogSmartObject, Warning, TEXT("%d lane location based requests failed since SmartObject annotations are not available"), NumEntities);
					return;
				}

				// Fetch smart object data associated to the current graph if different than last used one
				if (LastUsedDataHandle != RequestLaneHandle.DataHandle)
				{
					LastUsedDataHandle = RequestLaneHandle.DataHandle;
					GraphData = Annotations->GetAnnotationData(RequestLaneHandle.DataHandle);
				}

				if (GraphData == nullptr)
				{
					continue;
				}

				// Fetch current annotations for the specified lane and look for the smart object tag
				const FZoneGraphTagMask LaneMask = AnnotationSubsystem->GetAnnotationTags(RequestLaneHandle);
				if (!LaneMask.Contains(SmartObjectTag))
				{
					continue;
				}

				const FSmartObjectList* SmartObjectList = GraphData->LaneToSmartObjectsLookup.Find(RequestLaneHandle.Index);
				if (SmartObjectList == nullptr || !ensureMsgf(SmartObjectList->SmartObjects.Num() > 0, TEXT("Lookup table should only contains lanes with one or more associated object(s).")))
				{
					continue;
				}

				for (const FSmartObjectID ID : SmartObjectList->SmartObjects)
				{
					// Find entry point using FindChecked since all smart objects added to LaneToSmartObjects lookup table
					// were also added to the entry point lookup table
					const FSmartObjectLaneLocation EntryPoint = GraphData->ObjectToEntryPointLookup.FindChecked(ID);

					float Cost = 0.f;
					if (ensureMsgf(EntryPoint.LaneIndex == RequestLocation.LaneHandle.Index, TEXT("Must be on same lane to be able to use distance along lane.")))
					{
						// Only consider object ahead
						const float DistAhead = EntryPoint.DistanceAlongLane - RequestLocation.DistanceAlongLane;
						if (DistAhead < 0)
						{
							continue;
						}
						Cost = DistAhead;
					}

					// Make sure that we can use a slot in that object (availability with supported definitions, etc.)
					if (!SmartObjectSubsystem->FindSlot(ID, Filter).IsValid())
					{
						continue;
					}

					Result.Candidates[Result.NumCandidates++] = FSmartObjectCandidate(ID, Cost);

#if WITH_MASSGAMEPLAY_DEBUG
					if (bDisplayDebug)
					{
						FZoneGraphLaneLocation RequestLaneLocation, EntryPointLaneLocation;
						ZoneGraphSubsystem->CalculateLocationAlongLane(RequestLaneHandle, RequestLocation.DistanceAlongLane, RequestLaneLocation);
						ZoneGraphSubsystem->CalculateLocationAlongLane(RequestLaneHandle, EntryPoint.DistanceAlongLane, EntryPointLaneLocation);

						constexpr float DebugRadius = 10.f;
						UE_VLOG_LOCATION(SmartObjectSubsystem, LogSmartObject, Display, EntryPointLaneLocation.Position, DebugRadius, DebugColor, TEXT("%s"), *ID.Describe());
						UE_VLOG_SEGMENT(SmartObjectSubsystem, LogSmartObject, Display, RequestLaneLocation.Position, EntryPointLaneLocation.Position, DebugColor, TEXT(""));
					}
#endif // WITH_MASSGAMEPLAY_DEBUG

					if (Result.NumCandidates == FMassSmartObjectRequestResult::MaxNumCandidates)
					{
						break;
					}
				}
			}
		});

	// Signal entities that their search results are ready
	if (EntitiesToSignal.Num())
	{
		SignalSubsystem->SignalEntities(UE::Mass::Signals::SmartObjectCandidatesReady, EntitiesToSignal);
	}
}

//----------------------------------------------------------------------//
// UMassProcessor_SmartObjectTimedBehavior
//----------------------------------------------------------------------//
void UMassProcessor_SmartObjectTimedBehavior::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassSmartObjectUserFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassSmartObjectTimedBehaviorTag>(EMassFragmentPresence::All);
}

UMassProcessor_SmartObjectTimedBehavior::UMassProcessor_SmartObjectTimedBehavior()
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UMassProcessor_SmartObjectTimedBehavior::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	checkf(SmartObjectSubsystem != nullptr, TEXT("SmartObjectSubsystem should exist when executing processors."));
	checkf(SignalSubsystem != nullptr, TEXT("MassSignalSubsystem should exist when executing processors."));

	TArray<TTuple<FMassEntityHandle, FMassSmartObjectUserFragment*>> ToRelease;

	QUICK_SCOPE_CYCLE_COUNTER(UMassProcessor_SmartObjectTestBehavior_Run);

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this, &ToRelease](FMassExecutionContext& Context)
	{
		const int32 NumEntities = Context.GetNumEntities();
		const TArrayView<FMassSmartObjectUserFragment> UserList = Context.GetMutableFragmentView<FMassSmartObjectUserFragment>();

		for (int32 i = 0; i < NumEntities; ++i)
		{
			FMassSmartObjectUserFragment& SOUser = UserList[i];
			if (SOUser.InteractionStatus != EMassSmartObjectInteractionStatus::InProgress)
			{
				// Only expecting in progress interaction status
				continue;
			}

			const float DT = Context.GetDeltaTimeSeconds();
			SOUser.SetUseTime(FMath::Max(SOUser.GetUseTime() - DT, 0.0f));
			const bool bMustRelease = SOUser.GetUseTime() <= 0.f;

#if WITH_MASSGAMEPLAY_DEBUG
			const FMassEntityHandle Entity = Context.GetEntity(i);
			FColor DebugColor(FColor::White);
			const bool bIsDebuggingEntity = UE::Mass::Debug::IsDebuggingEntity(Entity, &DebugColor);
			if (bIsDebuggingEntity)
			{
				UE_CVLOG(bMustRelease, SmartObjectSubsystem, LogSmartObject, Log, TEXT("[%s] stops using [%s]"), *Entity.DebugGetDescription(), *SOUser.GetClaimHandle().Describe());
				UE_CVLOG(!bMustRelease, SmartObjectSubsystem, LogSmartObject, Verbose, TEXT("[%s] using [%s] for %.1f"), *Entity.DebugGetDescription(), *SOUser.GetClaimHandle().Describe(), SOUser.GetUseTime());

				const TOptional<FTransform> Transform = SmartObjectSubsystem->GetSlotTransform(SOUser.ClaimHandle);
				if (Transform.IsSet())
				{
					constexpr float Radius = 40.f;
					const FVector HalfHeightOffset(0.f, 0.f, 100.f);
					const FVector Pos = Transform.GetValue().GetLocation();
					const FVector Dir = Transform.GetValue().GetRotation().GetForwardVector();
					UE_VLOG_CYLINDER(SmartObjectSubsystem, LogSmartObject, Display, Pos - HalfHeightOffset, Pos + HalfHeightOffset, Radius, DebugColor, TEXT(""));
					UE_VLOG_ARROW(SmartObjectSubsystem, LogSmartObject, Display, Pos, Pos + Dir * 2.0f * Radius, DebugColor, TEXT(""));
				}
			}
#endif // WITH_MASSGAMEPLAY_DEBUG

			if (bMustRelease)
			{
				ToRelease.Add(MakeTuple(Context.GetEntity(i), &SOUser));
			}
		}
	});

	for (const TTuple<FMassEntityHandle, FMassSmartObjectUserFragment*> EntryToRelease : ToRelease)
	{
		const FMassEntityHandle Entity = EntryToRelease.Get<0>();
		SignalSubsystem->SignalEntity(UE::Mass::Signals::SmartObjectInteractionDone, Entity);

		FMassSmartObjectUserFragment* User = EntryToRelease.Get<1>();
		SmartObjectSubsystem->Release(User->ClaimHandle);
		User->InteractionStatus = EMassSmartObjectInteractionStatus::Completed;
		Context.Defer().RemoveTag<FMassSmartObjectTimedBehaviorTag>(Entity);
		User->ClaimHandle.Invalidate();
	}
}
