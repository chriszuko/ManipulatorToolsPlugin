// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "SButton.h"

class IPropertyHandle;

/**
 * Implements a details view customization for the FFilePath structure.
 */
class FManipulatorSettingsMainCustomization : public IPropertyTypeCustomization
{
public:

	/**
	 * Creates an instance of this class.
	 *
	 * @return The new instance.
	 */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance( )
	{
		return MakeShareable(new FManipulatorSettingsMainCustomization());
	}

	// IPropertyTypeCustomization interface

	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeHeader( TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

private:
	void OnPropertyTypeChanged(TSharedPtr<IPropertyHandle> StructPropertyHandle, TSharedPtr<IPropertyHandle> PropertyTypeHandle);
	void OnPropertyDrawTypeChanged(TSharedPtr<IPropertyHandle> StructPropertyHandle, TSharedPtr<IPropertyHandle> PropertyDrawTypeHandle);

protected:


};
