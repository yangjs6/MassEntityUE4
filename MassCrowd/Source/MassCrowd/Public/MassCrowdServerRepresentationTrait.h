// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "MassEntityTraitBase.h"
#include "MassRepresentationTypes.h"
#include "GameFramework/Actor.h"
#include "MassCrowdServerRepresentationTrait.generated.h"


UCLASS(meta=(DisplayName="Crowd Server Representation"))
class MASSCROWD_API UMassCrowdServerRepresentationTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	/** Actor class of this agent when spawned on server */
	UPROPERTY(EditAnywhere, Category = "Mass|Visual")
	TSubclassOf<AActor> TemplateActor;
};
