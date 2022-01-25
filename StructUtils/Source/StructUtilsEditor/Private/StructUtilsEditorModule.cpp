// Copyright Epic Games, Inc. All Rights Reserved.

#include "StructUtilsEditorModule.h"
#include "Modules/ModuleManager.h"
#include "InstancedStructDetails.h"

#define LOCTEXT_NAMESPACE "StructUtilsEditor"

IMPLEMENT_MODULE(FStructUtilsEditorModule, StructUtilsEditor)

void FStructUtilsEditorModule::StartupModule()
{
	// Register the details customizer
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("InstancedStruct", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInstancedStructDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FStructUtilsEditorModule::ShutdownModule()
{
	// Unregister the details customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("InstancedStruct");
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE
