// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassLODLogic.h"
#include "DrawDebugHelpers.h"



/**
 * Helper struct to collect needed information on each agent that will be needed later for LOD calculation
 *   Requires TTransformFragment fragment.
 *   Stores information in TViewerInfoFragment fragment.
 */
template <typename FLODLogic = FLODDefaultLogic >
struct TMassLODCollector : public FMassLODBaseLogic
{
	/**
	 * Prepares execution for the current frame, needed to be called before every execution
	 * @Param Viewers is the array of all the known viewers
	 */
	void PrepareExecution(TConstArrayView<FViewerInfo> Viewers);

	/**
	 * Collects the information for LOD calculation, called for each entity chunks
	 * Use next method when FLODLogic::bStoreInfoPerViewer is enabled
	 * @Param Context of the chunk execution
	 * @Param TransformList is the fragment transforms of the entities
	 * @Param ViewersInfoList is the fragment where to store source information for LOD calculation
	 */
	template <typename TTransformFragment, 
			  typename TViewerInfoFragment,
			  bool bCollectLocalViewers = FLODLogic::bLocalViewersOnly,
			  bool bCollectDistanceToFrustum = FLODLogic::bDoVisibilityLogic>
	FORCEINLINE void CollectLODInfo(FMassExecutionContext& Context, 
									TConstArrayView<TTransformFragment> TransformList, 
									TArrayView<TViewerInfoFragment> ViewersInfoList)
	{
		CollectLODInfo<TTransformFragment,
			TViewerInfoFragment,
			/*TPerViewerInfoFragment*/void*,
			bCollectLocalViewers,
			bCollectDistanceToFrustum,
			/*bCollectDistancePerViewer*/false,
			/*bCollectDistanceToFrustumPerViewer*/false>(Context, TransformList, ViewersInfoList, TArrayView<void*>());
	}


	/**
	 * Collects the information for LOD calculation, called for each entity chunks
	 * Use this version when FLODLogic::bStoreInfoPerViewer is enabled
	 * It collects information per viewer into the PerViewerInfoList fragments
	 * @Param Context of the chunk execution
	 * @Param TransformList is the fragment transforms of the entities
	 * @Param ViewersInfoList is the fragment where to store source information for LOD calculation
	 * @Param PerViewerInfoList is the per viewer information
	 */
	template <typename TTransformFragment,
			  typename TViewerInfoFragment,
			  typename TPerViewerInfoFragment,
			  bool bCollectLocalViewers = FLODLogic::bLocalViewersOnly,
			  bool bCollectDistanceToFrustum = FLODLogic::bDoVisibilityLogic,
			  bool bCollectDistancePerViewer = FLODLogic::bStoreInfoPerViewer,
			  bool bCollectDistanceToFrustumPerViewer = FLODLogic::bDoVisibilityLogic && FLODLogic::bStoreInfoPerViewer>
	void CollectLODInfo(FMassExecutionContext& Context, 
						TConstArrayView<TTransformFragment> TransformList, 
						TArrayView<TViewerInfoFragment> ViewersInfoList, 
						TArrayView<TPerViewerInfoFragment> PerViewerInfoList);
};

template <typename FLODLogic>
void TMassLODCollector<FLODLogic>::PrepareExecution(TConstArrayView<FViewerInfo> ViewersInfo)
{
	CacheViewerInformation(ViewersInfo);
}


inline float DistanceToVolume(const FConvexVolume& Volume,const FVector& Point)
{
	checkSlow(PermutedPlanes.Num() % 4 == 0);

	VectorRegister VMinimumDistance = MakeVectorRegister(-BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER);

	// Load the origin & radius
	VectorRegister VPoint = VectorLoadFloat3(&Point);
	VectorRegister VMinDistance = VMinimumDistance;
	// Splat point into 3 vectors
	VectorRegister VPointX = VectorReplicate(VPoint, 0);
	VectorRegister VPointY = VectorReplicate(VPoint, 1);
	VectorRegister VPointZ = VectorReplicate(VPoint, 2);
	// Since we are moving straight through get a pointer to the data
	const FPlane* RESTRICT PermutedPlanePtr = (FPlane*)Volume.PermutedPlanes.GetData();
	// Process four planes at a time until we have < 4 left
	for (int32 Count = 0; Count < Volume.PermutedPlanes.Num(); Count += 4)
	{
		// Load 4 planes that are already all Xs, Ys, ...
		VectorRegister PlanesX = VectorLoadAligned(PermutedPlanePtr);
		PermutedPlanePtr++;
		VectorRegister PlanesY = VectorLoadAligned(PermutedPlanePtr);
		PermutedPlanePtr++;
		VectorRegister PlanesZ = VectorLoadAligned(PermutedPlanePtr);
		PermutedPlanePtr++;
		VectorRegister PlanesW = VectorLoadAligned(PermutedPlanePtr);
		PermutedPlanePtr++;
		// Calculate the distance (x * x) + (y * y) + (z * z) - w
		VectorRegister DistX = VectorMultiply(VPointX, PlanesX);
		VectorRegister DistY = VectorMultiplyAdd(VPointY, PlanesY, DistX);
		VectorRegister DistZ = VectorMultiplyAdd(VPointZ, PlanesZ, DistY);
		VectorRegister Distance = VectorSubtract(DistZ, PlanesW);

		VMinDistance = VectorMax(Distance, VMinDistance);
	}

	const VectorRegister VMinDistanceWXYZ = VectorSwizzle(VMinDistance, 3, 0, 1, 2);
	const VectorRegister t0 = VectorMax(VMinDistance, VMinDistanceWXYZ);
	const VectorRegister VMinDistanceZWXY = VectorSwizzle(VMinDistance, 2, 3, 0, 1);
	const VectorRegister t1 = VectorMax(t0, VMinDistanceZWXY);
	const VectorRegister VMinDistanceYZWX = VectorSwizzle(VMinDistance, 1, 2, 3, 0);
	const VectorRegister t2 = VectorMax(t1, VMinDistanceYZWX);

	float MinDistance;
	VectorStoreFloat1(t2, &MinDistance);
	return MinDistance;
}


