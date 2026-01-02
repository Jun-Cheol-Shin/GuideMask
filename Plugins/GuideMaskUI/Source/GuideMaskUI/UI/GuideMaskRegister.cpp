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

	if (false == TagWidgetList.Contains(PreviewTag))
	{
		return;
	}

	UWidget* TagWidget = TagWidgetList[PreviewTag];
	if (nullptr == TagWidget)
	{
		return;
	}

	UWidget* Target = nullptr;

	TArray<FGuideHierarchyNode> Tree;
	ConstructWidgetTree(OUT Tree, TagWidget);


	for (int i = 0; i < Tree.Num(); ++i)
	{
		UWidget* ScopeWidget = Tree[i].Container;
		if (nullptr == ScopeWidget)
		{
			continue;
		}

		if (ScopeWidget->GetFName().IsEqual(PreviewWidget))
		{
			Target = ScopeWidget;
			break;
		}

		int Index = Tree[i].Children.IndexOfByPredicate([this](UWidget* InChild)
			{
				return InChild && InChild->GetFName().IsEqual(PreviewWidget);
			});

		if (INDEX_NONE != Index)
		{
			Target = Tree[i].Children[Index];
			break;
		}
	}

	if (nullptr == Target)
	{
		Target = TagWidget;
	}

	if (nullptr != Target)
	{
		SetLayer(Layer);

		UWidget* TargetWidget = nullptr;

		if (UListViewBase* ListView = Cast<UListViewBase>(Target))
		{
			TargetWidget = false == ListView->GetDisplayedEntryWidgets().IsEmpty() ? *ListView->GetDisplayedEntryWidgets().begin() : nullptr;
		}

		else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Target))
		{
			TargetWidget = false == EntryBox->GetAllEntries().IsEmpty() ? *EntryBox->GetAllEntries().begin() : nullptr;
		}

		Layer->SetGuideInternal(InViewportGeometry, nullptr != TargetWidget ? TargetWidget : Target);
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
	TArray<FGuideHierarchyNode> NewTree;
	UWidget* TagWidget = nullptr;

	if (TagWidgetList.Contains(PreviewTag))
	{
		TagWidget = TagWidgetList[PreviewTag];
		ConstructWidgetTree(OUT NewTree, TagWidget);
	}

	for (int i = 0; i < NewTree.Num(); ++i)
	{
		FGuideHierarchyNode Node = NewTree[i];

		if (nullptr == Node.Container)
		{
			continue;
		}

		NameList.AddUnique(Node.Container->GetFName());
		for (int j = 0; j < Node.Children.Num(); ++j)
		{
			UWidget* Child = Node.Children[j];
			if (nullptr == Child)
			{
				continue;
			}

			NameList.AddUnique(Child->GetFName());
		}
	}

	if (true == NameList.IsEmpty() && nullptr != TagWidget)
	{
		NameList.Emplace(TagWidget->GetFName());
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

		if (WidgetBlueprint->WidgetTree)
		{
			TArray<UWidget*> Widgets;
			WidgetBlueprint->WidgetTree->GetAllWidgets(OUT Widgets);

			for (int i = 0; i < Widgets.Num(); ++i)
			{
				if (Widgets[i]->IsA(UGuideMaskRegister::StaticClass()) && i > 0 && this == Widgets[i])
				{
					CompileLog.Error(LOCTEXT("GuideMaskRegister", "Please only one register at the top of the hierarchy!"));
					break;
				}
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

void UGuideMaskRegister::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져온다.
	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGuideMaskRegister, PreviewTag))
	{
		TArray<FName> NestedWidgetList = GetNestedWidgetOptions();

		PreviewWidget = false == NestedWidgetList.IsEmpty() ? *NestedWidgetList.begin() : FName();
	}

	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UGuideMaskRegister, TagWidgetList))
	{
		TArray<FName> TagList = GetTagOptions();

		if (false == TagList.Contains(PreviewTag))
		{
			PreviewTag = false == TagList.IsEmpty() ? *TagList.begin() : FName();

			TArray<FName> WidgetList = GetNestedWidgetOptions();
			PreviewWidget = false == WidgetList.IsEmpty() ? *WidgetList.begin() : FName();
		}
	}
}


void UGuideMaskRegister::ConstructWidgetTree(OUT TArray<FGuideHierarchyNode>& OutNodeTree, UWidget* InWidget) const
{
	if (nullptr == InWidget)
	{
		return;
	}

	FGuideHierarchyNode NewNode;
	TSubclassOf<UUserWidget> EntryClass = nullptr;
	TArray<UUserWidget*> EntryList;

	if (UListView* ListView = Cast<UListView>(InWidget))
	{
		NewNode.Container = InWidget;
		EntryList = ListView->GetDisplayedEntryWidgets();
		EntryClass = ListView->GetEntryWidgetClass();
	}

	else if (UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(InWidget))
	{
		NewNode.Container = InWidget;
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
			NewNode.Children.Emplace(Widget);

			UListView* ListView = Cast<UListView>(Widget);
			UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(Widget);

			if (ListView || EntryBox)
			{
				ContainerWidget.Emplace(Widget);
			}
		}
	}


	if (nullptr != NewNode.Container)
	{
		OutNodeTree.Emplace(NewNode);
	}
	

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

bool UGuideMaskRegister::GetGuideWidgetTree(OUT TArray<FGuideHierarchyNode>& OutWidgetTree, const FName& InGuideTag)
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
		TArray<FGuideHierarchyNode> NewTree;
		ConstructWidgetTree(OUT NewTree, TagWidgetList[InGuideTag]);

		OutWidgetList.Emplace(TagWidgetList[InGuideTag]);
		for (int i = 0; i < NewTree.Num(); ++i)
		{
			FGuideHierarchyNode Node = NewTree[i];

			for (int j = 0; j < Node.Children.Num(); ++j)
			{
				OutWidgetList.Emplace(Node.Children[j]);
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

#if WITH_EDITOR

	if (IsDesignTime())
	{
		bool bRemove = false;

		if (UUserWidget* OuterWidget = GetTypedOuter<UUserWidget>())
		{
			TArray<FName> RemovedTag;

			for (auto& [Tag, Widget] : TagWidgetList)
			{
				if (nullptr == Widget)
				{
					continue;
				}

				if (OuterWidget->WidgetTree)
				{
					if (nullptr == OuterWidget->WidgetTree->FindWidget(Widget->GetFName()))
					{
						RemovedTag.Add(Tag);
					}
				}
			}

			for (int i = 0; i < RemovedTag.Num(); ++i)
			{
				TagWidgetList.Remove(RemovedTag[i]);
				bRemove = true;
			}
		}

		if (true == bRemove)
		{
			TArray<FName> TagList = GetTagOptions();
			PreviewTag = false == TagList.IsEmpty() ? *TagList.begin() : FName();

			TArray<FName> WidgetList = GetNestedWidgetOptions();
			PreviewWidget = false == WidgetList.IsEmpty() ? *WidgetList.begin() : FName();
		} 

		WidgetHierarchy.Reset();
		if (TagWidgetList.Contains(PreviewTag))
		{
			UWidget* Widget = TagWidgetList.FindRef(PreviewTag);
			ConstructWidgetTree(OUT WidgetHierarchy, Widget);
		}
	}

#endif

}


#undef LOCTEXT_NAMESPACE