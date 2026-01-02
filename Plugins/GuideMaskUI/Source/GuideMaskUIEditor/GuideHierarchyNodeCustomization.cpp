// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideHierarchyNodeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "Components/ListView.h"
#include "Components/DynamicEntryBox.h"

#include "GuideMaskUI/UI/GuideMaskRegister.h"

TSharedRef<IPropertyTypeCustomization> FGuideHierarchyNodeCustomization::MakeInstance()
{
    return MakeShareable(new FGuideHierarchyNodeCustomization);
}

void FGuideHierarchyNodeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    ContainerHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGuideHierarchyNode, Container));
    ChildrenHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGuideHierarchyNode, Children));

    const bool bIsArrayElement =
        PropertyHandle->GetParentHandle().IsValid() &&
        PropertyHandle->GetParentHandle()->AsArray().IsValid();

    if (bIsArrayElement)
    {
        const int32 Depth = PropertyHandle->GetIndexInArray();

        HeaderRow
            .WholeRowContent()
            [
                SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [
                        SNew(SSpacer)
                            .Size_Lambda([Depth]() -> FVector2D
                                {
                                    return FVector2D(float(Depth) * 12.f, 0);
                                })
                    ]

                + SHorizontalBox::Slot().AutoWidth()
                    [
                        SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT(" Level %d"), Depth)))
                    ]

                    + SHorizontalBox::Slot().FillWidth(1.f).Padding(12, 0)
                    [
                        SNew(STextBlock)
                            .Text_Lambda([this]()
                                {
                                    FString WidgetName = GetContainerNameText();
                                    FString EntryClassName = GetContainerEntryClassText();

                                    return FText::FromString(FString::Printf(TEXT("%s  (Entry: %s)"),
                                        *WidgetName, *EntryClassName));
                                })
                    ]
            ]
        .ShouldAutoExpand(true);
    }
    else
    {
        HeaderRow
            .NameContent()
            [
                PropertyHandle->CreatePropertyNameWidget()
            ]
            .ValueContent()
            [
                PropertyHandle->CreatePropertyValueWidget()
            ];
    }

}

void FGuideHierarchyNodeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    if (ContainerHandle.IsValid())
    {
        ContainerHandle->MarkHiddenByCustomization();
    }

    if (ensure(ChildrenHandle.IsValid()))
    {
        ChildrenHandle->MarkHiddenByCustomization();


        TSharedRef<FDetailArrayBuilder> ChildArrayBuilder =
            MakeShared<FDetailArrayBuilder>(ChildrenHandle.ToSharedRef(), /*bGenerateHeader*/ false);

        ChildArrayBuilder->OnGenerateArrayElementWidget(
            FOnGenerateArrayElementWidget::CreateLambda(
                [](TSharedRef<IPropertyHandle> ElementHandle, int32 Index, IDetailChildrenBuilder& Children)
                {
                    IDetailPropertyRow& Row = Children.AddProperty(ElementHandle);

                    Row.ShowPropertyButtons(false);
                    Row.IsEnabled(false);
                    Row.ShouldAutoExpand(true);

                    // Row.CustomWidget().WholeRowContent()[  ];
                })
        );

        ChildBuilder.AddCustomBuilder(ChildArrayBuilder);
    }

}

FString FGuideHierarchyNodeCustomization::GetContainerNameText() const
{
    if (!ContainerHandle.IsValid()) return TEXT("None");


    UObject* ContainerWidget = nullptr;
    if (FPropertyAccess::Success != ContainerHandle->GetValue(ContainerWidget) || nullptr == ContainerWidget || nullptr == ContainerWidget->GetClass())
    {
        return TEXT("nullptr!");
    }

    /*if (ContainerWidget->IsA(UListView::StaticClass()) || ContainerWidget->IsA(UListViewBase::StaticClass()))
    {
        return TEXT("ListView");
    }
    else if (ContainerWidget->IsA(UDynamicEntryBox::StaticClass()))
    {
        return TEXT("DynamicEntryBox");
    }*/

    return ContainerWidget->GetFName().ToString();
}

FString FGuideHierarchyNodeCustomization::GetContainerEntryClassText() const
{
    if (!ContainerHandle.IsValid()) return TEXT("None");


    UObject* ContainerWidget = nullptr;
    if (FPropertyAccess::Success != ContainerHandle->GetValue(ContainerWidget) || nullptr == ContainerWidget || nullptr == ContainerWidget->GetClass())
    {
        return TEXT("nullptr!");
    }

    UListView* ListView = Cast<UListView>(ContainerWidget);
    UDynamicEntryBox* EntryBox = Cast<UDynamicEntryBox>(ContainerWidget);

    if (ListView && ListView->GetEntryWidgetClass())
    {
        return ListView->GetEntryWidgetClass()->GetFName().ToString();
    }

    else if (EntryBox && EntryBox->GetEntryWidgetClass())
    {
        return EntryBox->GetEntryWidgetClass()->GetFName().ToString();
    }

    return TEXT("None");
}