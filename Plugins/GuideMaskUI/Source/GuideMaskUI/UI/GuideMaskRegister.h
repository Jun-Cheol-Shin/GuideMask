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
struct FGuideTreeNode
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidget* Scope = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UWidget*> NestedWidgets {};
};



UCLASS(meta = (DisplayName = "Guide Mask Register", Category = "Guide_Mask", AutoExpandCategories = "Guide Mask Setting"))
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

	bool GetGuideWidgetTree(OUT TArray<FGuideTreeNode>& OutWidgetTree, const FName& InGuideTag);
	bool GetGuideWidgetList(OUT TArray<UWidget*>& OutWidgetList, const FName& InGuideTag);

private:
	void SetLayer(UWidget* InLayer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;	
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;

	void ConstructWidgetTree(OUT TArray<FGuideTreeNode>& OutNodeTree, UWidget* InWidget) const;

	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Setting", DisplayName = "Show Preview"))
	void ShowPreviewDebug();

	UFUNCTION(BlueprintCosmetic, CallInEditor, meta = (Category = "Guide Mask Setting", DisplayName = "Hide Preview"))
	void HidePreviewDebug();

	UFUNCTION()
	TArray<FName> GetTagOptions() const;

	UFUNCTION()
	TArray<FName> GetNestedWidgetOptions() const;


	void CreatePreviewLayer(const FGeometry& InViewportGeometry);
#endif

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, meta = (Category = "Guide Mask Setting", GetOptions = "GetTagOptions", AllowPrivateAccess = "true"))
	FName PreviewWidgetTag;

	UPROPERTY(EditInstanceOnly, meta = (Category = "Guide Mask Setting", GetOptions = "GetNestedWidgetOptions", AllowPrivateAccess = "true"))
	FName PreviewNestedWidget;

	UPROPERTY(VisibleInstanceOnly, meta = (Category = "Guide Mask Setting", AllowPrivateAccess = "true", DisplayAfter = "TagWidgetList"))
	TArray<FGuideTreeNode> GuideWidgetTree {};

#endif

	UPROPERTY(EditInstanceOnly, meta = (Category = "Guide Mask Setting", AllowPrivateAccess = "true"))
	TMap<FName, UWidget*> TagWidgetList;

private:
	TSharedPtr<SOverlay> Overlay;
	
	UPROPERTY(Transient)
	UWidget* LayerContent = nullptr;

};
