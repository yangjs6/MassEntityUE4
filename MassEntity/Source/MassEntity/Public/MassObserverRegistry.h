// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassObserverRegistry.generated.h"


struct FMassObserverManager;

USTRUCT()
struct FMassProcessorClassCollection
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TSubclassOf<UMassProcessor>> ClassCollection;
};


UCLASS()
class MASSENTITY_API UMassObserverRegistry : public UObject
{
	GENERATED_BODY()

public:
	UMassObserverRegistry();

	static UMassObserverRegistry& GetMutable() { return *GetMutableDefault<UMassObserverRegistry>(); }
	static const UMassObserverRegistry& Get() { return *GetDefault<UMassObserverRegistry>(); }

	template<typename TFragment>
	static void RegisterInitializer(const UMassProcessor& CDO)
	{
		ensureMsgf(CDO.HasAnyFlags(RF_ClassDefaultObject) && CDO.GetClass()->HasAnyClassFlags(CLASS_Abstract) == false
			, TEXT("Only CDOs are expected to be registered with UMassObserverRegistry::RegisterInitializer"));
		GetMutable().RegisterFragmentInitializer(*TFragment::StaticStruct(), CDO.GetClass());
	}

	template<typename TFragment>
	static void RegisterDeinitializer(const UMassProcessor& CDO)
	{
		ensureMsgf(CDO.HasAnyFlags(RF_ClassDefaultObject) && CDO.GetClass()->HasAnyClassFlags(CLASS_Abstract) == false
			, TEXT("Only CDOs are expected to be registered with UMassObserverRegistry::RegisterDeinitializer"));
		GetMutable().RegisterFragmentDeinitializer(*TFragment::StaticStruct(), CDO.GetClass());
	}

	void RegisterFragmentInitializer(const UScriptStruct& FragmentType, TSubclassOf<UMassProcessor> FragmentInitializerClass);
	void RegisterFragmentDeinitializer(const UScriptStruct& FragmentType, TSubclassOf<UMassProcessor> FragmentDeinitializerClass);

protected:
	friend FMassObserverManager;

	TConstArrayView<TSubclassOf<UMassProcessor>> GetFragmentInitializers(const UScriptStruct& FragmentType) const;
	TConstArrayView<TSubclassOf<UMassProcessor>> GetFragmentDeinitializers(const UScriptStruct& FragmentType) const;

	UPROPERTY()
	TMap<const UScriptStruct*, FMassProcessorClassCollection> FragmentInitializersMap;

	UPROPERTY()
	TMap<const UScriptStruct*, FMassProcessorClassCollection> FragmentDeinitializersMap;
};
