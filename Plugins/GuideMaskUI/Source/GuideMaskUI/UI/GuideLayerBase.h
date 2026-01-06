// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuideBoxBase.h"
#include "GuideLayerBase.generated.h"

class USizeBox;
class UCanvasPanel;
class UImage;

class UGuideBoxBase;
struct FGuideBoxActionParameters;

UCLASS()
class GUIDEMASKUI_API UGuideLayerBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	void SetGuide(UWidget* InWidget, const FGuideBoxActionParameters& InParameter);


	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	FVector2D GetWidgetPosition() const;

	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	FVector2D GetWidgetSize() const;


	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	void SetEnableAnim(bool bIsEnable);

	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	bool IsEnabledAnim() const;



	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	void SetCircularShape(bool bIsEnable);

	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	bool IsCircularShape() const;


	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	void SetOpacity(float InOpacity);

	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	float GetOpacity() const;


	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	const FMargin& GetBoxOffset() const;

	UFUNCTION(BlueprintCallable, Category = "GuideLayerBase")
	void SetBoxOffset(const FMargin& InMargin);

#if WITH_EDITOR
public:
	void SetPreviewGuide(const FGeometry& InViewportGeometry, UWidget* InWidget);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Preview Guide"))
	void BP_OnPreviewGuide(const FVector2D& InPreviewWidgetPosition, const FVector2D& InPreviewWidgetSize);

	/**
	 * Called whenever a preview layer is made for this widget in the designer.
	 */
	TFunction<void(const FVector2D&, const FVector2D&)> OnPreviewGuideLayerFunc;
#endif

protected:
	virtual void SetGuideInternal(const FGeometry& InViewportGeometry, UWidget* InWidget);

protected:
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "On Start Action"))
	void OnStartGuide(UWidget* InWidget, const FGuideBoxActionParameters& InParam);
	virtual void OnStartGuide_Implementation(UWidget* InWidget, const FGuideBoxActionParameters& InParam) {};

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "On End Action"))
	void OnEndGuide();
	virtual void OnEndGuide_Implementation() {};

	virtual FReply OnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent);
	virtual FReply OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent);
	virtual FReply OnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InEvent);

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
	UPROPERTY(EditDefaultsOnly, BlueprintSetter = SetOpacity, BlueprintGetter = GetOpacity, meta = (Category = "Layer Setting", AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "1"))
	float Opacity = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintSetter = SetCircularShape, BlueprintGetter = IsCircularShape, meta = (Category = "Layer Setting", AllowPrivateAccess = "true"))
	bool bShapeCircle = false;

	UPROPERTY(EditDefaultsOnly, BlueprintSetter = SetEnableAnim, BlueprintGetter = IsEnabledAnim, meta = (Category = "Layer Setting", AllowPrivateAccess = "true"))
	bool bAnimated = false;

	UPROPERTY(EditDefaultsOnly, BlueprintSetter = SetBoxOffset, BlueprintGetter = GetBoxOffset, meta = (Category = "Layer Setting", AllowPrivateAccess = "true"))
	FMargin GuideBoxOffset;


#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview Layer Setting", AllowPrivateAccess = "true"))
	FVector2D ScreenPosition;

	UPROPERTY(EditDefaultsOnly, meta = (Category = "Preview Layer Setting", AllowPrivateAccess = "true"))
	FVector2D GuideSize;
#endif



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

	UPROPERTY(Transient)
	UGuideBoxBase* BoxBaseWidget = nullptr;

	TWeakObjectPtr<UWidget> GuideWidget = nullptr;
};
