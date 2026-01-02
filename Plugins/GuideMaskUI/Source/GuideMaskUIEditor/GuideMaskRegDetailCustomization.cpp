// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideMaskRegDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

#include "GuideMaskUI/UI/GuideMaskRegister.h"


const FName PreviewOptionCategoryName = FName(TEXT("Guide Mask Preview Option"));
const FName SettingCategoryName = FName(TEXT("Guide Mask Setting"));
const FName HierarchyCategoryName = FName(TEXT("Virtualized Guide Widget Hierarchy"));



TSharedRef<IDetailCustomization> FGuideMaskRegDetailCustomization::MakeInstance()
{
	return MakeShareable(new FGuideMaskRegDetailCustomization);
}

void FGuideMaskRegDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    IDetailCategoryBuilder& PreviewCategory = DetailBuilder.EditCategory(PreviewOptionCategoryName);
    RegistProperty(DetailBuilder, PreviewCategory, FName("PreviewTag"), UGuideMaskRegister::StaticClass());
    RegisterPropertyChangedFullRefresh(DetailBuilder, FName("PreviewTag"));

    RegistProperty(DetailBuilder, PreviewCategory, FName("PreviewWidget"), UGuideMaskRegister::StaticClass());


    IDetailCategoryBuilder& HierarchyCategory = DetailBuilder.EditCategory(HierarchyCategoryName);
    RegistHierarchyProperty(DetailBuilder, HierarchyCategory, FName("WidgetHierarchy"), UGuideMaskRegister::StaticClass());


    IDetailCategoryBuilder& SettingCategory = DetailBuilder.EditCategory(SettingCategoryName);
    RegistProperty(DetailBuilder, SettingCategory, FName("TagWidgetList"), UGuideMaskRegister::StaticClass(), true);


    DetailBuilder.SortCategories(&SortCustomDetailsCategories);
}

void FGuideMaskRegDetailCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
    CachedDetailBuilder = DetailBuilder;
    CustomizeDetails(*DetailBuilder);
}

void FGuideMaskRegDetailCustomization::RegistProperty(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& InCategoryBuilder, const FName& InProperty, const UStruct* ClassOutermost, bool bShouldAutoExpand)
{
    TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(InProperty, ClassOutermost);
    auto& Row = InCategoryBuilder.AddProperty(PropertyHandle);
    Row.ShouldAutoExpand(bShouldAutoExpand);
}

void FGuideMaskRegDetailCustomization::RegistHierarchyProperty(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& InCategoryBuilder, const FName& InProperty, const UStruct* ClassOutermost)
{
    TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(InProperty, ClassOutermost);
    DetailBuilder.HideProperty(PropertyHandle);

    uint32 Num = 0;
    if (auto Array = PropertyHandle->AsArray())
    {
        Array->GetNumElements(Num);
    }


    if (Num == 0)
    {
        InCategoryBuilder.AddCustomRow(FText::FromString("HierarchyEmpty"))
            .WholeRowContent()
            [
                SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Hierarchy is Empty.")))
                    .AutoWrapText(true)
            ];
    }

    else
    {

        TSharedRef<FDetailArrayBuilder> ArrayBuilder =
            MakeShared<FDetailArrayBuilder>(PropertyHandle, /*bGenerateHeader*/ false);

        ArrayBuilder->OnGenerateArrayElementWidget(
            FOnGenerateArrayElementWidget::CreateLambda(
                [](TSharedRef<IPropertyHandle> ElementHandle, int32 Index, IDetailChildrenBuilder& Children)
                {
                    IDetailPropertyRow& Row = Children.AddProperty(ElementHandle);
                    Row.ShowPropertyButtons(false);
                    Row.IsEnabled(false);
                })
        );

        InCategoryBuilder.AddCustomBuilder(ArrayBuilder);
    }
}

void FGuideMaskRegDetailCustomization::RegisterPropertyChangedFullRefresh(IDetailLayoutBuilder& DetailBuilder, const FName Property)
{
    const TSharedPtr<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(Property);
    PropertyHandle.Get()->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FGuideMaskRegDetailCustomization::ForceRefreshDetailPanel));
    PropertyHandle.Get()->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FGuideMaskRegDetailCustomization::ForceRefreshDetailPanel));
}

void FGuideMaskRegDetailCustomization::ForceRefreshDetailPanel()
{
    if (IDetailLayoutBuilder* DetailBuilder = CachedDetailBuilder.Pin().Get())

    {
        DetailBuilder->ForceRefreshDetails();
    }
}

void FGuideMaskRegDetailCustomization::SortCustomDetailsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
{
    (*AllCategoryMap.Find(PreviewOptionCategoryName))->SetSortOrder(0);
    (*AllCategoryMap.Find(HierarchyCategoryName))->SetSortOrder(1);
    (*AllCategoryMap.Find(SettingCategoryName))->SetSortOrder(2);
}
