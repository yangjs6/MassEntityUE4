// Copyright Epic Games, Inc. All Rights Reserved.

#include "Elements/Framework/TypedElementList.h"
#include "Elements/Framework/TypedElementRegistry.h"

namespace TypedElementList_Private
{

void GetElementImpl(const UTypedElementRegistry* InRegistry, const FTypedElementHandle& InElementHandle, const TSubclassOf<UInterface>& InBaseInterfaceType, FTypedElement& OutElement)
{
	if (InRegistry)
	{
		InRegistry->Private_GetElementImpl(InElementHandle, InBaseInterfaceType, OutElement);
	}
	else
	{
		OutElement.Release();
	}
}

} // namespace TypedElementList_Private


FTypedElementListLegacySync::FTypedElementListLegacySync(const FTypedElementList& InElementList)
	: ElementList(InElementList)
{
}

FTypedElementListLegacySync::FOnSyncEvent& FTypedElementListLegacySync::OnSyncEvent()
{
	return OnSyncEventDelegate;
}

void FTypedElementListLegacySync::Private_EmitSyncEvent(const ESyncType InSyncType, const FTypedElementHandle& InElementHandle)
{
	const bool bIsWithinBatchOperation = IsRunningBatchOperation();
	bBatchOperationIsDirty |= bIsWithinBatchOperation;
	OnSyncEventDelegate.Broadcast(ElementList, InSyncType, InElementHandle, bIsWithinBatchOperation);
}

bool FTypedElementListLegacySync::IsRunningBatchOperation() const
{
	return NumOpenBatchOperations > 0;
}

void FTypedElementListLegacySync::BeginBatchOperation()
{
	++NumOpenBatchOperations;
}

void FTypedElementListLegacySync::EndBatchOperation(const bool InNotify)
{
	checkf(NumOpenBatchOperations > 0, TEXT("Batch operation underflow!"));

	if (--NumOpenBatchOperations == 0)
	{
		const bool bNotifyChange = bBatchOperationIsDirty && InNotify;
		bBatchOperationIsDirty = false;

		if (bNotifyChange)
		{
			Private_EmitSyncEvent(ESyncType::BatchComplete);
			check(!bBatchOperationIsDirty); // This should still be false after emitting the notification!
		}
	}
}

bool FTypedElementListLegacySync::IsBatchOperationDirty() const
{
	return bBatchOperationIsDirty;
}

void FTypedElementListLegacySync::ForceBatchOperationDirty()
{
	if (NumOpenBatchOperations > 0)
	{
		bBatchOperationIsDirty = true;
	}
}


FTypedElementListLegacySyncScopedBatch::FTypedElementListLegacySyncScopedBatch(const FTypedElementList& InElementList, const bool InNotify)
	: ElementListLegacySync(InElementList.Legacy_GetSyncPtr())
	, bNotify(InNotify)
{
	if (ElementListLegacySync)
	{
		ElementListLegacySync->BeginBatchOperation();
	}
}

FTypedElementListLegacySyncScopedBatch::~FTypedElementListLegacySyncScopedBatch()
{
	if (ElementListLegacySync)
	{
		ElementListLegacySync->EndBatchOperation(bNotify);
	}
}

bool FTypedElementListLegacySyncScopedBatch::IsDirty() const
{
	return ElementListLegacySync
		&& ElementListLegacySync->IsBatchOperationDirty();
}

void FTypedElementListLegacySyncScopedBatch::ForceDirty()
{
	if (ElementListLegacySync)
	{
		ElementListLegacySync->ForceBatchOperationDirty();
	}
}


FTypedElementListRef FTypedElementList::Private_CreateElementList(UTypedElementRegistry* InRegistry)
{
	return MakeShareable(new FTypedElementList(InRegistry));
}

FTypedElementList::FTypedElementList(UTypedElementRegistry* InRegistry)
	: Registry(InRegistry)
{
	checkf(InRegistry, TEXT("Registry is null!"));

	ElementCounts.Initialize(InRegistry);

	InRegistry->Private_OnElementListCreated(this);
}

FTypedElementList::~FTypedElementList()
{
	Empty();
	LegacySync.Reset();
	if (UTypedElementRegistry* RegistryPtr = Registry.Get())
	{
		RegistryPtr->Private_OnElementListDestroyed(this);
		Registry = nullptr;
	}
}

FTypedElementListRef FTypedElementList::Clone() const
{
	FTypedElementListRef ClonedElementList = Private_CreateElementList(Registry.Get());
	for (const FTypedElementHandle& ElementHandle : ElementHandles)
	{
		ClonedElementList->Add(ElementHandle);
	}
	return ClonedElementList;
}

