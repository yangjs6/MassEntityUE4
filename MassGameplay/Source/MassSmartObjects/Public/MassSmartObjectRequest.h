// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SmartObjectTypes.h"
#include "MassEntityTypes.h"
#include "ZoneGraphTypes.h"
#include "Containers/StaticArray.h"
#include "MassSmartObjectRequest.generated.h"

/**
 * Structure that represents a potential smart object for a LW Entity during the search
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FSmartObjectCandidate
{
	GENERATED_BODY()

	FSmartObjectCandidate() = default;
	FSmartObjectCandidate(const FSmartObjectID InID, const float InCost) : ID(InID), Cost(InCost) {}

	UPROPERTY(Transient)
	FSmartObjectID ID;

	UPROPERTY(Transient)
	float Cost = 0.f;
};

/**
 * Struct that holds status and results of a candidate finder request
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FMassSmartObjectRequestResult
{
	GENERATED_BODY()

	static constexpr uint32 MaxNumCandidates = 4;
	TStaticArray<FSmartObjectCandidate, MaxNumCandidates> Candidates;

	UPROPERTY(Transient)
	uint8 NumCandidates = 0;

	UPROPERTY(Transient)
	bool bProcessed = false;
};

/**
 * Fragment that holds the result of a request to find candidates.
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FMassSmartObjectRequestResultFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FMassSmartObjectRequestResult Result;
};

/**
 * Fragment used to build a list potential smart objects to use. Once added to an entity
 * this will be processed by the candidates finder processor to fill a SmartObjectCandidates
 * fragment that could then be processed by the reservation processor
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FMassSmartObjectWorldLocationRequestFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector SearchOrigin = FVector::ZeroVector;

	UPROPERTY(Transient)
	FMassEntityHandle RequestingEntity;
};

/**
 * Fragment used to build a list potential smart objects to use. Once added to an entity
 * this will be processed by the candidates finder processor to fill a SmartObjectCandidates
 * fragment that could then be processed by the reservation processor
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FMassSmartObjectLaneLocationRequestFragment : public FMassFragment
{
	GENERATED_BODY()

	FZoneGraphCompactLaneLocation CompactLaneLocation;

	UPROPERTY(Transient)
	FMassEntityHandle RequestingEntity;
};

/**
 * Special tag to mark processed requests
 */
USTRUCT()
struct MASSSMARTOBJECTS_API FMassSmartObjectCompletedRequestTag : public FMassTag
{
	GENERATED_BODY()
};
