// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideBoxBase.h"

//#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"

#include "Blueprint/WidgetLayoutLibrary.h"

void UGuideBoxBase::SetGuideWidget(UWidget* InWidget, const FGuideBoxActionParameters& InActionParam)
{
	ActionWidget = InWidget;

	/*if (nullptr != HoldProgressBar && nullptr != HoldProgressBar->GetParent())
	{
		HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
		HoldProgressBar->SetPercent(0.f);
	}*/

	SetGuideAction(InActionParam);
}

void UGuideBoxBase::SetGuideAction(const FGuideBoxActionParameters& InActionParam)
{
	ActionParam.ActionType = InActionParam.ActionType;
	ActionParam.ActivationKey = InActionParam.ActivationKey;

	if (true == IsDragType(InActionParam.ActionType))
	{
		ActionParam.DragThresholdVectorSize = InActionParam.DragThresholdVectorSize;
		ActionDPIScale = UWidgetLayoutLibrary::GetViewportScale(this);

		CorrectedDragThreshold = ActionParam.DragThresholdVectorSize * ActionDPIScale;
	}

	else if (EGuideActionType::Hold == InActionParam.ActionType)
	{
		ActionParam.HoldSeconds = InActionParam.HoldSeconds;
	}
}

FReply UGuideBoxBase::NativeOnStartClickAction(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	if (false == InGeometry.IsUnderLocation(InEvent.GetScreenSpacePosition()))
	{
		return FReply::Unhandled();
	}

	TouchStartPos = InGeometry.AbsoluteToLocal(InEvent.GetScreenSpacePosition());

	if (ActionWidget.IsValid())
	{
		if (UButton* ButtonWidget = Cast<UButton>(ActionWidget))
		{
			CachedClickMethod = ButtonWidget->GetClickMethod();
			CachedTouchMethod = ButtonWidget->GetTouchMethod();

			ButtonWidget->SetClickMethod(EButtonClickMethod::PreciseClick);
			ButtonWidget->SetTouchMethod(EButtonTouchMethod::PreciseTap);

			if (ActionWidget.IsValid())
			{
				TSharedRef<SWidget> ButtonSlateWidget = ButtonWidget->TakeWidget();
				if (ensure(&ButtonSlateWidget))
				{
					ButtonSlateWidget.Get().OnMouseButtonDown(InGeometry, InEvent);
				}
			}
		}

		else if (UCheckBox* CheckBoxWidget = Cast<UCheckBox>(ActionWidget))
		{
			CachedClickMethod = CheckBoxWidget->GetClickMethod();
			CachedTouchMethod = CheckBoxWidget->GetTouchMethod();

			CheckBoxWidget->SetClickMethod(EButtonClickMethod::PreciseClick);
			CheckBoxWidget->SetTouchMethod(EButtonTouchMethod::PreciseTap);

			TSharedRef<SWidget> CheckBoxSlateWidget = CheckBoxWidget->TakeWidget();
			if (ensure(&CheckBoxSlateWidget))
			{
				CheckBoxSlateWidget.Get().OnMouseButtonDown(InGeometry,
					CreateMouseLikePointerEventFromTouch(InEvent));
			}
		}

		else
		{
			TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
			if (ensure(&SlateWidget))
			{
				if (InEvent.IsTouchEvent())
				{
					SlateWidget->OnTouchStarted(InGeometry, InEvent);
				}

				else
				{
					SlateWidget->OnMouseButtonDown(InGeometry, InEvent);
				}
			}
		}

		if (OnMouseDownEvent.IsBound())
		{
			OnMouseDownEvent.Broadcast(InGeometry, InEvent);
		}

		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Unhandled();
}

FReply UGuideBoxBase::NativeOnMoveAction(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	if (TouchStartPos.IsZero())
	{
		return FReply::Unhandled();
	}

	float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);

	FVector2D CurrentPosition = InGeometry.AbsoluteToLocal(InEvent.GetScreenSpacePosition());
	FVector2D MoveVec = CurrentPosition - TouchStartPos;

	switch (ActionParam.ActionType)
	{
	case EGuideActionType::Drag:
	{
		if (ActionWidget.IsValid())
		{
			TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
			if (ensure(&SlateWidget))
			{
				SlateWidget->OnMouseMove(InGeometry, InEvent);
				SlateWidget->OnTouchMoved(InGeometry, InEvent);

				if (OnMouseMovedEvent.IsBound())
				{
					OnMouseMovedEvent.Broadcast(InGeometry, InEvent);
				}
			}
		}

		if (CorrectedDragThreshold <= MoveVec.Size())
		{
			UE_LOG(LogTemp, Warning, TEXT("Complete Drag!"));
			NativeOnEndAction(InEvent);
		}
	}
	break;
	case EGuideActionType::Swipe_Up:
	case EGuideActionType::Swipe_Down:
	case EGuideActionType::Swipe_Left:
	case EGuideActionType::Swipe_Right:
	{
		if (true == IsCorrectSwipe(MoveVec))
		{
			UE_LOG(LogTemp, Warning, TEXT("Success Swipe!"));

			if (ActionWidget.IsValid())
			{
				TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
				if (ensure(&SlateWidget))
				{
					SlateWidget->OnMouseMove(InGeometry, InEvent);
					SlateWidget->OnTouchMoved(InGeometry, InEvent);

					if (OnMouseMovedEvent.IsBound())
					{
						OnMouseMovedEvent.Broadcast(InGeometry, InEvent);
					}
				}
			}

			if (CorrectedDragThreshold <= MoveVec.Size())
			{
				UE_LOG(LogTemp, Warning, TEXT("Complete Drag!"));
				NativeOnEndAction(InEvent);
			}
		}

		else
		{
			TouchStartPos = FVector2D::Zero();
		}
	}
	break;
	default:
		break;
	}

	return FReply::Handled();
}

