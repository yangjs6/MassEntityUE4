// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "Async/TaskGraphInterfaces.h"


struct FMassRuntimePipeline;
struct FMassProcessingContext;
struct FMassEntityHandle;
struct FArchetypeChunkCollection;
class UMassProcessor;


namespace UE::Mass::Executor
{
	/** Executes processors in a given RuntimePipeline */
	MASSENTITY_API void Run(FMassRuntimePipeline& RuntimePipeline, FMassProcessingContext& ProcessingContext);

	/** Executes given Processor. Used mainly for triggering calculations via MassCompositeProcessors, e.g processing phases */
	MASSENTITY_API void Run(UMassProcessor& Processor, FMassProcessingContext& ProcessingContext);

	/** Similar to the Run function, but instead of using all the entities hosted by ProcessingContext.EntitySubsystem 
	 *  it is processing only the entities given by EntityID via the Entities input parameter. 
	 *  Note that all the entities need to be of Archetype archetype. 
	 *  Under the hood the function converts Archetype-Entities pair to FArchetypeChunkCollection and calls the other flavor of RunSparse
	 */
	MASSENTITY_API void RunSparse(FMassRuntimePipeline& RuntimePipeline, FMassProcessingContext& ProcessingContext, FArchetypeHandle Archetype, TConstArrayView<FMassEntityHandle> Entities);

	/** Similar to the Run function, but instead of using all the entities hosted by ProcessingContext.EntitySubsystem 
	 *  it is processing only the entities given by SparseEntities input parameter.
	 *  @todo rename
	 */
	MASSENTITY_API void RunSparse(FMassRuntimePipeline& RuntimePipeline, FMassProcessingContext& ProcessingContext, const FArchetypeChunkCollection& ChunkCollection);

	/** Executes given Processors array view. This function gets called under the hood by the rest of Run* functions */
	MASSENTITY_API void RunProcessorsView(TArrayView<UMassProcessor*> Processors, FMassProcessingContext& ProcessingContext, const FArchetypeChunkCollection* ChunkCollection = nullptr);

	/** 
	 *  Triggers tasks executing Processor (and potentially it's children) and returns the task graph event representing 
	 *  the task (the event will be "completed" once all the processors finish running). 
	 *  @param OnDoneNotification will be called after all the processors are done, just after flushing the command buffer.
	 *    Note that OnDoneNotification will be executed on GameThread.
	 */
	MASSENTITY_API FGraphEventRef TriggerParallelTasks(UMassProcessor& Processor, FMassProcessingContext& ProcessingContext, TFunction<void()> OnDoneNotification);
};
