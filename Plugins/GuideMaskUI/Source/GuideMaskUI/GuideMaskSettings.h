// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "UI/GuideLayerBase.h"
#include "UI/GuideBoxBase.h"

#include "GuideMaskSettings.generated.h"

/**
 * 
 */


UCLASS(Config = Game, DefaultConfig)
class GUIDEMASKUI_API UGuideMaskSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UGuideMaskSettings(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, Config, Category = "GuideMaskSetting", meta = (AllowedClasses = "/Script/GuideMaskUI.GuideLayerBase"))
	TSoftClassPtr<UGuideLayerBase> DefaultLayer;

	UPROPERTY(EditAnywhere, Config, Category = "GuideMaskSetting", meta = (AllowedClasses = "/Script/GuideMaskUI.GuideBoxBase"))
	TSoftClassPtr<UGuideBoxBase> DefaultBox;
};
