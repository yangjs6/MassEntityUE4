// Copyright Epic Games, Inc.All Rights Reserved.

#include "MassActorHelper.h"
#include "MassActorTypes.h"
#include "MassActorSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

namespace UE::MassActor
{
	bool AddEntityTagToActor(const AActor& Actor, const UScriptStruct& TagType)
	{
		UWorld* World = Actor.GetWorld();
		UMassEntitySubsystem* EntitySystem = UWorld::GetSubsystem<UMassEntitySubsystem>(World);
		UMassActorSubsystem* MassActorSubsystem = UWorld::GetSubsystem<UMassActorSubsystem>(World);
		if (EntitySystem && MassActorSubsystem)
		{
			const FMassEntityHandle AgentHandle = MassActorSubsystem->GetEntityHandleFromActor(&Actor);
			if (AgentHandle.IsValid())
			{
				EntitySystem->AddTagToEntity(AgentHandle, &TagType);
				return true;
			}
			UE_LOG(LogMassActor, Warning, TEXT("Failed to add tag %s to actor %s due to it not having an associated entity")
				, *TagType.GetName(), *Actor.GetName());
		}
		else
		{
			UE_LOG(LogMassActor, Warning, TEXT("Failed to add tag %s to actor %s due to missing: %s%s")
				, *TagType.GetName(), *Actor.GetName(), EntitySystem ? TEXT("EntitySubsystem, ") : TEXT("")
				, MassActorSubsystem ? TEXT("MassActorSubsystem, ") : TEXT(""));
		}
		return false;
	}

	bool RemoveEntityTagFromActor(const AActor& Actor, const UScriptStruct& TagType)
	{
		UWorld* World = Actor.GetWorld();
		UMassEntitySubsystem* EntitySystem = UWorld::GetSubsystem<UMassEntitySubsystem>(World);
		UMassActorSubsystem* MassActorSubsystem = UWorld::GetSubsystem<UMassActorSubsystem>(World);
		if (EntitySystem && MassActorSubsystem)
		{
			const FMassEntityHandle AgentHandle = MassActorSubsystem->GetEntityHandleFromActor(&Actor);
			if (AgentHandle.IsValid())
			{
				EntitySystem->RemoveTagFromEntity(AgentHandle, &TagType);
				return true;
			}
			UE_LOG(LogMassActor, Warning, TEXT("Failed to remove tag %s from actor %s due to it not having an associated entity")
				, *TagType.GetName(), *Actor.GetName());
		}
		else
		{
			UE_LOG(LogMassActor, Warning, TEXT("Failed to remove tag %s from actor %s due to missing: %s%s")
				, *TagType.GetName(), *Actor.GetName(), EntitySystem ? TEXT("EntitySubsystem, ") : TEXT("")
				, MassActorSubsystem ? TEXT("MassActorSubsystem, ") : TEXT(""));
		}
		return false;
	}
} // namespace UE::MassActor
