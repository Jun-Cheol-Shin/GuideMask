// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuideLayerBase.generated.h"

class USizeBox;
class UCanvasPanel;
class UImage;

UCLASS()
class GUIDEMASKUI_API UGuideLayerBase : public UUserWidget
{
	GENERATED_BODY()

	friend class UGuideMaskRegister;

public:
	void SetEnableAnim(bool bIsEnable);

	void SetAnimRate(float InRate);

	void SetCircularShape(bool bIsEnable);

	void SetOpacity(float InOpacity);

protected:
	virtual void SetGuide(const FGeometry& InViewportGeometry, UWidget* InWidget);
	virtual void SetGuideLayer(const FVector2D& InScreenSize, const FVector2D& InTargetLoc, const FVector2D& InTargetSize);
	virtual void SetGuideBox(UWidget* InWidget);
	virtual void SetMaterialTransform(const FVector2D& InViewportSize, const FVector2D& InPosiiton, const FVector2D& InWidgetSize);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InEvent) override;

	virtual void SynchronizeProperties() override;

private:
	void OnResizedViewport(FViewport* InViewport, uint32 InMessage);

	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "1"))
	float Opacity = 0.8f;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "1"))
	float AnimationRate = 0.8f;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true"))
	bool bUseCircle = false;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true"))
	bool bAnimated = false;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true"))
	FVector2D ScreenPosition;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview", AllowPrivateAccess = "true"))
	FVector2D HighlightSize;

protected:	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UCanvasPanel* LayerPanel;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UImage* BlackScreen;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	USizeBox* GuideBoxPanel;

protected:
	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstance = nullptr;	
};
