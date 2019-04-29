// Fill out your copyright notice in the Description page of Project Settings.

#include "ManipulatorComponent.h"
#include "..\Public\ManipulatorComponent.h"

// Sets default values for this component's properties
UManipulatorComponent::UManipulatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


FTransform UManipulatorComponent::ConstrainTransform(FTransform Transform)
{
	if (Settings.PropertyType == EManipulatorPropertyType::MT_TRANSFORM || Settings.PropertyType == EManipulatorPropertyType::MT_VECTOR)
	{
		if (Settings.ConstraintSettings.UseLocationConstraint)
		{
			// Constrain Location.
			FVector Location = Transform.GetLocation();
			Location.X = FMath::Clamp(Location.X, Settings.ConstraintSettings.XLocationMinMax.X, Settings.ConstraintSettings.XLocationMinMax.Y);
			Location.Y = FMath::Clamp(Location.Y, Settings.ConstraintSettings.YLocationMinMax.X, Settings.ConstraintSettings.YLocationMinMax.Y);
			Location.Z = FMath::Clamp(Location.Z, Settings.ConstraintSettings.ZLocationMinMax.X, Settings.ConstraintSettings.ZLocationMinMax.Y);
			Transform.SetLocation(Location);
		}

		if (Settings.ConstraintSettings.UseScaleConstraint)
		{
			FVector Scale = Transform.GetScale3D();
			Scale.X = FMath::Clamp(Scale.X, Settings.ConstraintSettings.XScaleMinMax.X, Settings.ConstraintSettings.XScaleMinMax.Y);
			Scale.Y = FMath::Clamp(Scale.Y, Settings.ConstraintSettings.YScaleMinMax.X, Settings.ConstraintSettings.YScaleMinMax.Y);
			Scale.Y = FMath::Clamp(Scale.Z, Settings.ConstraintSettings.ZScaleMinMax.X, Settings.ConstraintSettings.ZScaleMinMax.Y);
			Transform.SetScale3D(Scale);
		}
	}
	return Transform;
}

void UManipulatorComponent::SetColors(FLinearColor Color, FLinearColor SelectedColor)
{
	Settings.Color = Color;
	Settings.SelectedColor = SelectedColor;
}

void UManipulatorComponent::SetManipulatorVisualOffset(FTransform ManipulatorVisualOffset)
{
	this->Settings.ManipulatorVisualOffset = ManipulatorVisualOffset;
}

void UManipulatorComponent::TransferOldSettingsToNewFormat()
{
	Settings.Draw.BaseColor = Settings.Color;
}

// Called when the game starts
void UManipulatorComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
bool UManipulatorComponent::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == "EnumSettings")
	{
		return Settings.PropertyType == EManipulatorPropertyType::MT_ENUM;
	}

	if (InProperty->GetFName() == "ConstraintSettings")
	{
		if (Settings.PropertyType == EManipulatorPropertyType::MT_TRANSFORM || Settings.PropertyType == EManipulatorPropertyType::MT_VECTOR)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return ParentVal;
}
#endif
