// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassSpawnerSubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassEntityTemplate.h"
#include "MassEntitySubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "Engine/World.h"
#include "MassExecutor.h"
#include "InstancedStruct.h"
#include "VisualLogger/VisualLogger.h"
#include "MassSpawner.h"
#include "MassObserverProcessor.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "MassSimulationSubsystem.h"
#include "MassProcessor.h"
#include "MassEntityUtils.h"

//----------------------------------------------------------------------//
//  UMassSpawnerSubsystem
//----------------------------------------------------------------------//
void UMassSpawnerSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{	
	Super::Initialize(Collection);

	// making sure UMassSimulationSubsystem gets created before the MassActorManager
	Collection.InitializeDependency<UMassSimulationSubsystem>();

	UWorld* World = GetWorld();
	SimulationSystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(World);
	EntitySystem = UWorld::GetSubsystem<UMassEntitySubsystem>(World);
	check(EntitySystem);
}

void UMassSpawnerSubsystem::PostInitialize()
{
	TemplateRegistryInstance = NewObject<UMassEntityTemplateRegistry>(this);
}

void UMassSpawnerSubsystem::SpawnEntities(const FMassEntityTemplate& EntityTemplate, const uint32 NumberToSpawn, TArray<FMassEntityHandle>& OutEntities)
{
	check(EntitySystem);
	check(EntityTemplate.IsValid());

	if (NumberToSpawn == 0)
	{
		UE_VLOG(this, LogMassSpawner, Warning, TEXT("Trying to spawn 0 entities. This would cause inefficiency. Bailing out with result FALSE."));
		return;
	}

	DoSpawning(EntityTemplate, NumberToSpawn, FStructView(), TSubclassOf<UMassProcessor>(), OutEntities);
}

void UMassSpawnerSubsystem::SpawnEntities(FMassEntityTemplateID TemplateID, const uint32 NumberToSpawn, FConstStructView SpawnData, TSubclassOf<UMassProcessor> InitializerClass, TArray<FMassEntityHandle>& OutEntities)
{
	check(TemplateID.IsValid());

	if (!ensureMsgf(TemplateRegistryInstance, TEXT("UMassSpawnerSubsystem didn\'t get its OnPostWorldInit call yet!")))
	{
		return;
	}

	const FMassEntityTemplate* EntityTemplate = TemplateRegistryInstance->FindTemplateFromTemplateID(TemplateID);
	checkf(EntityTemplate && EntityTemplate->IsValid(), TEXT("SpawnEntities: TemplateID must have been registered!"));

	DoSpawning(*EntityTemplate, NumberToSpawn, SpawnData, InitializerClass, OutEntities);
}

void UMassSpawnerSubsystem::SpawnFromConfig(FStructView Config, const int32 NumToSpawn, FConstStructView SpawnData, TSubclassOf<UMassProcessor> InitializerClass)
{
	check(Config.IsValid());

	if (!ensureMsgf(TemplateRegistryInstance, TEXT("UMassSpawnerSubsystem didn\'t get its OnPostWorldInit call yet!")))
	{
		return;
	}
	
	const FMassEntityTemplate* EntityTemplate = TemplateRegistryInstance->FindOrBuildStructTemplate(Config);
	checkf(EntityTemplate && EntityTemplate->IsValid(), TEXT("SpawnFromConfig: TemplateID must have been registered!"));

	TArray<FMassEntityHandle> Entities;
	DoSpawning(*EntityTemplate, NumToSpawn, SpawnData, InitializerClass, Entities);
}

void UMassSpawnerSubsystem::RegisterCollection(TArrayView<FInstancedStruct> Collection)
{
	if (!ensureMsgf(TemplateRegistryInstance, TEXT("UMassSpawnerSubsystem didn't get its OnPostWorldInit called yet!")))
	{
		return;
	}

	for (FInstancedStruct& Entry : Collection)
	{
		if (Entry.IsValid())
		{
			TemplateRegistryInstance->FindOrBuildStructTemplate(Entry);
		}
	}
}

