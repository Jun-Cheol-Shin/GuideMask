// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "../GuideMaskUI/UI/GuideBoxBase.h"
#include "../GuideMaskUI/UI/GuideMaskRegister.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GuideMaskUIFunctionLibrary.generated.h"

class UObject;

UENUM(BlueprintType)
enum class EGuideWidgetPredTarget : uint8
{
	ListItem		UMETA(DisplayName = "List Item (UObject)"),
	EntryWidget		UMETA(DisplayName = "Dynamic Entry Widget (UUserWidget)"),

};


DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FOnGetDynamicEntryDynamicEvent, EGuideWidgetPredTarget, InPredTarget, UObject*, InEntryItem);

USTRUCT(BlueprintType)
struct FGuideDynamicWidgetPath
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "GuideDynamicWidgetPath")
	FOnGetDynamicEntryDynamicEvent Predicate;

	UPROPERTY(EditAnywhere, Category = "GuideDynamicWidgetPath")
	int NextChildIndex = -1;
};


class UGuideMaskRegister;
class UListView;

UCLASS()
class GUIDEMASKUI_API UGuideMaskUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideWidget(UObject* WorldContextObject, UWidget* InTagWidget, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder = 0);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideListEntry(UObject* WorldContextObject, UListView* InTagListView, UObject* InListItem, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder = 0, float InAsyncTimeout = 1.f);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static void ShowGuideDynamicWidget(UObject* WorldContextObject, UWidget* InWidget, const TArray<FGuideDynamicWidgetPath>& InPath, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder = 0, float InAsyncTimeout = 1.f);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "WidgetClass", DynamicOutputParam = "FoundWidgets"))
	static void GetAllGuideRegisters(UObject* WorldContextObject, TArray<UGuideMaskRegister*>& FoundWidgets);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static UWidget* GetTagWidget(UObject* WorldContextObject, const FName& InTag);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Guide Mask UI Functions", meta = (WorldContext = "WorldContextObject"))
	static UGuideMaskRegister* GetRegister(UObject* WorldContextObject, const FName& InTag);

};
