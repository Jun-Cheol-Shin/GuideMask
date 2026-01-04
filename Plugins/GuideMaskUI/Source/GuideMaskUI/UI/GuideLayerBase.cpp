// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideLayerBase.h"

#include "Components/SizeBoxSlot.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"

#include "Blueprint/WidgetLayoutLibrary.h"

#include "../GuideMaskSettings.h"

#if WITH_EDITOR
void UGuideLayerBase::SetPreviewGuide(const FGeometry& InViewportGeometry, UWidget* InWidget)
{
	SetGuideInternal(InViewportGeometry, InWidget);

	// Get target location
	FVector2D TargetLocalPosition = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().AbsolutePosition);
	FVector2D TargetLocation = InViewportGeometry.GetLocalPositionAtCoordinates(FVector2D(0, 0)) + TargetLocalPosition;

	// Get target size
	FVector2D TargetLocalBottomRight = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().LocalToAbsolute(InWidget->GetTickSpaceGeometry().GetLocalSize()));
	FVector2D TargetLocalTopLeft = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().GetAbsolutePosition());
	FVector2D TargetLocalSize = TargetLocalBottomRight - TargetLocalTopLeft;

	const FVector2D GuideWidgetPosition = TargetLocation - FVector2D(GuideBoxOffset.Left, GuideBoxOffset.Top);
	const FVector2D GuideWidgetSize = TargetLocalSize + FVector2D(GuideBoxOffset.Left + GuideBoxOffset.Right, GuideBoxOffset.Top + GuideBoxOffset.Bottom);

	if (IsDesignTime() && OnPreviewGuideLayerFunc)
	{
		OnPreviewGuideLayerFunc(GuideWidgetPosition, GuideWidgetSize);
	}

	BP_OnPreviewGuide(GuideWidgetPosition, GuideWidgetSize);
}

#endif


FReply UGuideLayerBase::OnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (nullptr != BoxBaseWidget && nullptr != GuideBoxPanel && ESlateVisibility::Collapsed == GuideBoxPanel->GetVisibility())
	{
		BoxBaseWidget->ForcedEndAction();
	}

	return FReply::Handled();
}

FReply UGuideLayerBase::OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	if (nullptr != BoxBaseWidget && nullptr != GuideBoxPanel && ESlateVisibility::Collapsed == GuideBoxPanel->GetVisibility())
	{
		BoxBaseWidget->ForcedEndAction();
	}

	return FReply::Handled();
}

FReply UGuideLayerBase::OnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	if (nullptr != BoxBaseWidget && nullptr != GuideBoxPanel && ESlateVisibility::Collapsed == GuideBoxPanel->GetVisibility())
	{
		BoxBaseWidget->ForcedEndAction();
	}

	return FReply::Handled();
}

void UGuideLayerBase::SetGuide(UWidget* InWidget, const FGuideBoxActionParameters& InParam)
{
	if (nullptr == InWidget)
	{
		return;
	}

	GuideWidget = InWidget;
	FGeometry ViewportGeo = UWidgetLayoutLibrary::GetViewportWidgetGeometry(GetWorld());
	SetGuideInternal(ViewportGeo, InWidget);

	if (nullptr != BoxBaseWidget && InParam.ActionType != EGuideActionType::None_Action)
	{
		BoxBaseWidget->SetGuideWidget(InWidget, InParam);

		if (nullptr != GuideBoxPanel)
		{
			GuideBoxPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	else
	{
		if (nullptr != GuideBoxPanel)
		{
			GuideBoxPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	OnStartGuide(InWidget, InParam);
}

void UGuideLayerBase::SetGuideInternal(const FGeometry& InViewportGeometry, UWidget* InWidget)
{
	if (nullptr == LayerPanel || nullptr == InWidget) return;

	ForceLayoutPrepass();
	InWidget->ForceLayoutPrepass();

	// Get target location
	FVector2D TargetLocalPosition = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().AbsolutePosition);
	FVector2D TargetLocation = InViewportGeometry.GetLocalPositionAtCoordinates(FVector2D(0, 0)) + TargetLocalPosition;

	// Get screen size
	FVector2D ScreenSize = InViewportGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5, 0.5)) * 2.f;

	// Get target size
	FVector2D TargetLocalBottomRight = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().LocalToAbsolute(InWidget->GetTickSpaceGeometry().GetLocalSize()));
	FVector2D TargetLocalTopLeft = InViewportGeometry.AbsoluteToLocal(InWidget->GetTickSpaceGeometry().GetAbsolutePosition());
	FVector2D TargetLocalSize = TargetLocalBottomRight - TargetLocalTopLeft;

	const FVector2D GuideWidgetPosition = TargetLocation - FVector2D(GuideBoxOffset.Left, GuideBoxOffset.Top);
	const FVector2D GuideWidgetSize = TargetLocalSize + FVector2D(GuideBoxOffset.Left + GuideBoxOffset.Right, GuideBoxOffset.Top + GuideBoxOffset.Bottom);

	if (nullptr != GuideBoxPanel)
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(GuideBoxPanel->Slot))
		{
			PanelSlot->SetAnchors(FAnchors(0, 0, 0, 0));
			PanelSlot->SetSize(GuideWidgetPosition);
			PanelSlot->SetPosition(GuideWidgetSize);
		}
	}

	FVector2D WidgetLeftTop = FVector2D(GuideWidgetPosition.X + GuideWidgetSize.X * 0.5f, GuideWidgetPosition.Y + GuideWidgetSize.Y * 0.5f);
	FVector2D WidgetCenter_Pixel = WidgetLeftTop;
	FVector2D WidgetSize_Pixel = GuideWidgetSize * 0.5f;

	// UV 변환
	FVector2D CenterUV = WidgetCenter_Pixel / ScreenSize;
	FVector2D SizeUV = WidgetSize_Pixel / ScreenSize;

	// 머티리얼 파라미터로 넘기기
	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetVectorParameterValue("Center", FLinearColor(CenterUV.X, CenterUV.Y, 0, 0));
		MaterialInstance->SetVectorParameterValue("Size", FLinearColor(SizeUV.X, SizeUV.Y, 0, 0));
	}


}

