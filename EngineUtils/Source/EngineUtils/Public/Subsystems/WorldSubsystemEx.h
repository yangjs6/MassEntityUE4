// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "WorldSubsystemEx.generated.h"

/**
* UWorldSubsystemEx
*/
UCLASS()
class ENGINEUTILS_API UWorldSubsystemEx : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UWorldSubsystemEx() {}
	
	UWorld& GetWorldRef();
	
	virtual void PostInitialize() {}

	virtual void OnWorldComponentsUpdated(UWorld& World) {}

	virtual void OnWorldBeginPlay(UWorld& World) {}
};

/**
 * UTickableWorldSubsystem
 * Base class for auto instanced and initialized systems that share the lifetime of a UWorld and are ticking along with it
 */
UCLASS(Abstract)
class ENGINEUTILS_API UTickableWorldSubsystem : public UWorldSubsystemEx, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UTickableWorldSubsystem();

	// FTickableGameObject implementation Begin
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsAllowedToTick() const /*override*/ final;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override PURE_VIRTUAL(UTickableWorldSubsystem::GetStatId, return TStatId(););
	// FTickableGameObject implementation End

	// USubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// USubsystem implementation End

	bool IsInitialized() const { return bInitialized; }

private:
	bool bInitialized = false;
};