// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideMaskUIFunctionLibrary.h"
#include "GuideListEntryAsyncAction.h"

#include "../GuideMaskUI/UI/GuideMaskRegister.h"
#include "../GuideMaskUI/UI/GuideLayerBase.h"
#include "../GuideMaskUI/GuideMaskSettings.h"
#include "../GuideMaskUI/EntryGuideIdentifiable.h"

#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

#include "Components/ListView.h"
#include "Components/DynamicEntryBox.h"

void UGuideMaskUIFunctionLibrary::ShowGuideWidget(const UObject* WorldContextObject, UWidget* InTagWidget, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder)
{
	if (nullptr == WorldContextObject)
	{
		return;
	}

	const UGuideMaskSettings* Settings = GetDefault<UGuideMaskSettings>();
	if (ensureAlways(Settings) && Settings->DefaultLayer.ToSoftObjectPath().IsValid())
	{
		TSubclassOf<UGuideLayerBase> WidgetClass = Settings->DefaultLayer.LoadSynchronous();
		UGuideLayerBase* GuideLayer = CreateWidget<UGuideLayerBase>(WorldContextObject->GetWorld(), WidgetClass);
		
		GuideLayer->AddToViewport(InLayerZOrder);

		if (ensure(GuideLayer))
		{
			GuideLayer->SetGuide(InTagWidget, InActionParam);
		}
	}
}

void UGuideMaskUIFunctionLibrary::ShowGuideListEntry(const UObject* WorldContextObject, UListView* InTagListView, UObject* InListItem, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder, float InAsyncTimeout)
{
	if (nullptr == WorldContextObject)
	{
		return;
	}

	if (UGuideListEntryAsyncAction* AsyncAction = 
		UGuideListEntryAsyncAction::Create(WorldContextObject->GetWorld(), 
			InTagListView, 
			InListItem, 
			InAsyncTimeout))
	{
		AsyncAction->OnReadyNative.AddWeakLambda(WorldContextObject,
			[InActionParam, InLayerZOrder](const UObject* InWorldContextObject, UUserWidget* InEntryWidget)
			{
				UGuideMaskUIFunctionLibrary::ShowGuideWidget(InWorldContextObject, InEntryWidget, InActionParam, InLayerZOrder);
			});

		AsyncAction->Activate();
	}

}


