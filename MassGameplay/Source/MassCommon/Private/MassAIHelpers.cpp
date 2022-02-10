// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassAIHelpers.h"

#include "Engine/InheritableComponentHandler.h"
#include "Engine/SCS_Node.h"

namespace UE::AI
{

TOptional<float> GetYawFromVector(const FVector& Vector)
{
	const FVector2D Vector2D(Vector);
	const float SizeSquared = Vector2D.SizeSquared();
	if (SizeSquared <= SMALL_NUMBER)
	{
		return TOptional<float>();
	}

	const FVector2D ForwardVector(1.0f, 0.0f);
	const FVector2D NormVector2D = Vector2D * FMath::InvSqrt(SizeSquared);
	const float DotVal = ForwardVector | NormVector2D;
	const float ClampedDot = FMath::Clamp(DotVal, -1.0f, 1.0f);
	const float Direction = ForwardVector ^ NormVector2D;
	const float Sign = Direction < 0.0f ? -1.0f : 1.0f;
	const float Yaw = Sign * FMath::Acos(ClampedDot);
	return Yaw;
}

TOptional<float> GetYawFromRotator(const FRotator& Rotator)
{
	return GetYawFromVector(Rotator.Vector());
}

TOptional<float> GetYawFromQuaternion(const FQuat& Quaternion)
{
	return GetYawFromVector(Quaternion.Vector());
}

void GetActorClassDefaultComponents(const TSubclassOf<AActor>& ActorClass, TArray<UActorComponent*>& OutComponents, const TSubclassOf<UActorComponent>& InComponentClass)
{
	if (!ensure(ActorClass.Get()))
	{
		return;
	}

	UClass* ClassPtr = InComponentClass.Get();
	TArray<UActorComponent*> ResultComponents;

	// Get the components defined on the native class.
	AActor* CDO = ActorClass->GetDefaultObject<AActor>();
	check(CDO);
	if (ClassPtr)
	{
		CDO->GetComponents(InComponentClass, ResultComponents);
	}
	else
	{
		CDO->GetComponents(ResultComponents);
	}

	// Try to get the components off the BP class.
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(*ActorClass);
	if (BPClass)
	{
		// A BlueprintGeneratedClass has a USimpleConstructionScript member. This member has an array of RootNodes
		// which contains the SCSNode for the root SceneComponent and non-SceneComponents. For the SceneComponent
		// hierarchy, each SCSNode knows its children SCSNodes. Each SCSNode stores the component template that will
		// be created when the Actor is spawned.
		//
		// WARNING: This may change in future engine versions!

		TArray<UActorComponent*> Unfiltered;
		// using this semantic to avoid duplicating following loops or adding a filtering check condition inside the loops
		TArray<UActorComponent*>& TmpComponents = ClassPtr ? Unfiltered : ResultComponents;

		// Check added components.
		USimpleConstructionScript* ConstructionScript = BPClass->SimpleConstructionScript;
		if (ConstructionScript)
		{
			for (const USCS_Node* Node : ConstructionScript->GetAllNodes())
			{
				TmpComponents.Add(Node->ComponentTemplate);
			}
		}
		// Check modified inherited components.
		UInheritableComponentHandler* InheritableComponentHandler = BPClass->InheritableComponentHandler;
		if (InheritableComponentHandler)
		{
			for (TArray<FComponentOverrideRecord>::TIterator It = InheritableComponentHandler->CreateRecordIterator(); It; ++It)
			{
				TmpComponents.Add(It->ComponentTemplate);
			}
		}

		// Filter to the ones matching the requested class.
		if (ClassPtr)
		{
			for (UActorComponent* TemplateComponent : Unfiltered)
			{
				if (TemplateComponent->IsA(ClassPtr))
				{
					ResultComponents.Add(TemplateComponent);
				}
			}
		}
	}

	OutComponents = MoveTemp(ResultComponents);
}
} // UE::AI