FReply UGuideBoxBase::NativeOnEndClickAction(const FGeometry& InGeometry, const FPointerEvent& InEvent)
{
	if (false == InGeometry.IsUnderLocation(InEvent.GetScreenSpacePosition()))
	{
		return FReply::Unhandled();
	}

	if (ActionWidget.IsValid())
	{
		if (UButton* ButtonWidget = Cast<UButton>(ActionWidget))
		{
			ButtonWidget->SetClickMethod(EButtonClickMethod::MouseUp);
			ButtonWidget->SetTouchMethod(EButtonTouchMethod::PreciseTap);

			TSharedRef<SWidget> ButtonSlateWidget = ButtonWidget->TakeWidget();
			if (ensure(&ButtonSlateWidget))
			{
				ButtonSlateWidget.Get().OnMouseButtonUp(InGeometry, InEvent);
			}

			ButtonWidget->SetClickMethod(CachedClickMethod);
			ButtonWidget->SetTouchMethod(CachedTouchMethod);
		}

		else if (UCheckBox* CheckBoxWidget = Cast<UCheckBox>(ActionWidget))
		{
			CheckBoxWidget->SetClickMethod(EButtonClickMethod::MouseUp);
			CheckBoxWidget->SetTouchMethod(EButtonTouchMethod::PreciseTap);

			TSharedRef<SWidget> CheckBoxSlateWidget = CheckBoxWidget->TakeWidget();
			if (ensure(&CheckBoxSlateWidget))
			{
				CheckBoxSlateWidget.Get().OnMouseButtonUp(InGeometry,
					CreateMouseLikePointerEventFromTouch(InEvent));
			}


			CheckBoxWidget->SetClickMethod(CachedClickMethod);
			CheckBoxWidget->SetTouchMethod(CachedTouchMethod);

		}

		else
		{
			TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
			if (ensure(&SlateWidget))
			{
				if (InEvent.IsTouchEvent())
				{
					SlateWidget->OnTouchEnded(InGeometry, InEvent);
				}

				else
				{
					SlateWidget->OnMouseButtonUp(InGeometry, InEvent);
				}
			}
		}


		if (OnMouseUpEvent.IsBound())
		{
			OnMouseUpEvent.Broadcast(InGeometry, InEvent);
		}

		NativeOnEndAction(InEvent);

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply UGuideBoxBase::NativeOnStartKeyAction(const FGeometry& InGeometry, const FKeyEvent& InEvent)
{
	TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
	if (ensure(&SlateWidget))
	{
		SlateWidget->OnKeyDown(InGeometry, InEvent);
	}

	if (OnKeyDownEvent.IsBound())
	{
		OnKeyDownEvent.Broadcast(InGeometry, InEvent);
	}

	return FReply::Handled();
}

FReply UGuideBoxBase::NativeOnEndKeyAction(const FGeometry& InGeometry, const FKeyEvent& InEvent)
{
	TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
	if (ensure(&SlateWidget))
	{
		SlateWidget->OnKeyUp(InGeometry, InEvent);
	}

	if (OnKeyUpEvent.IsBound())
	{
		OnKeyUpEvent.Broadcast(InGeometry, InEvent);
	}

	NativeOnEndAction();

	return FReply::Handled();
}

void UGuideBoxBase::NativeOnEndAction(const FPointerEvent& InEvent)
{
	if (false == InEvent.GetPressedButtons().IsEmpty())
	{
		NativeOnMouseLeave(InEvent);
	}

	if (ActionParam.WidgetActionEvent.IsBound())
	{
		ActionParam.WidgetActionEvent.Execute();
	}

	if (OnCompleteActionEvent.IsBound())
	{
		OnCompleteActionEvent.Broadcast();
	}

	Clear();
}


void UGuideBoxBase::NativeConstruct()
{
	Super::NativeConstruct();

	OnNativeVisibilityChanged.RemoveAll(this);
	OnNativeVisibilityChanged.AddUObject(this, &UGuideBoxBase::OnChangedVisibility);

	SetIsFocusable(true);
}

void UGuideBoxBase::NativeDestruct()
{
	Super::NativeDestruct();

	OnNativeVisibilityChanged.RemoveAll(this);
}

void UGuideBoxBase::NativeTick(const FGeometry& InGeometry, float InDeltaTime)
{
	Super::NativeTick(InGeometry, InDeltaTime);

	if (false == TouchStartPos.IsZero() &&
		ActionParam.ActionType == EGuideActionType::Hold)
	{
		if (FPlatformTime::Seconds() > StartTime + ActionParam.HoldSeconds)
		{
			NativeOnEndAction();
		}

		/*else if (nullptr != HoldProgressBar)
		{
			double Value = (FPlatformTime::Seconds() - StartTime) / HoldSeconds;

			HoldProgressBar->SetPercent(Value);
		}*/
	}
}

// PC
FReply UGuideBoxBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != ActionParam.ActivationKey)
	{
		return FReply::Unhandled();
	}

	else if (EGuideActionType::Hold == ActionParam.ActionType)
	{
		StartTime = FPlatformTime::Seconds();
		TouchStartPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
		{
			HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}*/

		return FReply::Handled();
	}

	return NativeOnStartClickAction(InGeometry, InMouseEvent);
}

