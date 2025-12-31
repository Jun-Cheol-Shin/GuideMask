// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "../GuideMaskUI/UI/GuideBoxBase.h"
#include "../GuideMaskUI/UI/GuideMaskRegister.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GuideMaskUIFunctionLibrary.generated.h"


DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnGetDynamicEntryDynamicEvent, UObject*, InEntryItem);


USTRUCT(BlueprintType)
struct FGuideDynamicWidgetPath
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FOnGetDynamicEntryDynamicEvent OnGetDynamicEvent;

	UPROPERTY(EditAnywhere)
	int NextSearchChildIndex = -1;
};



class UGuideMaskRegister;
class UListView;

UCLASS()
class GUIDEMASKUI_API UGuideMaskUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideWidget(const UObject* WorldContextObject, UWidget* InTagWidget, const FGuideBoxActionParameters& InActionParam = FGuideBoxActionParameters(), int InLayerZOrder = 0);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideListEntry(const UObject* WorldContextObject, UListView* InTagListView, UObject* InListItem, const FGuideBoxActionParameters& InActionParam = FGuideBoxActionParameters(), int InLayerZOrder = 0, float InAsyncTimeout = 1.f);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideDynamicWidget(const UObject* WorldContextObject, UWidget* InWidget, const TArray<FGuideDynamicWidgetPath>& InPath, const FGuideBoxActionParameters& InActionParam = FGuideBoxActionParameters(), int InLayerZOrder = 0, float InAsyncTimeout = 1.f);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "WidgetClass", DynamicOutputParam = "FoundWidgets"))
	static void GetAllGuideRegisters(const UObject* WorldContextObject, TArray<UGuideMaskRegister*>& FoundWidgets);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static UWidget* GetTagWidget(const UObject* WorldContextObject, const FName& InTag);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static UGuideMaskRegister* GetRegister(const UObject* WorldContextObject, const FName& InTag);

};
