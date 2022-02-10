// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTypes.h"
#include "SmartObjectDefinition.h"
#include "MassSmartObjectBehaviorDefinition.generated.h"

class UMassEntitySubsystem;
class USmartObjectSubsystem;
struct FMassExecutionContext;
struct FDataFragment_Transform;
struct FMassSmartObjectUserFragment;

/**
 * Struct to pass around the required set of information to activate a mass behavior definition on a given entity.
 */
struct MASSSMARTOBJECTS_API FMassBehaviorEntityContext
{
	FMassBehaviorEntityContext() = delete;

	FMassBehaviorEntityContext(const FMassEntityHandle InEntity, const FDataFragment_Transform& InTransformFragment, FMassSmartObjectUserFragment& InSOUser, USmartObjectSubsystem& InSubsystem)
		: Entity(InEntity), TransformFragment(InTransformFragment), SOUser(InSOUser), Subsystem(InSubsystem)
	{}

	const FMassEntityHandle Entity;
	const FDataFragment_Transform& TransformFragment;
	FMassSmartObjectUserFragment& SOUser;
	USmartObjectSubsystem& Subsystem;
};

/**
 * Base class for MassAIBehavior definitions. This is the type of definitions that LW Entities queries will look for.
 * Definition subclass can parameterized its associated behavior by overriding method Activate.
 */
UCLASS(EditInlineNew)
class MASSSMARTOBJECTS_API USmartObjectMassBehaviorDefinition : public USmartObjectBehaviorDefinition
{
	GENERATED_BODY()

public:
	/**
	 * This virtual method allows subclasses to configure the LW Entity based on their
	 * parameters (e.g. Add new fragments)
	 */
	virtual void Activate(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context, const FMassBehaviorEntityContext& EntityContext) const;

	/**
	 * Indicates the amount of time the LW entity
	 * will execute its behavior when reaching the smart object.
	 */
	UPROPERTY(EditDefaultsOnly, Category = SmartObject)
	float UseTime;
};