FReply UGuideBoxBase::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (true == TouchStartPos.IsZero())
	{
		return FReply::Unhandled();
	}

	switch (ActionParam.ActionType)
	{
	case EGuideActionType::Drag:
	case EGuideActionType::Swipe_Up:
	case EGuideActionType::Swipe_Down:
	case EGuideActionType::Swipe_Left:
	case EGuideActionType::Swipe_Right:
	{
		NativeOnMoveAction(InGeometry, InMouseEvent);
	}
	break;
	default:
		break;
	}

	return FReply::Handled();
}

FReply UGuideBoxBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
	{
		HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
	}*/

	if (false == TouchStartPos.IsZero())
	{
		TouchStartPos = FVector2D::Zero();
		StartTime = 0.f;

		switch (ActionParam.ActionType)
		{
		case EGuideActionType::DownAndUp:
		{
			if (InMouseEvent.GetEffectingButton() == ActionParam.ActivationKey)
			{
				return NativeOnEndClickAction(InGeometry, InMouseEvent);
			}
		}
		break;
		default:
			break;
		}
	}

	return FReply::Unhandled();
}

// Mobile
FReply UGuideBoxBase::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (EGuideActionType::Hold == ActionParam.ActionType)
	{
		StartTime = FPlatformTime::Seconds();
		TouchStartPos = InGeometry.AbsoluteToLocal(InGestureEvent.GetScreenSpacePosition());

		/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
		{
			HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}*/

		return FReply::Handled();
	}

	return NativeOnStartClickAction(InGeometry, InGestureEvent);
}

