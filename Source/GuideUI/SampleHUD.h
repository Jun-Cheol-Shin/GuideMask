// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuideMaskUI/UI/GuideBoxBase.h"
#include "SampleHUD.generated.h"


class UListView;
class UComboBoxString;
class UButton;
class UTextBlock;
class USpinBox;

struct FGuideDynamicWidgetPath;

UCLASS()
class GUIDEUI_API USampleHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FKey GetSelectedKey() const { return SelectedKey; }

	UFUNCTION(BlueprintCallable)
	EGuideActionType GetActionType() const { return ActionType; }

	UFUNCTION(BlueprintCallable)
	float GetDragValue() const { return DragValue; }

	UFUNCTION(BlueprintCallable)
	float GetHoldTime() const { return HoldTime; }

	UFUNCTION(BlueprintCallable)
	TArray<FGuideDynamicWidgetPath> GetDynamicPath() const;


	UFUNCTION(BlueprintCallable)
	FName GetSelectedTag() const;

	UFUNCTION(BlueprintCallable)
	UObject* GetSampleListItem(UWidget* InTagWidget) const;


protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnTagSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType);

	UFUNCTION()
	void OnScopeWidgetSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType);


	UFUNCTION()
	void OnNestedWidgetSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType);

	UFUNCTION()
	void OnActionTypeSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType);

	UFUNCTION()
	void OnActionKeySelectionChanged(FString InSelectedItem, ESelectInfo::Type InType);

	UFUNCTION()
	void OnChangedDragValue(float InValue);

	UFUNCTION()
	void OnChangedHoldTime(float InValue);

	UFUNCTION()
	bool OnGetListItem(UObject* InItem);

	UFUNCTION()
	bool OnGetDynamicEntry(UObject* InEntry) { return true; } 


private:
	void Initialize_SampleListView();
	void Initialize_TagComboBox();
	void Initialize_ActionTypeComboBox();
	
private:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess= "true"))
	UListView* SampleListView;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UComboBoxString* TagComboBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UComboBoxString* ScopeWidgetComboBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UComboBoxString* NestedWidgetComboBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UComboBoxString* ActionTypeComboBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UComboBoxString* ActionKeyComboBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	USpinBox* DragValueBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	USpinBox* HoldTimeBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* GuideButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* WarningText;


private:
	TMap<FString, FKey> DisplayToKey;


private:
	FKey SelectedKey;
	float DragValue = 0.f;
	float HoldTime = 0.f;
	EGuideActionType ActionType;
};
