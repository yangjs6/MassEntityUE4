// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssignRandomNavLocationProcessor.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavAgentInterface.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

//----------------------------------------------------------------------//
// UAssignRandomNavLocationProcessor 
//----------------------------------------------------------------------//
UAssignRandomNavLocationProcessor::UAssignRandomNavLocationProcessor()
{
	bAutoRegisterWithProcessingPhases = false;
}

void UAssignRandomNavLocationProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FDataFragment_NavLocation>(EMassFragmentAccess::ReadWrite);
}

void UAssignRandomNavLocationProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	if (!ensure(NavigationSystem))
	{
		return;
	}

	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
		{
			TArrayView<FDataFragment_Transform> LocationList = Context.GetMutableFragmentView<FDataFragment_Transform>();
			TArrayView<FDataFragment_NavLocation> NavLocationList = Context.GetMutableFragmentView<FDataFragment_NavLocation>();
			const int32 NumEntities = Context.GetNumEntities();
			TArray<FNavLocation> Locations;
			Locations.AddDefaulted(NumEntities);
			for (FNavLocation& Location : Locations)
			{
				NavigationSystem->GetRandomReachablePointInRadius(Origin, Radius, Location);
			}

			// apply locations
			for (int32 Index = 0; Index < NumEntities; ++Index)
			{
				LocationList[Index].GetMutableTransform().SetLocation(Locations[Index].Location);
				NavLocationList[Index].NodeRef = Locations[Index].NodeRef;
			}
		});
}

void UAssignRandomNavLocationProcessor::Initialize(UObject& InOwner)
{
	Super::Initialize(InOwner);

	if (INavAgentInterface* AsNavAgent = Cast<INavAgentInterface>(&InOwner))
	{
		Origin = AsNavAgent->GetNavAgentLocation();
	}
	else if (AActor* AsActor = Cast<AActor>(&InOwner))
	{
		Origin = AsActor->GetActorLocation();
	}
	else
	{
		// @todo add logging
	}

	NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InOwner.GetWorld());
	UE_CLOG(NavigationSystem == nullptr, LogMass, Error, TEXT("UAssignRandomNavLocationProcessor used while no NavigationSystem present"));
}
