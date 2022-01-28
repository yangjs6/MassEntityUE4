// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SmartObjectTypes.h"
#include "Templates/SharedPointer.h"
#include "Math/GenericOctree.h"

typedef TSharedRef<struct FSmartObjectOctreeID, ESPMode::ThreadSafe> FSmartObjectOctreeIDSharedRef;

struct SMARTOBJECTSMODULE_API FSmartObjectOctreeID : public TSharedFromThis<FSmartObjectOctreeID, ESPMode::ThreadSafe>
{
	FOctreeElementId2 ID;
};

struct SMARTOBJECTSMODULE_API FSmartObjectOctreeElement
{
	FBoxCenterAndExtent Bounds;
	FSmartObjectID SmartObjectID;
	FSmartObjectOctreeIDSharedRef SharedOctreeID;

	FSmartObjectOctreeElement(const FBoxCenterAndExtent& Bounds, const FSmartObjectID& SmartObjectID, const FSmartObjectOctreeIDSharedRef& SharedOctreeID);
};

struct FSmartObjectOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	FORCEINLINE static const FBoxCenterAndExtent& GetBoundingBox(const FSmartObjectOctreeElement& Element)
	{
		return Element.Bounds;
	}

	FORCEINLINE static bool AreElementsEqual(const FSmartObjectOctreeElement& A, const FSmartObjectOctreeElement& B)
	{
		return A.SmartObjectID == B.SmartObjectID;
	}

	static void SetElementId(const FSmartObjectOctreeElement& Element, FOctreeElementId2 Id);
};

class FSmartObjectOctree : public TOctree2<FSmartObjectOctreeElement, FSmartObjectOctreeSemantics>
{
public:
	FSmartObjectOctree();
	FSmartObjectOctree(const FVector& Origin, float Radius);
	virtual ~FSmartObjectOctree();

	/** Add new node and initialize using SmartObject runtime data */
	void AddNode(const FBoxCenterAndExtent& Bounds, const FSmartObjectID& SmartObjectID, const FSmartObjectOctreeIDSharedRef& SharedOctreeID);
	
	/** Updates element bounds remove/add operation */
	void UpdateNode(const FOctreeElementId2& Id, const FBox& NewBounds);

	/** Remove node */
	void RemoveNode(const FOctreeElementId2& Id);
};