// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleHUD.h"
#include "SampleListEntry.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/ComboBoxString.h"
#include "Components/SpinBox.h"
#include "Components/DynamicEntryBox.h"

#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#include "GuideMaskUI/UI/GuideMaskRegister.h"
#include "GuideMaskUI/GuideMaskUIFunctionLibrary.h"


TArray<FGuideNodePathParam> USampleHUD::GetNodeParam() const
{
	TArray<FGuideNodePathParam> RetVal;

	if (!ensureAlways(TagComboBox && ScopeWidgetComboBox && NestedWidgetComboBox))
	{
		return RetVal;
	}

	FName SelectedTag = FName(TagComboBox->GetSelectedOption());

	UGuideMaskRegister* Register = UGuideMaskUIFunctionLibrary::GetRegister(GetWorld(), SelectedTag);
	if (nullptr == Register)
	{
		return RetVal;
	}

	TArray<FGuideTreeNode> NewTree;
	Register->GetGuideWidgetTree(OUT NewTree, SelectedTag);

	int ScopeIndex = ScopeWidgetComboBox->GetSelectedIndex();
	int NestedIndex = NestedWidgetComboBox->GetSelectedIndex();

	UWidget* TempScope = nullptr;

	for (int i = ScopeIndex; i >= 0; --i)
	{
		if (!ensureAlways(NewTree.IsValidIndex(i)))
		{
			return RetVal;
		}

		FGuideTreeNode Node = NewTree[i];

		FGuideNodePathParam NewParam;

		if (nullptr != TempScope)
		{
			NewParam.NestedWidgetIndex = Node.NestedWidgets.IndexOfByPredicate([&TempScope](UWidget* InWidget)
				{
					return InWidget == TempScope;
				});
		}

		else
		{
			NewParam.NestedWidgetIndex = NestedIndex;
		}


		if (UListView* ListView = Cast<UListView>(Node.Scope))
		{
			NewParam.OnGetDynamicEvent.BindDynamic(this, &USampleHUD::OnGetListItem);
			RetVal.Emplace(NewParam);
		}

		else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Node.Scope))
		{
			NewParam.OnGetDynamicEvent.BindDynamic(this, &USampleHUD::OnGetDynamicEntry);
			RetVal.Emplace(NewParam);
		}

		TempScope = Node.Scope;
	}

	if (false == RetVal.IsEmpty())
	{
		Algo::Reverse(RetVal);
	}


	return RetVal;
}

FName USampleHUD::GetSelectedTag() const
{
	if (nullptr != TagComboBox)
	{
		return FName(TagComboBox->GetSelectedOption());
	}

	return FName();
}

UObject* USampleHUD::GetSampleListItem(UWidget* InTagWidget) const
{
	if (UListView* ListView = Cast<UListView>(InTagWidget))
	{
		UObject* const* FoundItem = SampleListView->GetListItems().FindByPredicate([](UObject* InItem)
			{
				if (USampleListItem* ListItem = Cast<USampleListItem>(InItem))
				{
					return ListItem->ItemId == 10;
				}

				return false;
			});

		return nullptr != FoundItem && nullptr != *FoundItem ? *FoundItem : nullptr;
	}

	return nullptr;
}

void USampleHUD::NativeConstruct()
{
	Super::NativeConstruct();

	/*
	if (nullptr != GuideButton)
	{
		GuideButton->OnClicked.AddDynamic(this, &USampleHUD::OnClickedGuide);
	}
	*/

	if (nullptr != ScopeWidgetComboBox)
	{
		ScopeWidgetComboBox->OnSelectionChanged.AddDynamic(this, &USampleHUD::OnScopeWidgetSelectionChanged);
	}

	if (nullptr != NestedWidgetComboBox)
	{
		NestedWidgetComboBox->OnSelectionChanged.AddDynamic(this, &USampleHUD::OnNestedWidgetSelectionChanged);
	}

	if (nullptr != ActionKeyComboBox)
	{
		ActionKeyComboBox->OnSelectionChanged.AddDynamic(this, &USampleHUD::OnActionKeySelectionChanged);

		if (DisplayToKey.Contains(ActionKeyComboBox->GetSelectedOption()))
		{
			SelectedKey = DisplayToKey[ActionKeyComboBox->GetSelectedOption()];
		}
	}

	if (nullptr != DragValueBox)
	{
		DragValueBox->OnValueChanged.AddDynamic(this, &USampleHUD::OnChangedDragValue);
		DragValue = DragValueBox->GetValue();
	}

	if (nullptr != HoldTimeBox)
	{
		HoldTimeBox->OnValueChanged.AddDynamic(this, &USampleHUD::OnChangedHoldTime);
		HoldTime = HoldTimeBox->GetValue();
	}

	Initialize_SampleListView();
	Initialize_TagComboBox();
	Initialize_ActionTypeComboBox();
}

