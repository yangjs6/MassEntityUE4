// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "Engine/DataTable.h"
#include "Logging/LogMacros.h"
#include "MassCommonTypes.h"
#include "InstancedStruct.h"
#include "MassEntityTemplate.h"
#include "MassEntitySpawnPointsGeneratorBase.h"

#include "MassSpawnerTypes.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMassSpawner, Warning, All);

class APawn;
class UCurveFloat;
class UMassEntityConfigAsset;

USTRUCT()
struct FMassSpawnAuxData
{
	GENERATED_BODY()

	TArray<FTransform> Transforms;
};

/**
 * Describes an entity type to spawn.
 */
USTRUCT(BlueprintType)
struct FMassSpawnedEntityType
{
	GENERATED_BODY()

	/** Asset that describes the entity */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass|Spawn")
	TSoftObjectPtr<UMassEntityConfigAsset> EntityConfig;

	/** Proportion of the count that should be this agent type, (the proportions will be normalized with other sibling agent types) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass|Spawn", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Proportion = 1.0f;
	
	bool IsLoaded() const { return EntityConfigPtr != nullptr; }

	void UnloadEntityConfig()
	{
		EntityConfigPtr = nullptr;
		EntityConfig.ResetWeakPtr();
	}

	const UMassEntityConfigAsset* GetEntityConfig() const;
	UMassEntityConfigAsset* GetEntityConfig();

private:
	UPROPERTY()
	mutable UMassEntityConfigAsset* EntityConfigPtr = nullptr;
};

USTRUCT(BlueprintType)
struct FMassSpawnPointGenerator
{
	GENERATED_BODY()

	/** The Generator to use to generate the spawn the points */
	UPROPERTY()
	TSubclassOf<UMassEntitySpawnPointsGeneratorBase> GeneratorClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, Category = "Mass|Generator")
	UMassEntitySpawnPointsGeneratorBase* GeneratorInstance = nullptr;

	/** Proportion of the spawn points that should be generated by this generator, (the proportions will be normalized with other sibling generators) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass|Generator", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Proportion = 1.0f;

	/** Runtime value to know if we received the generated points for this generator */
	bool bPointsGenerated = false;
};

USTRUCT()
struct MASSSPAWNER_API FDataFragment_ReplicationTemplateID : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FMassEntityTemplateID ID;
};