template <typename FLODLogic>
template <typename TTransformFragment, 
		  typename TViewerInfoFragment, 
		  typename TPerViewerInfoFragment,
		  bool bCollectLocalViewers,
		  bool bCollectDistanceToFrustum,
		  bool bCollectDistancePerViewer,
		  bool bCollectDistanceToFrustumPerViewer>
void TMassLODCollector<FLODLogic>::CollectLODInfo(FMassExecutionContext& Context, 
												  TConstArrayView<TTransformFragment> TransformList, 
												  TArrayView<TViewerInfoFragment> ViewersInfoList, 
												  TArrayView<TPerViewerInfoFragment> PerViewerInfoList)
{
	static TPerViewerInfoFragment DummyFragment;
	const int32 NumEntities = Context.GetNumEntities();
	for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
	{
		float ClosestViewerDistanceSq = FLT_MAX;
		float ClosestDistanceToFrustum = FLT_MAX;
		const TTransformFragment& EntityTransform = TransformList[EntityIdx];
		TViewerInfoFragment& EntityViewerInfo = ViewersInfoList[EntityIdx];
		TPerViewerInfoFragment& EntityInfoPerViewer = FLODLogic::bStoreInfoPerViewer ? PerViewerInfoList[EntityIdx] : DummyFragment;

		SetDistanceToViewerSqNum<bCollectDistancePerViewer>(EntityInfoPerViewer, Viewers.Num());
		SetDistanceToFrustumNum<bCollectDistanceToFrustumPerViewer>(EntityInfoPerViewer, Viewers.Num());
		for (int ViewerIdx = 0; ViewerIdx < Viewers.Num(); ++ViewerIdx)
		{
			const FViewerLODInfo& Viewer = Viewers[ViewerIdx];
			if (Viewer.bClearData)
			{
				SetDistanceToViewerSq<bCollectDistancePerViewer>(EntityInfoPerViewer, ViewerIdx, FLT_MAX);
				SetDistanceToFrustum<bCollectDistanceToFrustumPerViewer>(EntityInfoPerViewer, ViewerIdx, FLT_MAX);
			}

			// Check to see if we want only local viewer only
			if (bCollectLocalViewers && !Viewer.bLocal)
			{
				continue;
			}

			if (Viewer.Handle.IsValid())
			{
				const FVector& EntityLocation = EntityTransform.GetTransform().GetLocation();
				const FVector ViewerToEntity = EntityLocation - Viewer.Location;
				const float DistanceToViewerSq = ViewerToEntity.SizeSquared();
				if (ClosestViewerDistanceSq > DistanceToViewerSq)
				{
					ClosestViewerDistanceSq = DistanceToViewerSq;
				}
				SetDistanceToViewerSq<bCollectDistancePerViewer>(EntityInfoPerViewer, ViewerIdx, DistanceToViewerSq);

				if (bCollectDistanceToFrustum)
				{
					const float DistanceToFrustum = DistanceToVolume(Viewer.Frustum, EntityLocation);
					SetDistanceToFrustum<bCollectDistanceToFrustumPerViewer>(EntityInfoPerViewer, ViewerIdx, DistanceToFrustum);
					if (ClosestDistanceToFrustum > DistanceToFrustum)
					{
						ClosestDistanceToFrustum = DistanceToFrustum;
					}
				}
			}
		}
		EntityViewerInfo.ClosestViewerDistanceSq = ClosestViewerDistanceSq;
		SetClosestDistanceToFrustum<bCollectDistanceToFrustum>(EntityViewerInfo, ClosestDistanceToFrustum);
	}
}