void USampleHUD::NativeDestruct()
{
	Super::NativeDestruct();

	if (nullptr != SampleListView)
	{
		SampleListView->ClearListItems();
	}
}

void USampleHUD::OnTagSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType)
{
	if (nullptr != ScopeWidgetComboBox)
	{
		ScopeWidgetComboBox->ClearOptions();

		UGuideMaskRegister* Register = UGuideMaskUIFunctionLibrary::GetRegister(GetWorld(), FName(InSelectedItem));
		if (nullptr == Register)
		{
			return;
		}

		TArray<FGuideTreeNode> NewTree;
		Register->GetGuideWidgetTree(NewTree, FName(InSelectedItem));

		for (int i = 0; i < NewTree.Num(); ++i)
		{
			if (false == NewTree.IsValidIndex(i))
			{
				continue;
			}

			if (nullptr == NewTree[i].Scope)
			{
				continue;
			}

			ScopeWidgetComboBox->AddOption(NewTree[i].Scope->GetName());
		}

		ScopeWidgetComboBox->SetSelectedIndex(0);
	}
}

void USampleHUD::OnScopeWidgetSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType)
{
	if (!ensureAlways(NestedWidgetComboBox && ScopeWidgetComboBox && TagComboBox))
	{
		return;
	}


	NestedWidgetComboBox->ClearOptions();

	FName Tag = FName(TagComboBox->GetSelectedOption());
	int ScopeIndex = ScopeWidgetComboBox->GetSelectedIndex();

	UGuideMaskRegister* Register = UGuideMaskUIFunctionLibrary::GetRegister(GetWorld(), Tag);
	if (nullptr == Register)
	{
		return;
	}

	TArray<FGuideTreeNode> NewTree;
	Register->GetGuideWidgetTree(NewTree, FName(TagComboBox->GetSelectedOption()));

	if (NewTree.IsValidIndex(ScopeIndex))
	{
		FGuideTreeNode SelectedNode = NewTree[ScopeIndex];

		for (int i = 0; i < SelectedNode.NestedWidgets.Num(); ++i)
		{
			UWidget* ChildWidget = SelectedNode.NestedWidgets[i];
			if (nullptr == ChildWidget)
			{
				continue;
			}

			NestedWidgetComboBox->AddOption(ChildWidget->GetName());
		}

		NestedWidgetComboBox->SetSelectedIndex(0);
	}
}

void USampleHUD::OnNestedWidgetSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType)
{

}

void USampleHUD::OnActionTypeSelectionChanged(FString InSelectedItem, ESelectInfo::Type InType)
{
	if (nullptr != ActionKeyComboBox)
	{
		DisplayToKey.Reset();
		ActionKeyComboBox->ClearOptions();

		uint8 Value = StaticEnum<EGuideActionType>()->GetValueByName(FName(InSelectedItem));
		EGuideActionType Type = static_cast<EGuideActionType>(Value);

		switch (Type)
		{
		case EGuideActionType::DownAndUp:
		case EGuideActionType::Hold:
		{
			TArray<FKey> AllKeys;
			EKeys::GetAllKeys(AllKeys);

			for (FKey Key : AllKeys)
			{
				if (!UKismetInputLibrary::Key_IsValid(Key))
				{
					continue;
				}

				if (Key == EKeys::AnyKey)
				{
					continue;
				}

				if (Key.IsDeprecated() || Key.IsVirtual() || Key.IsTouch())
				{
					continue;
				}

				if (!Key.IsBindableInBlueprints())
				{
					continue;
				}

				if (UKismetInputLibrary::Key_IsGamepadKey(Key) ||
					UKismetInputLibrary::Key_IsAnalog(Key) ||
					UKismetInputLibrary::Key_IsAxis1D(Key) ||
					UKismetInputLibrary::Key_IsAxis2D(Key) ||
					UKismetInputLibrary::Key_IsAxis3D(Key) ||
					UKismetInputLibrary::Key_IsButtonAxis(Key))
				{
					continue;            
				}

				const FText DisplayText = Key.GetDisplayName();
				FString Display = DisplayText.ToString();

				DisplayToKey.Emplace(Display, Key);
				ActionKeyComboBox->AddOption(Display);
			}

		}
		break;

		default:
		case EGuideActionType::None_Action:
		case EGuideActionType::Drag:
		case EGuideActionType::Swipe_Up:
		case EGuideActionType::Swipe_Down:
		case EGuideActionType::Swipe_Left:
		case EGuideActionType::Swipe_Right:
		{
			//
		}
		break;
		}

		ActionType = Type;
		
		ActionKeyComboBox->SetSelectedIndex(0);
	}
}

