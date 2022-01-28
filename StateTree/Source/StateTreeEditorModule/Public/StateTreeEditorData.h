// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "StateTreeState.h"
#include "StateTreeEditorPropertyBindings.h"
#include "StateTreeEditorData.generated.h"

UENUM()
enum class EStateTreeItemType : uint8
{
	EnterCondition,
	Evaluator,
	Task,
	TransitionCondition,
};

/**
 * Edit time data for StateTree asset. This data gets baked into runtime format before being used by the StateTreeInstance.
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories)
class STATETREEEDITORMODULE_API UStateTreeEditorData : public UObject, public IStateTreeEditorPropertyBindingsOwner
{
	GENERATED_BODY()
public:
	// IStateTreeEditorPropertyBindingsOwner
	virtual void GetAccessibleStructs(const FGuid TargetStructID, TArray<FStateTreeBindableStructDesc>& OutStructDescs) const override;
	virtual bool GetStructByID(const FGuid StructID, FStateTreeBindableStructDesc& OutStructDesc) const override;
	virtual FStateTreeEditorPropertyBindings* GetPropertyEditorBindings() override { return &EditorBindings; }
	// ~IStateTreeEditorPropertyBindingsOwner

	/** Returns parent state of a struct, or nullptr if not found. */
	const UStateTreeState* GetStateByStructID(const FGuid TargetStructID) const;

	/** Gets the IDs of all bindable structs in the StateTree. */
	void GetAllStructIDs(TMap<FGuid, const UStruct*>& AllStructs) const;

	/**
	 * Iterates over all structs that are related to binding
	 * @param InFunc function called at each item, should return true if visiting is continued or false to stop.
	 */
	void VisitHierarchy(TFunctionRef<bool(const UStateTreeState& State, const FGuid& ID, const FName& Name, const EStateTreeItemType ItemType, const UScriptStruct* ItemStruct, const UStruct* InstanceStruct)> InFunc) const;

	// StateTree Builder API

	/**
	 * Adds new Subtree with specified name.
	 * @return Pointer to the new Subtree.
	 */
	UStateTreeState& AddSubTree(const FName Name)
	{
		UStateTreeState* SubTreeState = NewObject<UStateTreeState>(this);
		check(SubTreeState);
		SubTreeState->Name = Name;
		Routines.Add(SubTreeState);
		return *SubTreeState;
	}

	/**
	 * Adds property binding between two structs.
	 */
	void AddPropertyBinding(const FStateTreeEditorPropertyPath& SourcePath, const FStateTreeEditorPropertyPath& TargetPath)
	{
		EditorBindings.AddPropertyBinding(SourcePath, TargetPath);
	}
	// ~StateTree Builder API

	
	UPROPERTY()
	FStateTreeEditorPropertyBindings EditorBindings;

	// Top level States aka Routines.
	UPROPERTY()
	TArray<UStateTreeState*> Routines;
};