void UGuideMaskUIFunctionLibrary::ShowGuideDynamicWidget(const UObject* WorldContextObject, UWidget* InWidget, const TArray<FGuideDynamicWidgetPath>& InPath, const FGuideBoxActionParameters& InActionParam, int InLayerZOrder, float InAsyncTimeout)
{
	if (nullptr == WorldContextObject)
	{
		return;
	}

	if (true == InPath.IsEmpty())
	{
		ShowGuideWidget(WorldContextObject, InWidget, InActionParam, InLayerZOrder);
		return;
	}

	TArray<FGuideDynamicWidgetPath> NewPath;
	for (int i = 1; i < InPath.Num(); ++i)
	{
		NewPath.Emplace(InPath[i]);
	}

	FGuideDynamicWidgetPath CurrentPath = InPath[0];
	if (UListView* ListView = Cast<UListView>(InWidget))
	{
		UObject* const* ListItem = ListView->GetListItems().FindByPredicate([Event = CurrentPath.OnGetDynamicEvent](UObject* InItem) -> bool
			{
				return true == Event.IsBound() ? Event.Execute(InItem) : false;
			});

		if (ListItem && *ListItem)
		{
			if (UGuideListEntryAsyncAction* AsyncAction =
				UGuideListEntryAsyncAction::Create(WorldContextObject->GetWorld(),
					ListView,
					*ListItem,
					InAsyncTimeout))
			{
				AsyncAction->OnReadyNative.AddWeakLambda(WorldContextObject,
					[NewPath, ChildIndex = CurrentPath.NextSearchChildIndex, InActionParam, InLayerZOrder, InAsyncTimeout](const UObject* InWorldContextObject, UUserWidget* InEntryWidget)
					{
						if (nullptr == InEntryWidget)
						{
							return;
						}

						TArray<UWidget*> Childs;
						if (true == InEntryWidget->GetClass()->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
						{
							IEntryGuideIdentifiable::Execute_GetDesiredNestedWidgets(InEntryWidget, OUT Childs);
						}

						else if (IEntryGuideIdentifiable* Identify = Cast<IEntryGuideIdentifiable>(InEntryWidget))
						{
							Identify->GetDesiredNestedWidgets_Implementation(OUT Childs);
						}

						if (false == Childs.IsValidIndex(ChildIndex))
						{
							ShowGuideWidget(InWorldContextObject, InEntryWidget, InActionParam, InLayerZOrder);
						}

						else
						{
							ShowGuideDynamicWidget(InWorldContextObject, Childs[ChildIndex], NewPath, InActionParam, InLayerZOrder, InAsyncTimeout);
						}					
					});

				AsyncAction->OnFailedNative.AddWeakLambda(WorldContextObject,
					[WorldContextObject, ListView, InActionParam, InLayerZOrder]()
					{
						ShowGuideWidget(WorldContextObject, ListView, InActionParam, InLayerZOrder);
					});

				AsyncAction->Activate();
			}
		}
	}

	else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(InWidget))
	{
		UUserWidget* const* Entry = EntryBox->GetAllEntries().FindByPredicate([Event = CurrentPath.OnGetDynamicEvent](UUserWidget* InEntry)
			{
				return true == Event.IsBound() ? Event.Execute(InEntry) : false;
			});

		UUserWidget* EntryPtr = Entry && *Entry ? *Entry : nullptr;
		
		if (nullptr != EntryPtr)
		{
			TArray<UWidget*> Childs;
			if (true == EntryPtr->GetClass()->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
			{
				IEntryGuideIdentifiable::Execute_GetDesiredNestedWidgets(EntryPtr, OUT Childs);
			}

			else if (IEntryGuideIdentifiable* Identify = Cast<IEntryGuideIdentifiable>(EntryPtr))
			{
				Identify->GetDesiredNestedWidgets_Implementation(OUT Childs);
			}

			if (false == Childs.IsValidIndex(CurrentPath.NextSearchChildIndex))
			{
				ShowGuideWidget(WorldContextObject, EntryPtr, InActionParam, InLayerZOrder);
			}

			else
			{
				ShowGuideDynamicWidget(WorldContextObject, Childs[CurrentPath.NextSearchChildIndex], NewPath, InActionParam, InLayerZOrder, InAsyncTimeout);
			}
		}

		else
		{
			ShowGuideWidget(WorldContextObject, EntryBox, InActionParam, InLayerZOrder);
		}
	}

	else
	{
		ShowGuideWidget(WorldContextObject, InWidget, InActionParam, InLayerZOrder);
	}
}

void UGuideMaskUIFunctionLibrary::GetAllGuideRegisters(const UObject* WorldContextObject, TArray<UGuideMaskRegister*>& FoundWidgets)
{
	FoundWidgets.Empty();

	if (!WorldContextObject)
	{
		return;
	}

	const UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	for (TObjectIterator<UGuideMaskRegister> Itr; Itr; ++Itr)
	{
		UGuideMaskRegister* LiveWidget = *Itr;

		// Skip any widget that's not in the current world context or that is not a child of the class specified.
		if (LiveWidget->GetWorld() != World)
		{
			continue;
		}

		FoundWidgets.Add(LiveWidget);
	}
}

UWidget* UGuideMaskUIFunctionLibrary::GetTagWidget(const UObject* WorldContextObject, const FName& InTag)
{
	if (UGuideMaskRegister* Register = GetRegister(WorldContextObject, InTag))
	{
		return Register->GetTagWidget(InTag);
	}

	return nullptr;
}

UGuideMaskRegister* UGuideMaskUIFunctionLibrary::GetRegister(const UObject* WorldContextObject, const FName& InTag)
{
	TArray<UGuideMaskRegister*> Widgets;
	GetAllGuideRegisters(WorldContextObject, OUT Widgets);

	UGuideMaskRegister** FoundRegister = Widgets.FindByPredicate([InTag](UGuideMaskRegister* InRegister)
		{
			return InRegister && InRegister->IsContains(InTag);
		});

	return FoundRegister && *FoundRegister ? *FoundRegister : nullptr;
}