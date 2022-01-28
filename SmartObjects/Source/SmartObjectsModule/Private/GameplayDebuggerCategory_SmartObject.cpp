﻿// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayDebuggerCategory_SmartObject.h"
#include "SmartObjectSubsystem.h"
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
			AddShape(FGameplayDebuggerShape::MakeCircle(Pos, FVector::UpVector, DebugCircleRadius, DebugColor));
			AddShape(FGameplayDebuggerShape::MakeCircle(Pos, FVector::UpVector, 0.75f*DebugCircleRadius, /* Thickness */5.f, StateColor));
			AddShape(FGameplayDebuggerShape::MakeArrow(Pos, Pos + Dir * 2.0f * DebugCircleRadius, DebugArrowHeadSize, DebugArrowThickness, DebugColor));
		}
	}
}

#endif // WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
