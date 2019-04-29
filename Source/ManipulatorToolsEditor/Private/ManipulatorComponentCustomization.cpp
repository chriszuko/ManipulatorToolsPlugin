// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorComponentCustomization.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/GameViewportClient.h"
#include "Misc/PackageName.h"
#include "PropertyHandle.h"
#include "Misc/MessageDialog.h"
#include "EditorDirectories.h"
#include "DetailWidgetRow.h"
#include "SButton.h"
#include "SCheckBox.h"
#include "SColorBlock.h"
#include "STextBlock.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "TimerManager.h"
#include "Editor.h"
#include "ManipulatorComponent.h"
#include "IDetailChildrenBuilder.h"
#include "SResetToDefaultMenu.h"
#include "PropertyCustomizationHelpers.h"
//#include "Widgets/Input/SFilePathPicker.h"


#define LOCTEXT_NAMESPACE "ManipulatorComponentCustomization"


/* IDetailCustomization interface
 *****************************************************************************/

void FManipulatorSettingsMainCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	TArray<FString> PropertyNamesToHide;


	TSharedPtr<IPropertyHandle> StructPropertyHandlePointer = StructPropertyHandle;
	TSharedPtr<IPropertyHandle> PropertyTypeHandle = StructPropertyHandle->GetChildHandle("PropertyType");
	TSharedPtr<IPropertyHandle> PropertyDrawTypeHandle = StructPropertyHandle->GetChildHandle("ManipulatorDrawType");
	
	if (!PropertyTypeHandle->IsValidHandle() || !PropertyDrawTypeHandle->IsValidHandle() || !StructPropertyHandlePointer->IsValidHandle())
	{
		return;
	}

	//Make Delegates for when property types change to handle visibility.
	FSimpleDelegate OnPropertyTypeChange = FSimpleDelegate::CreateSP(this, &FManipulatorSettingsMainCustomization::OnPropertyTypeChanged, StructPropertyHandlePointer, PropertyTypeHandle);
	PropertyTypeHandle->SetOnPropertyValueChanged(OnPropertyTypeChange);

	FSimpleDelegate OnPropertyDrawTypeChange = FSimpleDelegate::CreateSP(this, &FManipulatorSettingsMainCustomization::OnPropertyDrawTypeChanged, StructPropertyHandlePointer, PropertyDrawTypeHandle);
	PropertyDrawTypeHandle->SetOnPropertyValueChanged(OnPropertyDrawTypeChange);

	// Build Struct
	UE_LOG(LogTemp, Warning, TEXT("---------------"));
	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		if (ChildHandle->IsValidHandle())
		{
			FString PropertyValue;
			ChildHandle->GetValueAsFormattedString(PropertyValue);
			FString PropertyName = ChildHandle->GetProperty()->GetName();

			UE_LOG(LogTemp, Warning, TEXT("-------%s"), *PropertyName);
			StructBuilder.AddProperty(ChildHandle);
		}
	}

	OnPropertyTypeChanged(StructPropertyHandlePointer, PropertyTypeHandle);
	OnPropertyDrawTypeChanged(StructPropertyHandlePointer, PropertyDrawTypeHandle);
}
/*
LogTemp: Warning: -------------- -
	LogTemp : Warning : Value PropertyNameToEdit Torso
	LogTemp : Warning: Value PropertyIndex 0
	LogTemp : Warning : Value PropertyType MT_TRANSFORM
	LogTemp : Warning: Value ManipulatorDrawType MDT_BOXWIRE
	LogTemp : Warning: Value DepthPriorityGroup SDPG_Foreground
	LogTemp : Warning: Value Color(R = 0.700000, G = 0.530296, B = 0.060574, A = 1.000000)
	LogTemp : Warning : Value SelectedColor(R = 0.700000, G = 0.598401, B = 0.317187, A = 1.000000)
	LogTemp : Warning : Value PropertyEnumSettings(StepSize = 10.000000, Direction = X, EnumSize = 10)
	LogTemp : Warning : Value ManipulatorVisualOffset(Rotation = (X = 0.000000, Y = 0.000000, Z = 0.000000, W = 1.000000), Translation = (X = 0.000000, Y = 0.000000, Z = 0.000000), Scale3D = (X = 1.000000, Y = 1.000000, Z = 1.000000))
	LogTemp : Warning : Value ConstraintSettings(UseLocationConstraint = True, XLocationMinMax = (X = -300.000000, Y = 300.000000), YLocationMinMax = (X = 0.000000, Y = 0.000000), ZLocationMinMax = (X = -500.000000, Y = 500.000000), UseScaleConstraint = False, XScaleMinMax = (X = 0.010000, Y = 2.000000), YScaleMinMax = (X = 0.010000, Y = 2.000000), ZScaleMinMax = (X = 0.010000, Y = 2.000000))
	LogTemp : Warning : Value WireBoxSettings(DrawThickness = 1.500000, SizeMultiplier = 1.000000, BoxSize = (Min = (X = -10.000000, Y = -5.000000, Z = -8.000000), Max = (X = 10.000000, Y = 5.000000, Z = 0.000000)))
	LogTemp : Warning : Value WireDiamondSettings(DrawThickness = 2.000000, Size = 10.000000)
	LogTemp : Warning : Value DrawUsingZoomOffset False
*/
void FManipulatorSettingsMainCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FManipulatorSettingsMainCustomization::OnPropertyTypeChanged(TSharedPtr<IPropertyHandle> StructPropertyHandle, TSharedPtr<IPropertyHandle> PropertyTypeHandle)
{
	//Full Reset of property editor
}

void FManipulatorSettingsMainCustomization::OnPropertyDrawTypeChanged(TSharedPtr<IPropertyHandle> StructPropertyHandle, TSharedPtr<IPropertyHandle> PropertyDrawTypeHandle)
{
}

#undef LOCTEXT_NAMESPACE
