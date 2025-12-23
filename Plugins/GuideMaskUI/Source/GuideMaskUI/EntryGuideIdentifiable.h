// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EntryGuideIdentifiable.generated.h"


class UUserWidget;
class UWidget;

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnGetEqualListItemEvent, UObject*, InItem);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnGetEqualDynamicEntryEvent, UUserWidget*, InEntry);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetTreeViewChildrenEvent, UObject*, InItem, TArray<UObject*>&, OutChildren);


USTRUCT(BlueprintType)
struct FListViewIdentifyParam
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IsBindableEvent = true))
	FOnGetEqualListItemEvent OnGetEqualItemDynamicEvent {};

	UPROPERTY(EditAnywhere)
	UWidget* NestedView = nullptr;

};

USTRUCT(BlueprintType)
struct FTreeViewIdentifyParam
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IsBindableEvent = true))
	FOnGetEqualListItemEvent OnGetEqualItemDynamicEvent {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IsBindableEvent = true))
	FOnGetTreeViewChildrenEvent OnGetChildrenDynamicEvent {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecursiveFindChildren = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidget* NestedView = nullptr;
};




USTRUCT(BlueprintType)
struct FDynamicEntryBoxIdentifyParam
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (IsBindableEvent = true))
	FOnGetEqualDynamicEntryEvent OnGetEqualEntryDynamicEvent{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidget* NestedView = nullptr;

};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEntryGuideIdentifiable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GUIDEMASKUI_API IEntryGuideIdentifiable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, meta = (Category = "Guide Mask UI Plugin", DisplayName = "On Get Desired List View Parameter"))
	void GetDesiredListViewParameter(FListViewIdentifyParam& OutParam);
	virtual void GetDesiredListViewParameter_Implementation(FListViewIdentifyParam& OutParam) const {};

	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, meta = (Category = "Guide Mask UI Plugin", DisplayName = "On Get Desired Tree View Parameter"))
	void GetDesiredTreeViewParameter(FTreeViewIdentifyParam& OutParam);
	virtual void GetDesiredTreeViewParameter_Implementation(FTreeViewIdentifyParam& OutParam) const {};

	UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, meta = (Category = "Guide Mask UI Plugin", DisplayName = "On Get Desired Dynamic Entry Box Parameter"))
	void GetDesiredDynamicEntryBoxParameter(FDynamicEntryBoxIdentifyParam& OutParam);
	virtual void GetDesiredDynamicEntryBoxParameter_Implementation(FDynamicEntryBoxIdentifyParam& OutParam) const {};

};
