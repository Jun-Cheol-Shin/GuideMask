// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "GuideMaskRegister.generated.h"

/**
 * 
 */


class SOverlay;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGuideEventDynamic, const FName&, InTag, UWidget*, InGuideWidget);


UCLASS(meta = (DisplayName = "Guide Mask Register", Category = "Guide_Mask"))
class GUIDEMASKUI_API UGuideMaskRegister : public UContentWidget
{
	GENERATED_BODY()



private:
	void SetLayer(UWidget* InLayer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;	
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	void CreatePreviewLayer(const FGeometry& InViewportGeometry);

	virtual const FText GetPaletteCategory() override;
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;

	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Setting", DisplayName = "Show Preview"))
	void ShowPreviewDebug();

	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Setting", DisplayName = "Hide Preview"))
	void HidePreviewDebug();

	UFUNCTION()
	TArray<FName> GetTagOptions() const;
#endif

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, meta = (Category = "Guide Mask Setting", GetOptions = "GetTagOptions", AllowPrivateAccess = "true"))
	FName PreviewWidgetTag;

	UPROPERTY(EditInstanceOnly, meta = (Category = "Guide Mask Setting", AllowPrivateAccess = "true"))
	TMap<FName, UWidget*> TagWidgetList;
#endif

private:
	TSharedPtr<SOverlay> Overlay;
	
	UPROPERTY(Transient)
	UWidget* LayerContent = nullptr;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Show Guide"))
	FOnGuideEventDynamic OnShowEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Complete Guide Action"))
	FOnGuideEventDynamic OnCompleteActionEvent;


};