FReply UGuideBoxBase::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (true == TouchStartPos.IsZero())
	{
		return FReply::Unhandled();
	}

	switch (ActionParam.ActionType)
	{
	case EGuideActionType::Drag:
	case EGuideActionType::Swipe_Up:
	case EGuideActionType::Swipe_Down:
	case EGuideActionType::Swipe_Left:
	case EGuideActionType::Swipe_Right:
	{
		NativeOnMoveAction(InGeometry, InGestureEvent);
	}
	break;
	default:
		break;
	}

	return FReply::Handled();
}

FReply UGuideBoxBase::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
	{
		HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
	}*/

	if (false == TouchStartPos.IsZero())
	{
		TouchStartPos = FVector2D::Zero();
		StartTime = 0.f;

		switch (ActionParam.ActionType)
		{
		case EGuideActionType::DownAndUp:
		{
			return NativeOnEndClickAction(InGeometry, InGestureEvent);
		}
		break;
		default:
			break;
		}
	}

	return FReply::Unhandled();
}

FReply UGuideBoxBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == ActionParam.ActivationKey)
	{
		if (EGuideActionType::Hold == ActionParam.ActionType && true == TouchStartPos.IsZero())
		{
			StartTime = FPlatformTime::Seconds();
			TouchStartPos = InGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());

			/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
			{
				HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}*/

			return NativeOnStartKeyAction(InGeometry, InKeyEvent);
		}

		else
		{
			return NativeOnStartKeyAction(InGeometry, InKeyEvent);
		}
	}

	return FReply::Unhandled();
}

FReply UGuideBoxBase::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
	{
		HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
	}*/

	if (EGuideActionType::Hold == ActionParam.ActionType && false == TouchStartPos.IsZero())
	{
		StartTime = 0.f;
		TouchStartPos = FVector2D::Zero();

		return FReply::Handled();
	}

	else if (InKeyEvent.GetKey() == ActionParam.ActivationKey)
	{
		return NativeOnEndKeyAction(InGeometry, InKeyEvent);
	}

	return FReply::Unhandled();
}

void UGuideBoxBase::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ActionWidget.IsValid())
	{
		TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
		if (ensure(&SlateWidget))
		{
			SlateWidget.Get().OnMouseEnter(InGeometry, InMouseEvent);
		}
	}
}

void UGuideBoxBase::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	/*if (nullptr != HoldProgressBar && HoldProgressBar->GetParent())
	{
		HoldProgressBar->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
	}*/

	if (ActionWidget.IsValid())
	{
		TSharedRef<SWidget> SlateWidget = ActionWidget->TakeWidget();
		if (ensure(&SlateWidget))
		{
			SlateWidget.Get().OnMouseLeave(InMouseEvent);
		}


		if (UButton* ButtonWidget = Cast<UButton>(ActionWidget))
		{
			ButtonWidget->SetClickMethod(CachedClickMethod);
			ButtonWidget->SetTouchMethod(CachedTouchMethod);
		}

		else if (UCheckBox* CheckBoxWidget = Cast<UCheckBox>(ActionWidget))
		{
			CheckBoxWidget->SetClickMethod(CachedClickMethod);
			CheckBoxWidget->SetTouchMethod(CachedTouchMethod);
		}
	}

	if (false == IsDragType(ActionParam.ActionType))
	{
		TouchStartPos = FVector2D::Zero();
	}

	if (EGuideActionType::Hold != ActionParam.ActionType)
	{
		StartTime = 0.f;
	}
}