void UGuideLayerBase::SetEnableAnim(bool bIsEnable)
{
	bAnimated = bIsEnable;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("Animate"), true == bAnimated ? 1.f : 0.f);
		MaterialInstance->SetScalarParameterValue(TEXT("AnimSpeed"), true == bAnimated ? 1.f : 0.f);
	}
}


void UGuideLayerBase::SetCircularShape(bool bIsEnable)
{
	bShapeCircle = bIsEnable;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("Shape"), true == bShapeCircle ? 1.f : 0.f);
	}
}

void UGuideLayerBase::SetOpacity(float InOpacity)
{
	Opacity = InOpacity;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	}
}

void UGuideLayerBase::SetBoxOffset(const FMargin& InMargin)
{
	GuideBoxOffset = InMargin;

	if (true == GuideWidget.IsValid())
	{
		SetGuide(GuideWidget.Get());
	}
}

void UGuideLayerBase::NativeConstruct()
{
	Super::NativeConstruct();

	MaterialInstance = BlackScreen->GetDynamicMaterial();

	if (nullptr != LayerPanel)
	{
		LayerPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (nullptr != BlackScreen)
	{
		BlackScreen->SetVisibility(ESlateVisibility::Visible);
	}

	if (nullptr != GuideBoxPanel)
	{
		GuideBoxPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	const UGuideMaskSettings* Settings = GetDefault<UGuideMaskSettings>();
	if (ensureAlways(Settings))
	{
		if (!ensureAlwaysMsgf(Settings->DefaultBox.ToSoftObjectPath().IsValid(), 
			TEXT("Invalid Box base class in the project settings.")))
		{
			return;
		}

		TSubclassOf<UGuideBoxBase> BoxBaseClass = Settings->DefaultBox.LoadSynchronous();

		BoxBaseWidget = CreateWidget<UGuideBoxBase>(this, BoxBaseClass);

		if (ensure(BoxBaseWidget))
		{
			if (USizeBoxSlot* PanelSlot = Cast<USizeBoxSlot>(GuideBoxPanel->AddChild(BoxBaseWidget)))
			{
				PanelSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				PanelSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			}


			BoxBaseWidget->SetVisibility(ESlateVisibility::Visible);
			BoxBaseWidget->OnCompleteActionEvent.AddDynamic(this, &UGuideLayerBase::OnEndGuide);
		}
	}


	FViewport::ViewportResizedEvent.AddUObject(this, &UGuideLayerBase::OnResizedViewport);
}

void UGuideLayerBase::NativeDestruct()
{
	FViewport::ViewportResizedEvent.RemoveAll(this);
	MaterialInstance = nullptr;

	Super::NativeDestruct();
}

FReply UGuideLayerBase::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return OnKeyUp(InGeometry, InKeyEvent);
}

FReply UGuideLayerBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{

	return OnMouseButtonUp(InGeometry, InEvent);
}

FReply UGuideLayerBase::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	// Mobile Mode
	return OnTouchEnded(InGeometry, InEvent);
}

void UGuideLayerBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (BlackScreen && nullptr == MaterialInstance)
	{
		MaterialInstance = BlackScreen->GetDynamicMaterial();
	}


#if WITH_EDITOR
	if (nullptr != MaterialInstance)
	{
		SetEnableAnim(bAnimated);
		SetCircularShape(bShapeCircle);
		SetOpacity(Opacity);

		const FVector2D GuideWidgetPosition = ScreenPosition - FVector2D(GuideBoxOffset.Left, GuideBoxOffset.Top);
		const FVector2D GuideWidgetSize = GuideSize + FVector2D(GuideBoxOffset.Left + GuideBoxOffset.Right, GuideBoxOffset.Top + GuideBoxOffset.Bottom);

		FVector2D WidgetLeftTop = FVector2D(GuideWidgetPosition.X + GuideWidgetSize.X * 0.5f, GuideWidgetPosition.Y + GuideWidgetSize.Y * 0.5f);
		FVector2D WidgetCenter_Pixel = WidgetLeftTop;
		FVector2D WidgetSize_Pixel = GuideWidgetSize * 0.5f;

		// UV 변환
		FVector2D CenterUV = WidgetCenter_Pixel / FVector2D(1920, 1080);
		FVector2D SizeUV = WidgetSize_Pixel / FVector2D(1920, 1080);

		// 머티리얼 파라미터로 넘기기
		if (ensure(MaterialInstance))
		{
			MaterialInstance->SetVectorParameterValue("Center", FLinearColor(CenterUV.X, CenterUV.Y, 0, 0));
			MaterialInstance->SetVectorParameterValue("Size", FLinearColor(SizeUV.X, SizeUV.Y, 0, 0));
		}
	}
#endif
}


void UGuideLayerBase::OnResizedViewport(FViewport* InViewport, uint32 InMessage)
{
	if (true == GuideWidget.IsValid())
	{
		SetGuide(GuideWidget.Get());
	}
}
