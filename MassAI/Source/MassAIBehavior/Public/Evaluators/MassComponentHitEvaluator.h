// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassComponentHitSubsystem.h"
#include "MassStateTreeTypes.h"
#include "MassComponentHitEvaluator.generated.h"

class UMassComponentHitSubsystem;

/**
 * Evaluator to extract last hit from the MassComponentHitSubsystem and expose it for tasks and transitions
 */

USTRUCT()
struct MASSAIBEHAVIOR_API FMassComponentHitEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Output)
	bool bGotHit = false;

	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassEntityHandle LastHitEntity;
};

USTRUCT(meta = (DisplayName = "Mass ComponentHit Eval"))
struct MASSAIBEHAVIOR_API FMassComponentHitEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassComponentHitEvaluatorInstanceData::StaticStruct(); }
	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;

	TStateTreeExternalDataHandle<UMassComponentHitSubsystem> ComponentHitSubsystemHandle;

	TStateTreeInstanceDataPropertyHandle<bool> GotHitHandle;
	TStateTreeInstanceDataPropertyHandle<FMassEntityHandle> LastHitEntityHandle;
};
