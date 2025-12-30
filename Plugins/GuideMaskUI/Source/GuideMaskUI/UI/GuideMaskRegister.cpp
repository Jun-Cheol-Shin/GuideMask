// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideMaskRegister.h"
#include "Components/PanelSlot.h"
#include "Components/OverlaySlot.h"

#include "Components/TreeView.h"
#include "Components/ListView.h"
#include "Components/DynamicEntryBox.h"
#include "Components/WrapBox.h"

#include "../EntryGuideIdentifiable.h"
#include "../GuideMaskUIFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "GuideMaskRegister"

#if WITH_EDITOR
#include "Editor/UMGEditor/Public/WidgetBlueprint.h"
#include "Editor/WidgetCompilerLog.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

#include "../GuideMaskSettings.h"

void UGuideMaskRegister::HidePreviewDebug()
{
	if (nullptr != LayerContent)
	{
		if (Overlay)
		{
			Overlay->RemoveSlot(LayerContent->TakeWidget());
		}

		LayerContent->RemoveFromParent();
		LayerContent = nullptr;
	}
}

void UGuideMaskRegister::ShowPreviewDebug()
{
	HidePreviewDebug();

	ForceLayoutPrepass();

	const UGuideMaskSettings* Settings = GetDefault<UGuideMaskSettings>();
	if (ensureAlways(Settings))
	{
		if (!ensureAlwaysMsgf(Settings->DefaultLayer.ToSoftObjectPath().IsValid(),
			TEXT("Invalid Layer base class in the project settings.")))
		{
			return;
		}

		UWidget* ContentWidget = GetContent();
		if (ContentWidget && ContentWidget->GetCachedWidget())
		{
			FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
			TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(Settings->DefaultLayer.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
				[this, Geometry = ContentWidget->GetCachedWidget()->GetTickSpaceGeometry()]()
				{
					CreatePreviewLayer(Geometry);
				}));
		}
	}
}

void UGuideMaskRegister::CreatePreviewLayer(const FGeometry& InViewportGeometry)
{
	const UGuideMaskSettings* Settings = GetDefault<UGuideMaskSettings>();
	if (!ensureAlways(Settings))
	{
		return;
	}

	UGuideLayerBase* Layer = CreateWidget<UGuideLayerBase>(GetWorld(), Settings->DefaultLayer.Get());
	if (nullptr == Layer)
	{
		return;
	}

	if (false == TagWidgetList.Contains(PreviewWidgetTag))
	{
		return;
	}

	UWidget* TagWidget = TagWidgetList[PreviewWidgetTag];
	if (nullptr == TagWidget)
	{
		return;
	}

	UWidget* Target = nullptr;

	TArray<FGuideTreeNode> Tree;
	ConstructWidgetTree(OUT Tree, TagWidget);


	for (int i = 0; i < Tree.Num(); ++i)
	{
		UWidget* ScopeWidget = Tree[i].Scope;
		if (nullptr == ScopeWidget)
		{
			continue;
		}

		if (ScopeWidget->GetFName().IsEqual(PreviewNestedWidget))
		{
			Target = ScopeWidget;
			break;
		}

		int Index = Tree[i].NestedWidgets.IndexOfByPredicate([this](UWidget* InChild)
			{
				return InChild && InChild->GetFName().IsEqual(PreviewNestedWidget);
			});

		if (INDEX_NONE != Index)
		{
			Target = Tree[i].NestedWidgets[Index];
			break;
		}
	}

	if (nullptr != Target)
	{
		SetLayer(Layer);

		UWidget* PreviewWidget = nullptr;

		if (UListViewBase* ListView = Cast<UListViewBase>(Target))
		{
			PreviewWidget = false == ListView->GetDisplayedEntryWidgets().IsEmpty() ? *ListView->GetDisplayedEntryWidgets().begin() : nullptr;
		}

		else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Target))
		{
			PreviewWidget = false == EntryBox->GetAllEntries().IsEmpty() ? *EntryBox->GetAllEntries().begin() : nullptr;
		}

		Layer->SetGuide(InViewportGeometry, nullptr != PreviewWidget ? PreviewWidget : Target);
	}
}


TArray<FName> UGuideMaskRegister::GetTagOptions() const
{
	TArray<FName> TagList;

	Algo::Transform(TagWidgetList, TagList, [](const TPair<FName, UWidget*>& InEntry) -> FName
		{
			return InEntry.Key;
		});

	return TagList;
}

