// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayDebuggerCategory_SmartObject.h"

#include "GameplayDebuggerCategoryReplicator.h"
#include "GameplayDebuggerPlayerManager.h"
#include "SmartObjectSubsystem.h"
#include "Engine/DebugCameraController.h"
#include "Math/ColorList.h"
#include "Engine/World.h"

#if WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG

FGameplayDebuggerCategory_SmartObject::FGameplayDebuggerCategory_SmartObject()
{
	bShowOnlyWithDebugActor = false;
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_SmartObject::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_SmartObject());
}

void FGameplayDebuggerCategory_SmartObject::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	UWorld* World = GetDataWorld(OwnerPC, DebugActor);
	check(World);

	USmartObjectSubsystem* Subsystem = World->GetSubsystem<USmartObjectSubsystem>();
	if (Subsystem == nullptr)
	{
		AddTextLine(FString::Printf(TEXT("{Red}SmartObjectSubsystem instance is missing")));
		return;
	}

	const uint32 NumRuntimeObjects = Subsystem->DebugGetNumRuntimeObjects();
	const uint32 NumRegisteredComponents = Subsystem->DebugGetNumRegisteredComponents();

	ASmartObjectCollection* MainCollection = Subsystem->GetMainCollection();
	const uint32 NumCollectionEntries = MainCollection != nullptr ? MainCollection->GetEntries().Num() : 0;
	AddTextLine(FString::Printf(TEXT("{White}Collection entries = {Green}%d\n{White}Runtime objects = {Green}%s\n{White}Registered components = {Green}%s"),
		NumCollectionEntries, *LexToString(NumRuntimeObjects), *LexToString(NumRegisteredComponents)));

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	bool bApplyCulling = GetViewPoint(OwnerPC, ViewLocation, ViewDirection);

	FColor DebugColor = FColor::Yellow;

	const FColor FreeColor = FColorList::Grey;
	const FColor ClaimedColor = FColor::Yellow;
	const FColor OccupiedColor = FColor::Red;

	const TMap<FSmartObjectID, FSmartObjectRuntime>& Entries = Subsystem->DebugGetRuntimeObjects();
	for (auto& LookupEntry : Entries)
	{
		const FSmartObjectRuntime& Entry = LookupEntry.Value;
		const FTransform LocalToWorld = Entry.GetTransform();
		const USmartObjectDefinition& Definition = Entry.GetDefinition();

		const FVector Location = LocalToWorld.GetLocation();
		if (bApplyCulling && !IsLocationInViewCone(ViewLocation, ViewDirection, Location))
		{
			continue;
		}

		constexpr float DebugReferencePointRadius = 10.f;
		AddShape(FGameplayDebuggerShape::MakePoint(Location, DebugReferencePointRadius, FColorList::Grey));

		for (int32 i = 0; i < Definition.GetSlots().Num(); ++i)
		{
			constexpr float DebugArrowThickness = 2.f;
			constexpr float DebugCircleRadius = 40.f;
			constexpr float DebugArrowHeadSize = 10.f;
			TOptional<FTransform> Transform = Definition.GetSlotTransform(LocalToWorld, FSmartObjectSlotIndex(i));
			if (!Transform.IsSet())
			{
				continue;
			}
#if WITH_EDITORONLY_DATA
			DebugColor = Definition.GetSlots()[i].DEBUG_DrawColor;
#endif
			const FVector Pos = Transform.GetValue().GetLocation() + FVector(0.0f ,0.0f ,25.0f );
			const FVector Dir = Transform.GetValue().GetRotation().GetForwardVector();

			const ESmartObjectSlotState State = Entry.GetSlotState(i);
			FColor StateColor = FColor::Silver;
			switch (State)
			{
			case ESmartObjectSlotState::Free:		StateColor = FreeColor;		break;
			case ESmartObjectSlotState::Claimed:	StateColor = ClaimedColor;	break;
			case ESmartObjectSlotState::Occupied:	StateColor = OccupiedColor;	break;
			default:
				ensureMsgf(false, TEXT("Unsupported value: %s"), *UEnum::GetValueAsString(State));
			}

			AddShape(FGameplayDebuggerShape::MakeSegment(Location, Pos, /* Thickness */1.f, FColorList::Grey));
			//AddShape(FGameplayDebuggerShape::MakeCircle(Pos, FVector::UpVector, DebugCircleRadius, DebugColor));
			//AddShape(FGameplayDebuggerShape::MakeCircle(Pos, FVector::UpVector, 0.75f*DebugCircleRadius, /* Thickness */5.f, StateColor));
			//AddShape(FGameplayDebuggerShape::MakeArrow(Pos, Pos + Dir * 2.0f * DebugCircleRadius, DebugArrowHeadSize, DebugArrowThickness, DebugColor));
		}
	}
}


bool FGameplayDebuggerCategory_SmartObject::GetViewPoint(const APlayerController* OwnerPC, FVector& OutViewLocation, FVector& OutViewDirection) const
{
	if (OwnerPC == nullptr)
	{
		return false;
	}

	UWorld* World = OwnerPC->GetWorld();
	FVector CameraLocation;
	FRotator CameraRotation;
	if (OwnerPC->Player)
	{
		// normal game
		OwnerPC->GetPlayerViewPoint(CameraLocation, CameraRotation);
	}
	else
	{
		// spectator mode
		for (FLocalPlayerIterator It(GEngine, World); It; ++It)
		{
			ADebugCameraController* SpectatorPC = Cast<ADebugCameraController>(It->PlayerController);
			if (SpectatorPC)
			{
				SpectatorPC->GetPlayerViewPoint(CameraLocation, CameraRotation);
				break;
			}
		}
	}

	OutViewLocation = CameraLocation;
	OutViewDirection = CameraRotation.Vector();
	
	return true;
}

bool FGameplayDebuggerCategory_SmartObject::IsLocationInViewCone(const FVector& ViewLocation, const FVector& ViewDirection, const FVector& TargetLocation)
{
	const float MaxViewDistance = 25000.0f;
	const float MaxViewAngle = 45.f;
	
	const FVector DirToEntity = TargetLocation - ViewLocation;
	const float DistanceToEntitySq = DirToEntity.Size();
	if (DistanceToEntitySq > MaxViewDistance)
	{
		return false;
	}

	const float ViewDot = FVector::DotProduct(DirToEntity.GetSafeNormal(), ViewDirection);
	const float MinViewDirDot = FMath::Cos(FMath::DegreesToRadians(MaxViewAngle));
	if (ViewDot < MinViewDirDot)
	{
		return false;
	}

	return true;
}

UWorld* FGameplayDebuggerCategory_SmartObject::GetWorldFromReplicator() const
{
	AGameplayDebuggerCategoryReplicator* Replicator = GetReplicator();
	return Replicator ? Replicator->GetWorld() : nullptr;
}

UWorld* FGameplayDebuggerCategory_SmartObject::GetDataWorld(const APlayerController* OwnerPC, const AActor* DebugActor) const
{
	// We're trying OwnerPC first because that's the main owner for what's being displayed by gameplay debugger
	// but it editor-only mode there are no player controllers, so we try the DebugActor, which can be picked
	// also in the editor-mode by selecting an actor on the level.
	// finally, DebugActor can be null as well, when no actor is selected, when we fetch the world which the
	// AGameplayDebuggerCategoryReplicator instance owning this debugger addon belongs to.
	UWorld* World = OwnerPC ? OwnerPC->GetWorld() : nullptr;
	if (World == nullptr && DebugActor != nullptr)
	{
		World = DebugActor->GetWorld();
	}
	return World ? World : GetWorldFromReplicator();
}

#endif // WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
