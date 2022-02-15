// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassCommonTypes.h"
#include "MassAIMovementTypes.h"
#include "MassZoneGraphMovementTypes.generated.h"

/** Describes path request along one ZoneGraph lane. If the NextLaneHandle is set, lane is changed when path finishes. */
USTRUCT()
struct MASSAIMOVEMENT_API FZoneGraphShortPathRequest
{
	GENERATED_BODY()

	FZoneGraphShortPathRequest()
		: bMoveReverse(false)
		, bIsEndOfPathPositionSet(false)
		, bIsEndOfPathDirectionSet(false)
	{
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%s to distance %.1f Next lane: %s of type %s. End of path intent:%s"),
			bMoveReverse ? TEXT("forward") : TEXT("reverse"), TargetDistance,
			NextLaneHandle.IsValid() ? *NextLaneHandle.ToString() : TEXT("unset"),
			NextLaneHandle.IsValid() ? *UEnum::GetValueAsString(NextExitLinkType) : TEXT("Unset"),
			*UEnum::GetValueAsString(EndOfPathIntent));
	}

	/** Position used as the start of the path*/
	UPROPERTY(Transient)
	FVector StartPosition = FVector::ZeroVector;

	/** Optional specific point at the end of the path. */
	UPROPERTY(Transient)
	FVector EndOfPathPosition = FVector::ZeroVector;

	/** If set, the lane will be switched when path finishes. */
	UPROPERTY(Transient)
	FZoneGraphLaneHandle NextLaneHandle;

	/** Distance to move. */
	UPROPERTY(Transient)
	float TargetDistance = 0.0f;

	/** Optional specific direction at the end of the path. Used only if EndOfPathPosition is set. */
	UPROPERTY(Transient)
	FMassSnorm8Vector EndOfPathDirection;

	/** If start or end of path is off-lane, the distance along the lane is pushed forward/back along the lane to make smoother transition. */
	UPROPERTY(Transient)
	FMassInt16Real AnticipationDistance = FMassInt16Real(50.f);

	UPROPERTY(Transient)
	FMassInt16Real EndOfPathOffset = FMassInt16Real(0.0f);

	/** Movement intent at the end of the path. */
	UPROPERTY(Transient)
	EMassMovementAction EndOfPathIntent = EMassMovementAction::Stand;

	/** How the NextLaneHandle links to current lane. */
	UPROPERTY(Transient)
	EZoneLaneLinkType NextExitLinkType = EZoneLaneLinkType::None;

	/** If true, move backwards along the lane. */
	UPROPERTY(Transient)
	uint8 bMoveReverse : 1;

	/** Indicates if the optional end of path position is set. */
	UPROPERTY(Transient)
	uint8 bIsEndOfPathPositionSet : 1;

	/** Indicates if the optional end of path direction is set. Used only if EndOfPathPosition is set. */
	UPROPERTY(Transient)
	uint8 bIsEndOfPathDirectionSet : 1;
};

USTRUCT()
struct MASSAIMOVEMENT_API FMassZoneGraphPathPoint
{
	GENERATED_BODY()

	/** Position of the path. */
	FVector Position = FVector::ZeroVector;

	/** Tangent direction of the path. */
	FMassSnorm8Vector2D Tangent;

	/** Position of the point along the original path. (Could potentially be uint16 at 10cm accuracy) */
	FMassInt16Real10 DistanceAlongLane = FMassInt16Real10(0.0f);

	/** Distance along the offset path from first point. (Could potentially be uint16 at 10cm accuracy) */
	FMassInt16Real Distance = FMassInt16Real(0.0f);

	/** True if this point is assumed to be off lane. */
	uint8 bOffLane : 1;

	/** True if this point is lane start or end point. */
	uint8 bIsLaneExtrema : 1;
};