TArray<FName> UGuideMaskRegister::GetNestedWidgetOptions() const
{
	TArray<FName> NameList;
	TArray<FGuideTreeNode> NewTree;

	if (TagWidgetList.Contains(PreviewWidgetTag))
	{
		UWidget* TagWidget = TagWidgetList[PreviewWidgetTag];
		ConstructWidgetTree(OUT NewTree, TagWidget);
	}

	for (int i = 0; i < NewTree.Num(); ++i)
	{
		FGuideTreeNode Node = NewTree[i];

		if (nullptr == Node.Scope)
		{
			continue;
		}

		NameList.AddUnique(Node.Scope->GetFName());
		for (int j = 0; j < Node.NestedWidgets.Num(); ++j)
		{
			UWidget* Child = Node.NestedWidgets[j];
			if (nullptr == Child)
			{
				continue;
			}

			NameList.AddUnique(Child->GetFName());
		}
	}

	return NameList;
}

const FText UGuideMaskRegister::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "Guide Mask Plugin", "Guide Mask Plugin");
}


void UGuideMaskRegister::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	if (UWidgetBlueprint* WidgetBlueprint = GetTypedOuter<UWidgetBlueprint>())
	{
		if (WidgetBlueprint->GeneratedClass)
		{
			if (WidgetBlueprint->GeneratedClass->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
			{
				CompileLog.Error(LOCTEXT("GuideMaskRegister", "Do not Inherited EntryGuideIdentifiable Interface!"));
			}

			if (WidgetBlueprint->GeneratedClass->ImplementsInterface(UUserObjectListEntry::StaticClass()))
			{
				CompileLog.Error(LOCTEXT("GuideMaskRegister", "Do not Inherited UserObjectListEntry Interface!"));
			}
		}
	}


	for (auto& [Tag, Widget] : TagWidgetList)
	{
		if (UListViewBase* ListView = Cast<UListViewBase>(Widget))
		{
			UClass* WidgetClass = ListView->GetEntryWidgetClass();
			if (WidgetClass && false == WidgetClass->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
			{
				CompileLog.Error(FText::Format(LOCTEXT("GuideMaskRegister", 
					"{0} Class doesn't implement EntryGuideIdentifiable Interface!"), 
					FText::FromString(WidgetClass->GetName())));
			}

		}

		else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Widget))
		{
			UClass* WidgetClass = EntryBox->GetEntryWidgetClass();
			if (WidgetClass && false == WidgetClass->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
			{
				CompileLog.Error(FText::Format(LOCTEXT("GuideMaskRegister",
					"{0} Class doesn't implement EntryGuideIdentifiable Interface!"),
					FText::FromString(WidgetClass->GetName())));
			}
		}

		else if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (nullptr == UserWidget->GetClass())
			{
				return;
			}

			else if (UWidgetBlueprint* GeneratedWidget = Cast<UWidgetBlueprint>(UserWidget->GetClass()->ClassGeneratedBy))
			{
				if (GeneratedWidget->WidgetTree && GeneratedWidget->WidgetTree->RootWidget)
				{
					if (UGuideMaskRegister* Register = Cast<UGuideMaskRegister>(GeneratedWidget->WidgetTree->RootWidget))
					{
						CompileLog.Error(FText::Format(LOCTEXT("GuideMaskRegister",
							"Do not containing GuideRegister in TagWidgetList. Widget Name : {0}"),
							FText::FromString(UserWidget->GetName())));
					}
				}
			}
		}
	}


}


void UGuideMaskRegister::ConstructWidgetTree(OUT TArray<FGuideTreeNode>& OutNodeTree, UWidget* InWidget) const
{
	if (nullptr == InWidget)
	{
		return;
	}

	FGuideTreeNode NewNode;
	NewNode.Scope = InWidget;

	TSubclassOf<UUserWidget> EntryClass = nullptr;
	TArray<UUserWidget*> EntryList;

	if (UListView* ListView = Cast<UListView>(InWidget))
	{
		EntryList = ListView->GetDisplayedEntryWidgets();
		EntryClass = ListView->GetEntryWidgetClass();
	}

	else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(InWidget))
	{
		EntryList = EntryBox->GetAllEntries();
		EntryClass = EntryBox->GetEntryWidgetClass();
	}


	UUserWidget* Entry = 
		true == EntryList.IsEmpty() ?
		nullptr != EntryClass ? 
		CreateWidget<UUserWidget>(GetWorld(), EntryClass) : nullptr : *EntryList.begin();

	TArray<UWidget*> ContainerWidget {};

	if (nullptr != Entry)
	{
		TArray<UWidget*> Childs;

		if (true == Entry->GetClass()->ImplementsInterface(UEntryGuideIdentifiable::StaticClass()))
		{
			IEntryGuideIdentifiable::Execute_GetDesiredNestedWidgets(Entry, OUT Childs);
		}

		else if (IEntryGuideIdentifiable* Identify = Cast<IEntryGuideIdentifiable>(Entry))
		{
			Identify->GetDesiredNestedWidgets_Implementation(OUT Childs);
		}

		for (auto& Widget : Childs)
		{
			NewNode.NestedWidgets.Emplace(Widget);

			UListView* ListView = Cast<UListView>(Widget);
			UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Widget);

			if (ListView || EntryBox)
			{
				ContainerWidget.Emplace(Widget);
			}
		}
	}

	OutNodeTree.Emplace(NewNode);

	for (int i = 0; i < ContainerWidget.Num(); ++i)
	{
		ConstructWidgetTree(OutNodeTree, ContainerWidget[i]);
	}

}

