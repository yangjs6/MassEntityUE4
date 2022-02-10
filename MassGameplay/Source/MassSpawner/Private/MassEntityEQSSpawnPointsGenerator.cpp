// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassEntityEQSSpawnPointsGenerator.h"
#include "MassSpawnerTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "VisualLogger/VisualLogger.h"


UMassEntityEQSSpawnPointsGenerator::UMassEntityEQSSpawnPointsGenerator()
{
	EQSRequest.RunMode = EEnvQueryRunMode::AllMatching; 
}

int32 RequestExecuteObject(FEQSParametrizedQueryExecutionRequest& Request, UObject& QueryOwner, const UBlackboardComponent* BlackboardComponent, FQueryFinishedSignature& QueryFinishedDelegate)
{
	UEnvQuery*& QueryTemplate = Request.QueryTemplate;
	TArray<FAIDynamicParam>& QueryConfig = Request.QueryConfig;
	if (Request.bUseBBKeyForQueryTemplate)
	{
		check(BlackboardComponent);

		// set QueryTemplate to contents of BB key
		UObject* QueryTemplateObject = BlackboardComponent->GetValue<UBlackboardKeyType_Object>(Request.EQSQueryBlackboardKey.GetSelectedKeyID());
		QueryTemplate = Cast<UEnvQuery>(QueryTemplateObject);

		// UE_CVLOG(QueryTemplate == nullptr, &QueryOwner, LogBehaviorTree, Warning, TEXT("Trying to run EQS query configured to use BB key, but indicated key (%s) doesn't contain EQS template pointer")
		// 	, *EQSQueryBlackboardKey.SelectedKeyName.ToString());
	}

	if (QueryTemplate != nullptr)
	{
		FEnvQueryRequest QueryRequest(QueryTemplate, &QueryOwner);
		if (QueryConfig.Num() > 0)
		{
			// resolve 
			for (FAIDynamicParam& RuntimeParam : QueryConfig)
			{
				// check if given param requires runtime resolve, like reading from BB
				if (RuntimeParam.BBKey.IsSet())
				{
					check(BlackboardComponent && "If BBKey.IsSet and there's no BB component then we\'re in the error land!");

					// grab info from BB
					switch (RuntimeParam.ParamType)
					{
					case EAIParamType::Float:
					{
						const float Value = BlackboardComponent->GetValue<UBlackboardKeyType_Float>(RuntimeParam.BBKey.GetSelectedKeyID());
						QueryRequest.SetFloatParam(RuntimeParam.ParamName, Value);
					}
					break;
					case EAIParamType::Int:
					{
						const int32 Value = BlackboardComponent->GetValue<UBlackboardKeyType_Int>(RuntimeParam.BBKey.GetSelectedKeyID());
						QueryRequest.SetIntParam(RuntimeParam.ParamName, Value);
					}
					break;
					case EAIParamType::Bool:
					{
						const bool Value = BlackboardComponent->GetValue<UBlackboardKeyType_Bool>(RuntimeParam.BBKey.GetSelectedKeyID());
						QueryRequest.SetBoolParam(RuntimeParam.ParamName, Value);
					}
					break;
					default:
						checkNoEntry();
						break;
					}
				}
				else
				{
					switch (RuntimeParam.ParamType)
					{
					case EAIParamType::Float:
					{
						QueryRequest.SetFloatParam(RuntimeParam.ParamName, RuntimeParam.Value);
					}
					break;
					case EAIParamType::Int:
					{
						QueryRequest.SetIntParam(RuntimeParam.ParamName, RuntimeParam.Value);
					}
					break;
					case EAIParamType::Bool:
					{
						bool Result = RuntimeParam.Value > 0;
						QueryRequest.SetBoolParam(RuntimeParam.ParamName, Result);
					}
					break;
					default:
						checkNoEntry();
						break;
					}
				}
			}
		}

		return QueryRequest.Execute(Request.RunMode, QueryFinishedDelegate);
	}

	return INDEX_NONE;
}

void UMassEntityEQSSpawnPointsGenerator::GenerateSpawnPoints(UObject& QueryOwner, int32 Count, FFinishedGeneratingSpawnPointsSignature& FinishedGeneratingSpawnPointsDelegate) const
{
	// Need to copy the request as it is called inside a CDO and CDO states cannot be changed.
	FEQSParametrizedQueryExecutionRequest EQSRequestInstanced = EQSRequest;
	if (EQSRequestInstanced.IsValid() == false)
	{
		EQSRequestInstanced.InitForOwnerAndBlackboard(QueryOwner, /*BBAsset=*/nullptr);
		if (!ensureMsgf(EQSRequestInstanced.IsValid(), TEXT("Query request initialization can fail due to missing parameters. See the runtime log for details")))
		{
			return;
		}
	}

	FQueryFinishedSignature Delegate = FQueryFinishedSignature::CreateUObject(this, &UMassEntityEQSSpawnPointsGenerator::OnEQSQueryFinished, Count, FinishedGeneratingSpawnPointsDelegate);
	RequestExecuteObject(EQSRequestInstanced, QueryOwner, /*BlackboardComponent=*/nullptr, Delegate);
}

void UMassEntityEQSSpawnPointsGenerator::OnEQSQueryFinished(TSharedPtr<FEnvQueryResult> Result, int32 Count, FFinishedGeneratingSpawnPointsSignature FinishedGeneratingSpawnPointsDelegate) const
{
	TArray<FVector> Locations;

	if (Result.IsValid() == false || Result->GetRawStatus() != EEnvQueryStatus::Type::Success)
	{
		UE_VLOG_UELOG(this, LogMassSpawner, Error, TEXT("EQS query failed or result is invalid"));
	}
	else
	{
		Result->GetAllAsLocations(Locations);
	}

	// Randomize them
	FRandomStream RandomStream(GFrameNumber);
	for (int32 I = 0; I < Locations.Num(); ++I)
	{
		const int32 J = RandomStream.RandHelper(Locations.Num());
		Locations.Swap(I, J);
	}

	// If we generated too many, shrink it.
	if (Locations.Num() > Count)
	{
		Locations.SetNum(Count);
	}

	FinishedGeneratingSpawnPointsDelegate.Execute(Locations);
}