UObject* FTypedElementList::GetElementInterface(const FTypedElementHandle& InElementHandle, const TSubclassOf<UInterface>& InBaseInterfaceType) const
{
	UTypedElementRegistry* RegistryPtr = Registry.Get();
	return RegistryPtr
		? Registry->GetElementInterface(InElementHandle, InBaseInterfaceType)
		: nullptr;
}

bool FTypedElementList::HasElements(const TSubclassOf<UInterface>& InBaseInterfaceType) const
{
	bool bHasFilteredElements = false;

	if (InBaseInterfaceType)
	{
		ForEachElementHandle([&bHasFilteredElements](const FTypedElementHandle&)
		{
			bHasFilteredElements = true;
			return false;
		}, InBaseInterfaceType);
	}
	else
	{
		bHasFilteredElements = Num() > 0;
	}

	return bHasFilteredElements;
}

int32 FTypedElementList::CountElements(const TSubclassOf<UInterface>& InBaseInterfaceType) const
{
	int32 NumFilteredElements = 0;

	if (InBaseInterfaceType)
	{
		ForEachElementHandle([&NumFilteredElements](const FTypedElementHandle&)
		{
			++NumFilteredElements;
			return true;
		}, InBaseInterfaceType);
	}
	else
	{
		NumFilteredElements = Num();
	}

	return NumFilteredElements;
}

bool FTypedElementList::HasElementsOfType(const FName InElementTypeName) const
{
	return CountElementsOfType(InElementTypeName) > 0;
}

bool FTypedElementList::HasElementsOfType(const FTypedHandleTypeId InElementTypeId) const
{
	return CountElementsOfType(InElementTypeId) > 0;
}

int32 FTypedElementList::CountElementsOfType(const FName InElementTypeName) const
{
	if (UTypedElementRegistry* RegistryPtr = Registry.Get())
	{
		const FTypedHandleTypeId ElementTypeId = RegistryPtr->GetRegisteredElementTypeId(InElementTypeName);
		if (ElementTypeId > 0)
		{
			return CountElementsOfType(ElementTypeId);
		}
	}
	return 0;
}

int32 FTypedElementList::CountElementsOfType(const FTypedHandleTypeId InElementTypeId) const
{
	return ElementCounts.GetCounterValue(FTypedElementCounter::GetElementTypeCategoryName(), InElementTypeId);
}

TArray<FTypedElementHandle> FTypedElementList::GetElementHandles(const TSubclassOf<UInterface>& InBaseInterfaceType) const
{
	TArray<FTypedElementHandle> FilteredElementHandles;
	FilteredElementHandles.Reserve(ElementHandles.Num());

	ForEachElementHandle([&FilteredElementHandles](const FTypedElementHandle& InElementHandle)
	{
		FilteredElementHandles.Add(InElementHandle);
		return true;
	}, InBaseInterfaceType);

	return FilteredElementHandles;
}

void FTypedElementList::ForEachElementHandle(TFunctionRef<bool(const FTypedElementHandle&)> InCallback, const TSubclassOf<UInterface>& InBaseInterfaceType) const
{
	for (const FTypedElementHandle& ElementHandle : ElementHandles)
	{
		if (ElementHandle && (!InBaseInterfaceType || GetElementInterface(ElementHandle, InBaseInterfaceType)))
		{
			if (!InCallback(ElementHandle))
			{
				break;
			}
		}
	}
}

bool FTypedElementList::AddElementImpl(FTypedElementHandle&& InElementHandle)
{
	if (!InElementHandle)
	{
		return false;
	}

	NoteListMayChange();

	bool bAlreadyAdded = false;
	ElementCombinedIds.Add(InElementHandle.GetId().GetCombinedId(), &bAlreadyAdded);

	if (!bAlreadyAdded)
	{
		const FTypedElementHandle& AddedElementHandle = ElementHandles.Add_GetRef(MoveTemp(InElementHandle));
		ElementCounts.AddElement(AddedElementHandle);
		NoteListChanged(EChangeType::Added, AddedElementHandle);
	}

	return !bAlreadyAdded;
}

bool FTypedElementList::RemoveElementImpl(const FTypedElementId& InElementId)
{
	if (!InElementId)
	{
		return false;
	}

	NoteListMayChange();

	const bool bRemoved = ElementCombinedIds.Remove(InElementId.GetCombinedId()) > 0;

	if (bRemoved)
	{
		const int32 ElementHandleIndexToRemove = ElementHandles.IndexOfByPredicate([&InElementId](const FTypedElementHandle& InElementHandle)
		{
			return InElementHandle.GetId() == InElementId;
		});
		checkSlow(ElementHandleIndexToRemove != INDEX_NONE);

		FTypedElementHandle RemovedElementHandle = MoveTemp(ElementHandles[ElementHandleIndexToRemove]);
		ElementHandles.RemoveAt(ElementHandleIndexToRemove, 1, /*bAllowShrinking*/false);
		ElementCounts.RemoveElement(RemovedElementHandle);

		NoteListChanged(EChangeType::Removed, RemovedElementHandle);
	}

	return bRemoved;
}