#endif


bool UGuideMaskRegister::IsContains(const FName& InTag) const
{
	return TagWidgetList.Contains(InTag);
}

TArray<FName> UGuideMaskRegister::GetTagList() const
{
	TArray<FName> Retval;
	TagWidgetList.GenerateKeyArray(OUT Retval);

	return Retval;
}

UWidget* UGuideMaskRegister::GetTagWidget(const FName& InGuideTag)
{
	if (TagWidgetList.Contains(InGuideTag))
	{
		return TagWidgetList[InGuideTag];
	}

	return nullptr;
}

bool UGuideMaskRegister::GetGuideWidgetTree(OUT TArray<FGuideTreeNode>& OutWidgetTree, const FName& InGuideTag)
{
	if (TagWidgetList.Contains(InGuideTag))
	{
		ConstructWidgetTree(OUT OutWidgetTree, TagWidgetList[InGuideTag]);
		return true;
	}

	return false;
}

bool UGuideMaskRegister::GetGuideWidgetList(OUT TArray<UWidget*>& OutWidgetList, const FName& InGuideTag)
{
	OutWidgetList.Reset();

	if (TagWidgetList.Contains(InGuideTag))
	{
		TArray<FGuideTreeNode> NewTree;
		ConstructWidgetTree(OUT NewTree, TagWidgetList[InGuideTag]);

		OutWidgetList.Emplace(TagWidgetList[InGuideTag]);
		for (int i = 0; i < NewTree.Num(); ++i)
		{
			FGuideTreeNode Node = NewTree[i];

			for (int j = 0; j < Node.NestedWidgets.Num(); ++j)
			{
				OutWidgetList.Emplace(Node.NestedWidgets[j]);
			}
		}

		return true;
	}

	return false;
}

void UGuideMaskRegister::SetLayer(UWidget* InLayer)
{
	if (!ensureAlways(InLayer && Overlay))
	{
		return;
	}

	LayerContent = InLayer;
	
	if (Overlay)
	{
		Overlay->
			AddSlot()
			[
				LayerContent->TakeWidget()
			];
	}
}

TSharedRef<SWidget> UGuideMaskRegister::RebuildWidget()
{
	Overlay = SNew(SOverlay);

	if (UWidget* WidgetInDesigner = GetContent())
	{
		Overlay->
			AddSlot()
			[
				WidgetInDesigner->TakeWidget()
			];
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	return Overlay.ToSharedRef();
}

void UGuideMaskRegister::ReleaseSlateResources(bool bReleaseChildren)
{
	if (Overlay)
	{
		if (LayerContent)
		{
			Overlay->RemoveSlot(LayerContent->TakeWidget());
		}

		if (UWidget* ContentWidget = GetContent())
		{
			Overlay->RemoveSlot(ContentWidget->TakeWidget());
		}

		LayerContent = nullptr;
		Overlay = nullptr;
	}

	Super::ReleaseSlateResources(bReleaseChildren);
}

void UGuideMaskRegister::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	TArray<FName> RemovedTag;

	for (auto& [Tag, Widget] : TagWidgetList)
	{
		if (nullptr == Widget)
		{
			RemovedTag.Add(Tag);
		}
	}

	for (const FName& Tag : RemovedTag)
	{
		TagWidgetList.Remove(Tag);
	}

#if WITH_EDITOR

	GuideWidgetTree.Reset();

	if (TagWidgetList.Contains(PreviewWidgetTag))
	{
		UWidget* Widget = TagWidgetList.FindRef(PreviewWidgetTag);
		ConstructWidgetTree(OUT GuideWidgetTree, Widget);
	}

#endif 
}


#undef LOCTEXT_NAMESPACE