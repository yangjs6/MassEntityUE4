// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassReplicationTransformHandlers.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "MassAIHelpers.h"

void FMassReplicationProcessorTransformHandlerBase::AddRequirements(FMassEntityQuery& InQuery)
{
	InQuery.AddRequirement<FDataFragment_Transform>(EMassFragmentAccess::ReadWrite);
}

void FMassReplicationProcessorTransformHandlerBase::CacheFragmentViews(FMassExecutionContext& ExecContext)
{
	TransformList = ExecContext.GetMutableFragmentView<FDataFragment_Transform>();
}

void FMassReplicationProcessorPositionYawHandler::AddEntity(const int32 EntityIdx, FReplicatedAgentPositionYawData& InOutReplicatedPositionYawData) const
{
	const FDataFragment_Transform& TransformFragment = TransformList[EntityIdx];
	InOutReplicatedPositionYawData.SetPosition(TransformFragment.GetTransform().GetLocation());

	if (const TOptional<float> Yaw = UE::AI::GetYawFromQuaternion(TransformFragment.GetTransform().GetRotation()))
	{
		InOutReplicatedPositionYawData.SetYaw(Yaw.GetValue());
	}
}