void USampleHUD::OnActionKeySelectionChanged(FString InSelectedItem, ESelectInfo::Type InType)
{
	if (DisplayToKey.Contains(InSelectedItem))
	{
		SelectedKey = DisplayToKey[InSelectedItem];
	}
}

void USampleHUD::OnChangedDragValue(float InValue)
{
	DragValue = InValue;
}

void USampleHUD::OnChangedHoldTime(float InValue)
{
	HoldTime = InValue;
}

bool USampleHUD::OnGetListItem(UObject* InItem)
{
	if (USampleListItem* ListItem = Cast<USampleListItem>(InItem))
	{
		return ListItem->ItemId == 10;
	}

	return false;
}

void USampleHUD::Initialize_ActionTypeComboBox()
{
	if (nullptr != ActionTypeComboBox)
	{
		ActionTypeComboBox->OnSelectionChanged.AddDynamic(this, &USampleHUD::OnActionTypeSelectionChanged);
		for (uint8 i = 0; i < static_cast<uint8>(EGuideActionType::None_Action) - 1; ++i)
		{
			FString String = StaticEnum<EGuideActionType>()->GetNameStringByValue(i);
			ActionTypeComboBox->AddOption(String);
		}

		ActionTypeComboBox->SetSelectedIndex(0);
		ActionType = static_cast<EGuideActionType>(0);
	}
}
void USampleHUD::Initialize_SampleListView()
{
	if (nullptr != SampleListView)
	{
		TArray<UObject*> ListItems;

		for (int i = 1; i <= 50; ++i)
		{
			USampleListItem* ListItem = NewObject<USampleListItem>();
			ListItem->ItemId = i;

			ListItems.Emplace(ListItem);
		}

		SampleListView->SetListItems(ListItems);
	}

}
void USampleHUD::Initialize_TagComboBox()
{
	if (nullptr != TagComboBox)
	{
		TArray<UGuideMaskRegister*> Registers;
		UGuideMaskUIFunctionLibrary::GetAllGuideRegisters(GetWorld(), OUT Registers);

		for (int i = 0; i < Registers.Num(); ++i)
		{
			UGuideMaskRegister* Register = Registers[i];
			if (nullptr == Register)
			{
				continue;
			}

			TArray<FName> TagList = Register->GetTagList();

			for (int j = 0; j < TagList.Num(); ++j)
			{
				FName Tag = TagList[j];
				TagComboBox->AddOption(Tag.ToString());
			}
		}

		TagComboBox->OnSelectionChanged.AddDynamic(this, &USampleHUD::OnTagSelectionChanged);
		TagComboBox->SetSelectedIndex(0);
	}
}


// goto Blueprint graph
/*
void USampleHUD::OnClickedGuide()
{
	// Guide Setting Sample Code

	if (nullptr != WarningText)
	{
		WarningText->SetVisibility(ESlateVisibility::Collapsed);
	}

	FString TagString = TagComboBox->GetSelectedOption();
	FString LevelString = LevelComboBox->GetSelectedOption();

	if (LevelString.IsNumeric())
	{
		int32 Level = UKismetStringLibrary::Conv_StringToInt(LevelString);
		UWidget* Widget = UGuideMaskUIFunctionLibrary::GetTagWidget(GetWorld(), FName(TagString), Level);

		if (UListView* ListView = Cast<UListView>(Widget))
		{
			UObject* const* FoundItem = SampleListView->GetListItems().FindByPredicate([](UObject* InItem)
				{
					if (USampleListItem* ListItem = Cast<USampleListItem>(InItem))
					{
						return ListItem->ItemId == 10;
					}

					return false;
				});

			if (FoundItem && *FoundItem)
			{
				//UGuideMaskUIFunctionLibrary::ShowGuideListEntry(GetWorld(), ListView, *FoundItem, 5.f);
			}

			else
			{
				if (nullptr != WarningText)
				{
					WarningText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				}
			}
		}

		else if (nullptr != Widget)
		{
			//UGuideMaskUIFunctionLibrary::ShowGuideWidget(GetWorld(), Widget);
		}

		else
		{
			if (nullptr != WarningText)
			{
				WarningText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
		}
	}
	
}
*/