void UGuideBoxBase::Clear()
{
	StartTime = 0.f;
	TouchStartPos = FVector2D::Zero();
	ActionWidget.Reset();

	ActionParam.WidgetActionEvent.Clear();
	ActionParam.ActionType = EGuideActionType::None_Action;
	ActionParam.DragThresholdVectorSize = 0.f;
	ActionParam.HoldSeconds = 0.f;
}

void UGuideBoxBase::OnChangedVisibility(ESlateVisibility InVisiblity)
{
	switch (InVisiblity)
	{
	case ESlateVisibility::Visible:
	case ESlateVisibility::HitTestInvisible:
	case ESlateVisibility::SelfHitTestInvisible:
	{
		FViewport::ViewportResizedEvent.AddUObject(this, &UGuideBoxBase::OnResizedViewport);
	}
	break;
	case ESlateVisibility::Collapsed:
	case ESlateVisibility::Hidden:
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}
	break;

	default:
		break;
	}
}

void UGuideBoxBase::OnResizedViewport(FViewport* InViewport, uint32 InWindowMode /*?*/)
{
	if (nullptr == InViewport || nullptr == InViewport->GetClient())
	{
		return;
	}

	ActionDPIScale = InViewport->GetClient()->GetDPIScale();
	CorrectedDragThreshold = ActionParam.DragThresholdVectorSize * ActionDPIScale;
}

FPointerEvent UGuideBoxBase::CreateMouseLikePointerEventFromTouch(const FPointerEvent& InTouchEvent)
{
	return FPointerEvent(
		InTouchEvent.GetPointerIndex(),
		InTouchEvent.GetScreenSpacePosition(),
		InTouchEvent.GetLastScreenSpacePosition(),
		{},
		EKeys::LeftMouseButton,
		InTouchEvent.GetWheelDelta(),
		InTouchEvent.GetModifierKeys()
	);
}

void UGuideBoxBase::ForcedEndAction()
{
	if (ActionParam.WidgetActionEvent.IsBound())
	{
		ActionParam.WidgetActionEvent.Execute();
	}

	if (OnCompleteActionEvent.IsBound())
	{
		OnCompleteActionEvent.Broadcast();
	}

	Clear();
}

bool UGuideBoxBase::IsDragType(EGuideActionType InType) const
{
	return  InType == EGuideActionType::Drag ||
			InType == EGuideActionType::Swipe_Up ||
			InType == EGuideActionType::Swipe_Down ||
			InType == EGuideActionType::Swipe_Left ||
			InType == EGuideActionType::Swipe_Right;
}

bool UGuideBoxBase::IsCorrectSwipe(const FVector2D& InMoveVec)
{
	float XValue = FMath::Abs(InMoveVec.X);
	float YValue = FMath::Abs(InMoveVec.Y);

	switch (ActionParam.ActionType)
	{
	case EGuideActionType::Swipe_Up:
	{
		if (XValue <= YValue && InMoveVec.Y < 0)
		{
			return true;
		}
	}
	break;
	case EGuideActionType::Swipe_Down:
	{
		if (XValue <= YValue && InMoveVec.Y > 0)
		{
			return true;
		}
	}
	break;
	case EGuideActionType::Swipe_Left:
	{
		if (XValue >= YValue && InMoveVec.X < 0)
		{
			return true;
		}
	}
	break;
	case EGuideActionType::Swipe_Right:
	{
		if (XValue >= YValue && InMoveVec.X > 0)
		{
			return true;
		}
	}
	break;
	default:
		break;
	}

	return false;
}

