// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class GUIDEMASKUIEDITOR_API FGuideHierarchyNodeCustomization : public IPropertyTypeCustomization
{

public:
    //프로퍼티 커스터마이제이션의 인스턴스를 생성하기 위한 유틸리티 함수입니다.
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    // IPropertyTypeCustomization에서 상속됨
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
    // ---- Header 요약용 ----
    FString GetContainerNameText() const;
    FString GetContainerEntryClassText() const;
    UClass* GetEntryClass() const;

private:
    TSharedPtr<IPropertyHandle> ContainerHandle;
    TSharedPtr<IPropertyHandle> ChildrenHandle;
};
