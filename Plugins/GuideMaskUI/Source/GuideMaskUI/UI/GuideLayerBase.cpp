// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideLayerBase.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"

void UGuideLayerBase::SetGuide(const FGeometry& InViewportGeometry, UWidget* InWidget)
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

	SetGuideLayer(ScreenSize, TargetLocation, TargetLocalSize);
	SetGuideBox(InWidget);
}

void UGuideLayerBase::SetEnableAnim(bool bIsEnable)
{
	bAnimated = bIsEnable;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("Animate"), true == bAnimated ? 1.f : 0.f);
	}
}

void UGuideLayerBase::SetAnimRate(float InRate)
{
	AnimationRate = InRate;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("AnimSpeed"), AnimationRate);
	}
}

void UGuideLayerBase::SetCircularShape(bool bIsEnable)
{
	bUseCircle = bIsEnable;

	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetScalarParameterValue(TEXT("Shape"), true == bUseCircle ? 1.f : 0.f);
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

void UGuideLayerBase::SetGuideLayer(const FVector2D& InScreenSize, const FVector2D& InTargetLoc, const FVector2D& InTargetSize)
{
	if (nullptr != GuideBoxPanel)
	{
		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(GuideBoxPanel->Slot))
		{
			PanelSlot->SetAnchors(FAnchors(0, 0, 0, 0));
			PanelSlot->SetSize(InTargetSize);
			PanelSlot->SetPosition(InTargetLoc);
		}
	}

	SetMaterialTransform(InScreenSize, InTargetLoc, InTargetSize);

	if (GuideBoxPanel)
	{
		if (UCanvasPanelSlot* BoxSlot = Cast<UCanvasPanelSlot>(GuideBoxPanel->Slot))
		{
			BoxSlot->SetSize(InTargetSize /* + InLayerParam.Padding */);
			BoxSlot->SetPosition(InTargetLoc /*- InLayerParam.Padding * 0.5f */);
		}
	}	
}

void UGuideLayerBase::SetMaterialTransform(const FVector2D& InViewportSize, const FVector2D& InPosiiton, const FVector2D& InWidgetSize)
{
	// 실제 픽셀 좌표로 변환
	FVector2D WidgetLeftTop = FVector2D(InPosiiton.X + InWidgetSize.X * 0.5f, InPosiiton.Y + InWidgetSize.Y * 0.5f);
	FVector2D WidgetSize = InWidgetSize;

	/*(false == InOffset.IsZero())
	{
		WidgetSize += InOffset;
	}*/

	FVector2D WidgetCenter_Pixel = WidgetLeftTop;
	FVector2D WidgetSize_Pixel = WidgetSize * 0.5f;

	// UV 변환
	FVector2D CenterUV = WidgetCenter_Pixel / InViewportSize;
	FVector2D SizeUV = WidgetSize_Pixel / InViewportSize;

	// 머티리얼 파라미터로 넘기기
	if (ensure(MaterialInstance))
	{
		MaterialInstance->SetVectorParameterValue("Center", FLinearColor(CenterUV.X, CenterUV.Y, 0, 0));
		MaterialInstance->SetVectorParameterValue("Size", FLinearColor(SizeUV.X, SizeUV.Y, 0, 0));
	}
}

void UGuideLayerBase::SetGuideBox(UWidget* InWidget)
{

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
		GuideBoxPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	/*if (nullptr != GuideMaskBox)
	{
		GuideMaskBox->SetVisibility(ESlateVisibility::Visible);
		GuideMaskBox->OnPostAction.BindUObject(this, &UUIGuideLayer::OnPostAction);
	}*/


}

void UGuideLayerBase::NativeDestruct()
{
	MaterialInstance = nullptr;

	Super::NativeDestruct();
}

FReply UGuideLayerBase::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);

	/**if (nullptr != GuideBoxPanel && ESlateVisibility::HitTestInvisible == GuideBoxPanel->GetVisibility())
	{
		GuideMaskBox->ForceComplete();
	}*/

}

FReply UGuideLayerBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	return Super::NativeOnMouseButtonUp(InGeometry, InEvent);

	/*if (nullptr != GuideBoxPanel && ESlateVisibility::HitTestInvisible == GuideBoxPanel->GetVisibility())
	{
		GuideMaskBox->ForceComplete();
	}*/

}

FReply UGuideLayerBase::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	return Super::NativeOnTouchEnded(InGeometry, InEvent);

	// Mobile Mode

	/*if (nullptr != GuideBoxPanel && ESlateVisibility::HitTestInvisible == GuideBoxPanel->GetVisibility())
	{
		GuideMaskBox->ForceComplete();
	}*/


}

void UGuideLayerBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (BlackScreen && nullptr == MaterialInstance)
	{
		MaterialInstance = BlackScreen->GetDynamicMaterial();
	}
}


void UGuideLayerBase::OnResizedViewport(FViewport* InViewport, uint32 InMessage)
{

}