int32 FTypedElementList::RemoveAllElementsImpl(TFunctionRef<bool(const FTypedElementHandle&)> InPredicate)
{
	int32 RemovedCount = 0;

	if (ElementHandles.Num() > 0)
	{
		FTypedElementListLegacySyncScopedBatch LegacySyncBatch(*this);

		NoteListMayChange();

		for (int32 Index = ElementHandles.Num() - 1; Index >= 0; --Index)
		{
			if (InPredicate(ElementHandles[Index]))
			{
				FTypedElementHandle RemovedElementHandle = MoveTemp(ElementHandles[Index]);
				ElementCombinedIds.Remove(RemovedElementHandle.GetId().GetCombinedId());
				ElementHandles.RemoveAt(Index, 1, /*bAllowShrinking*/false);
				ElementCounts.RemoveElement(RemovedElementHandle);

				NoteListChanged(EChangeType::Removed, RemovedElementHandle);

				++RemovedCount;
			}
		}
	}

	return RemovedCount;
}

bool FTypedElementList::ContainsElementImpl(const FTypedElementId& InElementId) const
{
	return InElementId 
		&& ElementCombinedIds.Contains(InElementId.GetCombinedId());
}

FTypedElementListLegacySync& FTypedElementList::Legacy_GetSync()
{
	if (!LegacySync)
	{
		LegacySync = MakeUnique<FTypedElementListLegacySync>(*this);
	}
	return *LegacySync;
}

FTypedElementListLegacySync* FTypedElementList::Legacy_GetSyncPtr() const
{
	return LegacySync.Get();
}

bool FTypedElementList::NotifyPendingChanges()
{
	if (bHasPendingNotify)
	{
		bHasPendingNotify = false;
		OnChangedDelegate.Broadcast(*this);
		return true;
	}

	return false;
}

FTypedElementList::FScopedClearNewPendingChange::FScopedClearNewPendingChange(FTypedElementList& InTypeElementList)
{
	bool bCanClearNewPendingChange = !InTypeElementList.bHasPendingNotify;

	if (InTypeElementList.LegacySync)
	{
		bCanClearNewPendingChange &= !InTypeElementList.LegacySync->IsRunningBatchOperation();
	}
	
	if (bCanClearNewPendingChange)
	{
		TypedElementList = &InTypeElementList;
	}
}


FTypedElementList::FScopedClearNewPendingChange::FScopedClearNewPendingChange(FScopedClearNewPendingChange&& Other)
	: TypedElementList(Other.TypedElementList)
{
	Other.TypedElementList = nullptr;
}

FTypedElementList::FScopedClearNewPendingChange& FTypedElementList::FScopedClearNewPendingChange::operator=(FScopedClearNewPendingChange&& Other)
{
	TypedElementList = Other.TypedElementList;
	Other.TypedElementList = nullptr;

	return *this;
}

FTypedElementList::FScopedClearNewPendingChange::~FScopedClearNewPendingChange()
{
	if (TypedElementList)
	{
		TypedElementList->bHasPendingNotify = false;
	}
}

FTypedElementList::FScopedClearNewPendingChange FTypedElementList::GetScopedClearNewPendingChange()
{
	return FScopedClearNewPendingChange(*this);
}

void FTypedElementList::NoteListMayChange()
{
	OnPreChangeDelegate.Broadcast(*this);
}

void FTypedElementList::NoteListChanged(const EChangeType InChangeType, const FTypedElementHandle& InElementHandle)
{
	bHasPendingNotify = true;

	if (LegacySync)
	{
		FTypedElementListLegacySync::ESyncType SyncType = FTypedElementListLegacySync::ESyncType::Modified;
		switch (InChangeType)
		{
		case EChangeType::Added:
			SyncType = FTypedElementListLegacySync::ESyncType::Added;
			break;

		case EChangeType::Removed:
			SyncType = FTypedElementListLegacySync::ESyncType::Removed;
			break;

		case EChangeType::Cleared:
			SyncType = FTypedElementListLegacySync::ESyncType::Cleared;
			break;

		default:
			break;
		}

		LegacySync->Private_EmitSyncEvent(SyncType, InElementHandle);
	}
}
