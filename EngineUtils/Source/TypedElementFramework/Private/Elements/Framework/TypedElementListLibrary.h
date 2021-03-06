// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Elements/Framework/TypedElementRegistry.h"
#include "Elements/Framework/TypedElementListProxy.h"
#include "Elements/Framework/TypedElementList.h"
#include "TypedElementListLibrary.generated.h"

UCLASS()
class UTypedElementListLibrary : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Create an empty list of elements associated with the given registry.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|Registry", meta=(ScriptMethod))
	static FTypedElementListProxy CreateElementList(UTypedElementRegistry* Registry)
	{
		return Registry->CreateElementList();
	}

	/**
	 * Clone this list instance.
	 * @note Only copies elements; does not copy any bindings!
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static FTypedElementListProxy Clone(const FTypedElementListProxy ElementList)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->Clone() : FTypedElementListProxy();
	}

	/**
	 * Get the element handle at the given index.
	 * @note Use IsValidIndex to test for validity.
	 */
	UFUNCTION(BlueprintPure, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static FTypedElementHandle GetElementHandleAt(const FTypedElementListProxy ElementList, const int32 Index)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->GetElementHandleAt(Index) : FTypedElementHandle();
	}

	/**
	 * Get the element interface from the given handle.
	 */
	UFUNCTION(BlueprintPure, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static UObject* GetElementInterface(const FTypedElementListProxy ElementList, const FTypedElementHandle& ElementHandle, const TSubclassOf<UInterface> BaseInterfaceType)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->GetElementInterface(ElementHandle, BaseInterfaceType) : nullptr;
	}

	/**
	 * Test whether there are elements in this list, optionally filtering to elements that implement the given interface.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool HasElements(const FTypedElementListProxy ElementList, const TSubclassOf<UInterface> BaseInterfaceType = nullptr)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->HasElements(BaseInterfaceType);
	}

	/**
	 * Count the number of elements in this list, optionally filtering to elements that implement the given interface.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static int32 CountElements(const FTypedElementListProxy ElementList, const TSubclassOf<UInterface> BaseInterfaceType = nullptr)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->CountElements(BaseInterfaceType) : 0;
	}

	/**
	 * Test whether there are elements in this list of the given type.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool HasElementsOfType(const FTypedElementListProxy ElementList, const FName ElementTypeName)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->HasElementsOfType(ElementTypeName);
	}

	/**
	 * Count the number of elements in this list of the given type.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static int32 CountElementsOfType(const FTypedElementListProxy ElementList, const FName ElementTypeName)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->CountElementsOfType(ElementTypeName) : 0;
	}

	/**
	 * Get the handle of every element in this list, optionally filtering to elements that implement the given interface.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static TArray<FTypedElementHandle> GetElementHandles(const FTypedElementListProxy ElementList, const TSubclassOf<UInterface> BaseInterfaceType)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->GetElementHandles(BaseInterfaceType) : TArray<FTypedElementHandle>();
	}

	/**
	 * Is the given index a valid entry within this element list?
	 */
	UFUNCTION(BlueprintPure, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool IsValidIndex(const FTypedElementListProxy ElementList, const int32 Index)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->IsValidIndex(Index);
	}

	/**
	 * Get the number of entries within this element list.
	 */
	UFUNCTION(BlueprintPure, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static int32 Num(const FTypedElementListProxy ElementList)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr ? ElementListPtr->Num() : 0;
	}

	/**
	 * Shrink this element list storage to avoid slack.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void Shrink(FTypedElementListProxy ElementList)
	{
		if (FTypedElementListPtr ElementListPtr = ElementList.GetElementList())
		{
			ElementListPtr->Shrink();
		}
	}

	/**
	 * Pre-allocate enough memory in this element list to store the given number of entries.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void Reserve(FTypedElementListProxy ElementList, const int32 Size)
	{
		if (FTypedElementListPtr ElementListPtr = ElementList.GetElementList())
		{
			ElementListPtr->Reserve(Size);
		}
	}

	/**
	 * Remove all entries from this element list, potentially leaving space allocated for the given number of entries.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void Empty(FTypedElementListProxy ElementList, const int32 Slack = 0)
	{
		if (FTypedElementListPtr ElementListPtr = ElementList.GetElementList())
		{
			ElementListPtr->Empty(Slack);
		}
	}

	/**
	 * Remove all entries from this element list, preserving existing allocations.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void Reset(FTypedElementListProxy ElementList)
	{
		if (FTypedElementListPtr ElementListPtr = ElementList.GetElementList())
		{
			ElementListPtr->Reset();
		}
	}

	/**
	 * Does this element list contain an entry for the given element handle?
	 */
	UFUNCTION(BlueprintPure, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool Contains(const FTypedElementListProxy ElementList, const FTypedElementHandle& ElementHandle)
	{
		FTypedElementListConstPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->Contains(ElementHandle);
	}

	/**
	 * Add the given element handle to this element list, if it isn't already in the list.
	 * @return True if the element handle was added, false if it is already in the list.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool Add(FTypedElementListProxy ElementList, const FTypedElementHandle& ElementHandle)
	{
		FTypedElementListPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->Add(ElementHandle);
	}

	/**
	 * Append the given element handles to this element list.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void Append(FTypedElementListProxy ElementList, const TArray<FTypedElementHandle>& ElementHandles)
	{
		if (FTypedElementListPtr ElementListPtr = ElementList.GetElementList())
		{
			ElementListPtr->Append(ElementHandles);
		}
	}

	/**
	 * Append the another element list to this element list.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static void AppendList(FTypedElementListProxy ElementList, const FTypedElementListProxy OtherElementList)
	{
		FTypedElementListPtr ElementListPtr = ElementList.GetElementList();
		FTypedElementListConstPtr OtherElementListPtr = OtherElementList.GetElementList();
		if (ElementListPtr && OtherElementListPtr)
		{
			ElementListPtr->Append(OtherElementListPtr.ToSharedRef());
		}
	}

	/**
	 * Remove the given element handle from this element list, if it is in the list.
	 * @return True if the element handle was removed, false if it isn't in the list.
	 */
	UFUNCTION(BlueprintCallable, Category="TypedElementFramework|List", meta=(ScriptMethod))
	static bool Remove(FTypedElementListProxy ElementList, const FTypedElementHandle& ElementHandle)
	{
		FTypedElementListPtr ElementListPtr = ElementList.GetElementList();
		return ElementListPtr && ElementListPtr->Remove(ElementHandle);
	}
};
