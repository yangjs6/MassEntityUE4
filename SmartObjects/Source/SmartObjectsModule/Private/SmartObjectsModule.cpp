// Copyright Epic Games, Inc. All Rights Reserved.

#include "SmartObjectsModule.h"

#include "Modules/ModuleManager.h"
#include "SmartObjectTypes.h"
#include "UObject/CoreRedirects.h"

#if WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_SmartObject.h"
#endif

#define LOCTEXT_NAMESPACE "SmartObjects"

class FSmartObjectsModule : public ISmartObjectsModule
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FSmartObjectsModule, SmartObjectsModule)

void FSmartObjectsModule::StartupModule()
{
	TArray<FCoreRedirect> Redirects;
	Redirects.Emplace(ECoreRedirectFlags::Type_Class, TEXT("/Script/SmartObjectsModule.SmartObjectBehaviorConfigBase"), TEXT("/Script/SmartObjectsModule.SmartObjectBehaviorDefinition"));
	Redirects.Emplace(ECoreRedirectFlags::Type_Class, TEXT("/Script/SmartObjectsModule.SmartObjectGameplayBehaviorConfig"), TEXT("/Script/SmartObjectsModule.SmartObjectGameplayBehaviorDefinition"));

	Redirects.Emplace(ECoreRedirectFlags::Type_Property, TEXT("/Script/SmartObjectsModule.SmartObjectDefinition.DefaultBehaviorConfigurations"), TEXT("/Script/SmartObjectsModule.SmartObjectDefinition.DefaultBehaviorDefinitions"));
	Redirects.Emplace(ECoreRedirectFlags::Type_Property, TEXT("/Script/SmartObjectsModule.SmartObjectRequestFilter.BehaviorConfigurationClass"), TEXT("/Script/SmartObjectsModule.SmartObjectRequestFilter.BehaviorDefinitionClass"));
	Redirects.Emplace(ECoreRedirectFlags::Type_Property, TEXT("/Script/SmartObjectsModule.SmartObjectSlot.BehaviorConfigurations"), TEXT("/Script/SmartObjectsModule.SmartObjectSlot.BehaviorDefinitions"));

	FCoreRedirects::AddRedirectList(Redirects, TEXT("SmartObjectsModule"));

#if WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("SmartObject",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_SmartObject::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FSmartObjectsModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER && WITH_SMARTOBJECT_DEBUG
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("SmartObject");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE
