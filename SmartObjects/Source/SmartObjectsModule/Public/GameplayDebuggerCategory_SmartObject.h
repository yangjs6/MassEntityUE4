// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SmartObjectTypes.h"

#if WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG

#include "GameplayDebuggerCategory.h"

class APlayerController;
class AActor;

class FGameplayDebuggerCategory_SmartObject : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_SmartObject();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;


	UWorld* GetWorldFromReplicator() const;
	/** Returns the first non-null world associated with, in order: OwnerPC, DebugActor, AGameplayDebuggerCategoryReplicator owner */
	UWorld* GetDataWorld(const APlayerController* OwnerPC, const AActor* DebugActor) const;

	/**
	* Returns view location and direction from replicated data if available or will extract it from
	* the provided player controller. The replicated data is used first since it might be provided
	* from a detached camera.
	* @return whether OutViewLocation and OutViewDirection were assigned to valid values
	* @note Method is expected to always return valid outputs if valid controller is provided. Otherwise it
	* depends if the view information has been replicated before the methods gets called.
	*/
	bool GetViewPoint(const APlayerController* OwnerPC, FVector& OutViewLocation, FVector& OutViewDirection) const;

	/**
	* Indicates if a given location is within a vision cone built from provided view location and direction based
	* on MaxViewDistance and MaxViewAngle from GameplayDebuggerUserSettings
	*/
	static bool IsLocationInViewCone(const FVector& ViewLocation, const FVector& ViewDirection, const FVector& TargetLocation);

};

#endif // WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