void UMassSpawnerSubsystem::DestroyEntities(const FMassEntityTemplateID TemplateID, TConstArrayView<FMassEntityHandle> Entities)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("MassSpawnerSubsystem_DestroyEntities")

	if (!ensureMsgf(TemplateRegistryInstance, TEXT("UMassSpawnerSubsystem didn\'t get its OnPostWorldInit call yet!")))
	{
		return;
	}

	check(EntitySystem);
	check(SimulationSystem);
	checkf(!SimulationSystem->GetPhaseManager().IsDuringMassProcessing()
		, TEXT("%s called while MassEntity processing in progress. This is unsupported and dangerous!"), ANSI_TO_TCHAR(__FUNCTION__));

	UWorld* World = GetWorld();
	check(World);


	TArray<FArchetypeChunkCollection> ChunkCollections;
	UE::Mass::Utils::CreateSparseChunks(*EntitySystem, Entities, FArchetypeChunkCollection::NoDuplicates, ChunkCollections);

	for (const FArchetypeChunkCollection& Chunks : ChunkCollections)
	{
		EntitySystem->BatchDestroyEntityChunks(Chunks);
	}
}

UMassProcessor* UMassSpawnerSubsystem::GetSpawnLocationInitializer(TSubclassOf<UMassProcessor> InitializerClass)
{	
	if (!InitializerClass)
	{
		return nullptr;
	}

	UMassProcessor** const Initializer = SpawnLocationInitializers.FindByPredicate([InitializerClass](const UMassProcessor* Processor)
		{
			return Processor && Processor->GetClass() == InitializerClass;
		}
	);

	if (Initializer == nullptr)
	{
		UMassProcessor* NewInitializer = NewObject<UMassProcessor>(this, InitializerClass);
		NewInitializer->Initialize(*this);
		SpawnLocationInitializers.Add(NewInitializer);
		return NewInitializer;
	}

	return *Initializer;
}

void UMassSpawnerSubsystem::DoSpawning(const FMassEntityTemplate& EntityTemplate, const int32 NumToSpawn, FConstStructView SpawnData, TSubclassOf<UMassProcessor> InitializerClass, TArray<FMassEntityHandle>& OutEntities)
{
	check(EntitySystem);
	check(EntityTemplate.GetArchetype().IsValid());
	UE_VLOG(this, LogMassSpawner, Log, TEXT("Spawning with EntityTemplate:\n%s"), *EntityTemplate.DebugGetDescription(EntitySystem));

	if (NumToSpawn <= 0)
	{
		UE_VLOG(this, LogMassSpawner, Warning, TEXT("%s: Trying to spawn %d entities. Ignoring."), ANSI_TO_TCHAR(__FUNCTION__), NumToSpawn);
		return;
	}

	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("MassSpawnerSubsystem DoSpawning");

	// 1. Create required number of entities with EntityTemplate.Archetype
	// 2. Copy data from FMassEntityTemplate.Fragments.
	//		a. @todo, could be done as part of creation?
	// 3. Run SpawlLocationInitializer if set
	// 4. "OnEntitiesCreated" notifies will be sent out once the CreationContext gets destroyed (via its destructor).

	TArray<FMassEntityHandle> SpawnedEntities;
	TSharedRef<UMassEntitySubsystem::FEntityCreationContext> CreationContext = EntitySystem->BatchCreateEntities(EntityTemplate.GetArchetype(), NumToSpawn, SpawnedEntities);

	TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate.GetInitialFragmentValues();
	EntitySystem->BatchSetEntityFragmentsValues(CreationContext->GetChunkCollection(), FragmentInstances);
	
	UMassProcessor* SpawnLocationInitializer = SpawnData.IsValid() ? GetSpawnLocationInitializer(InitializerClass) : nullptr;

	if (SpawnLocationInitializer)
	{
		FMassProcessingContext ProcessingContext(*EntitySystem, /*TimeDelta=*/0.0f);
		ProcessingContext.AuxData = SpawnData;
		UE::Mass::Executor::RunProcessorsView(MakeArrayView(&SpawnLocationInitializer, 1), ProcessingContext, &CreationContext->GetChunkCollection());
	}

	OutEntities.Append(MoveTemp(SpawnedEntities));
}

const FMassEntityTemplate* UMassSpawnerSubsystem::GetMassEntityTemplate(FMassEntityTemplateID TemplateID) const
{
	check(TemplateID.IsValid());

	if (!ensureMsgf(TemplateRegistryInstance, TEXT("UMassSpawnerSubsystem didn\'t get its OnPostWorldInit call yet!")))
	{
		return nullptr;
	}

	return TemplateRegistryInstance->FindTemplateFromTemplateID(TemplateID);
}
