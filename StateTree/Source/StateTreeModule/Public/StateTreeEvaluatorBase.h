// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTypes.h"
#include "StateTreeEvaluatorBase.generated.h"

struct FStateTreeExecutionContext;

/**
 * Base struct of StateTree Evaluators.
 * Evaluators calculate and expose data to be used for decision making in a StateTree.
 */
USTRUCT()
struct STATETREEMODULE_API FStateTreeEvaluatorBase
{
	GENERATED_BODY()

	FStateTreeEvaluatorBase() = default;

	virtual ~FStateTreeEvaluatorBase() {}

	/** @return Struct that represents the runtime data of the evaluator. */
	virtual const UStruct* GetInstanceDataType() const { return nullptr; };

	/**
	 * Called when the StateTree asset is linked. Allows to resolve references to other StateTree data.
	 * @see TStateTreeExternalDataHandle
	 * @see TStateTreeInstanceDataPropertyHandle
	 * @param Linker Reference to the linker
	 * @return true if linking succeeded. 
	 */
	virtual bool Link(FStateTreeLinker& Linker) { return true; }

	/**
	 * Called when a new state is entered and evaluator is part of active states. The change type parameter describes if the evaluator's state
	 * was previously part of the list of active states (Sustained), or if it just became active (Changed).
	 * @param Context Reference to current execution context.
	 * @param ChangeType Describes the change type (Changed/Sustained).
	 * @param Transition Describes the states involved in the transition
	 */
	virtual void EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const {}

	/**
	 * Called when a current state is exited and evaluator is part of active states. The change type parameter describes if the evaluator's state
	 * will be active after the transition (Sustained), or if it will became inactive (Changed).
	 * @param Context Reference to current execution context.
	 * @param ChangeType Describes the change type (Changed/Sustained).
	 * @param Transition Describes the states involved in the transition
	 */
	virtual void ExitState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const {}

	/**
	 * Called Right after a state has been completed. StateCompleted is called in reverse order to allow to propagate state to Evaluators and Tasks that
	 * are executed earlier in the tree. Note that StateCompleted is not called if conditional transition changes the state.
	 * @param Context Reference to current execution context.
	 * @param CompletionStatus Describes the running status of the completed state (Succeeded/Failed).
	 * @param CompletedState Handle of the state that was completed.
	 */
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeHandle CompletedState) const {}
	
	/**
	 * Called when evaluator needs to be updated. EvalType describes if the tick happens during state tree tick when the evaluator is on active state (Tick),
	 * or during state selection process when the evaluator's state is visited while it's inactive (PreSelection).
	 * That is, type "Tick" means that the call happens between EnterState()/ExitState() pair, "PreSelection" is used otherwise.
	 * @param Context Reference to current execution context.
	 * @param EvalType Describes tick type.
	 * @param DeltaTime Time since last StateTree tick, or 0 if called during preselection.
	 */
	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const {}

#if WITH_GAMEPLAY_DEBUGGER
	virtual void AppendDebugInfoString(FString& DebugString, const FStateTreeExecutionContext& Context) const;
#endif // WITH_GAMEPLAY_DEBUGGER

	UPROPERTY(EditDefaultsOnly, Category = Evaluator, meta=(EditCondition = "false", EditConditionHides))
	FName Name;

	/** Property binding copy batch handle. */
	UPROPERTY()
	FStateTreeHandle BindingsBatch = FStateTreeHandle::Invalid;		// Property binding copy batch handle.

	/** The runtime data's data view index in the StateTreeExecutionContext, and source struct index in property binding. */
	UPROPERTY()
	uint16 DataViewIndex = 0;

	UPROPERTY()
	uint16 InstanceIndex = 0;

	UPROPERTY()
	uint8 bInstanceIsObject : 1;
};

/**
* Base class (namespace) for all common Evaluators that are generally applicable.
* This allows schemas to safely include all Evaluators child of this struct. 
*/
USTRUCT(Meta=(Hidden))
struct STATETREEMODULE_API FStateTreeEvaluatorCommonBase : public FStateTreeEvaluatorBase
{
	GENERATED_BODY()
};