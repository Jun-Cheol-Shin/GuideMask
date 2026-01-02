// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"

#include "GuideMaskRegister.generated.h"

/**
 * 
 */


class SOverlay;

USTRUCT(BlueprintType)
struct FGuideHierarchyNode
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "Entry Based Container"))
	UWidget* Container = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "Children"))
	TArray<UWidget*> Children {};
};



UCLASS(meta = (DisplayName = "Guide Mask Register"))
class GUIDEMASKUI_API UGuideMaskRegister : public UContentWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	bool IsContains(const FName& InTag) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	TArray<FName> GetTagList() const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	UWidget* GetTagWidget(const FName& InGuideTag);

	bool GetGuideWidgetTree(OUT TArray<FGuideHierarchyNode>& OutWidgetTree, const FName& InGuideTag);
	bool GetGuideWidgetList(OUT TArray<UWidget*>& OutWidgetList, const FName& InGuideTag);

private:
	void SetLayer(UWidget* InLayer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;	 
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR

protected:
	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Preview Option", DisplayName = "Show Preview"))
	void ShowPreviewDebug();

	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Preview Option", DisplayName = "Hide Preview"))
	void HidePreviewDebug();

	virtual const FText GetPaletteCategory() override;
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void ConstructWidgetTree(OUT TArray<FGuideHierarchyNode>& OutNodeTree, UWidget* InWidget) const;

	UFUNCTION()
	TArray<FName> GetTagOptions() const;

	UFUNCTION()
	TArray<FName> GetNestedWidgetOptions() const;


	void CreatePreviewLayer(const FGeometry& InViewportGeometry);
#endif

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, meta = (GetOptions = "GetTagOptions", DisplayName = "Tag"))
	FName PreviewTag;

	UPROPERTY(EditInstanceOnly, meta = (GetOptions = "GetNestedWidgetOptions", DisplayName = "Target Widget"))
	FName PreviewWidget;

	UPROPERTY(VisibleInstanceOnly)
	TArray<FGuideHierarchyNode> WidgetHierarchy {};

#endif
	UPROPERTY(EditInstanceOnly)
	TMap<FName, UWidget*> TagWidgetList;

private:
	TSharedPtr<SOverlay> Overlay;
	
	UPROPERTY(Transient)
	UWidget* LayerContent = nullptr;

};
