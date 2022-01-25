// Copyright Epic Games, Inc. All Rights Reserved.

#include "Elements/Interfaces/TypedElementSelectionInterface.h"
#include "Elements/Framework/TypedElementList.h"

bool ITypedElementSelectionInterface::IsElementSelected(const FTypedElementHandle& InElementHandle, const FTypedElementListProxy InSelectionSet, const FTypedElementIsSelectedOptions& InSelectionOptions)
{
	FTypedElementListConstPtr SelectionSetPtr = InSelectionSet.GetElementList();
	return SelectionSetPtr && SelectionSetPtr->Contains(InElementHandle);
}

bool ITypedElementSelectionInterface::SelectElement(const FTypedElementHandle& InElementHandle, FTypedElementListProxy InSelectionSet, const FTypedElementSelectionOptions& InSelectionOptions)
{
	FTypedElementListPtr SelectionSetPtr = InSelectionSet.GetElementList();
	return SelectionSetPtr && SelectionSetPtr->Add(InElementHandle);
}

bool ITypedElementSelectionInterface::DeselectElement(const FTypedElementHandle& InElementHandle, FTypedElementListProxy InSelectionSet, const FTypedElementSelectionOptions& InSelectionOptions)
{
	FTypedElementListPtr SelectionSetPtr = InSelectionSet.GetElementList();
	return SelectionSetPtr && SelectionSetPtr->Remove(InElementHandle);
}
