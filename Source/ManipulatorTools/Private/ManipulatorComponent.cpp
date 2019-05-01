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
		if (Settings.Constraints.UseLocationConstraint)
		{
			// Constrain Location.
			FVector Location = Transform.GetLocation();
			Location.X = FMath::Clamp(Location.X, Settings.Constraints.XLocationMinMax.X, Settings.Constraints.XLocationMinMax.Y);
			Location.Y = FMath::Clamp(Location.Y, Settings.Constraints.YLocationMinMax.X, Settings.Constraints.YLocationMinMax.Y);
			Location.Z = FMath::Clamp(Location.Z, Settings.Constraints.ZLocationMinMax.X, Settings.Constraints.ZLocationMinMax.Y);
			Transform.SetLocation(Location);
		}

		if (Settings.Constraints.UseScaleConstraint)
		{
			FVector Scale = Transform.GetScale3D();
			Scale.X = FMath::Clamp(Scale.X, Settings.Constraints.XScaleMinMax.X, Settings.Constraints.XScaleMinMax.Y);
			Scale.Y = FMath::Clamp(Scale.Y, Settings.Constraints.YScaleMinMax.X, Settings.Constraints.YScaleMinMax.Y);
			Scale.Y = FMath::Clamp(Scale.Z, Settings.Constraints.ZScaleMinMax.X, Settings.Constraints.ZScaleMinMax.Y);
			Transform.SetScale3D(Scale);
		}
	}
	return Transform;
}

void UManipulatorComponent::SetColors(FLinearColor Color, FLinearColor SelectedColor)
{
	Settings.Draw.BaseColor = Color;
	Settings.Draw.SelectedColor = SelectedColor;
}

void UManipulatorComponent::SetManipulatorVisualOffset(FTransform ManipulatorVisualOffset)
{
	Settings.ManipulatorVisualOffset = ManipulatorVisualOffset;
}

void UManipulatorComponent::TransferOldSettingsToNewFormat()
{
	// Colors
	Settings.Draw.BaseColor = Settings.Color;
	Settings.Draw.SelectedColor = Settings.SelectedColor;

	// Property Settings
	Settings.Property.NameToEdit = Settings.PropertyNameToEdit;
	Settings.Property.Index = Settings.PropertyIndex;
	Settings.Property.Type = Settings.PropertyType;
	Settings.Property.EnumSettings = Settings.PropertyEnumSettings;

	// Populate Extras
	Settings.Draw.Extras.DepthPriorityGroup = Settings.DepthPriorityGroup;
	Settings.Draw.Extras.FlipVisualXLocation = Settings.AdvancedSettings.FlipVisualXLocation;
	Settings.Draw.Extras.FlipVisualXScale = Settings.AdvancedSettings.FlipVisualXScale;
	Settings.Draw.Extras.FlipVisualYRotation = Settings.AdvancedSettings.FlipVisualYRotation;
	Settings.Draw.Extras.UseZoomOffset = Settings.DrawUsingZoomOffset;
	Settings.Draw.Extras.UsePropertyValueAsInitialOffset = !Settings.IgnorePropertyValueForVisualOffset;

	// Visual Offsets
	Settings.Draw.Offsets.Empty();
	Settings.Draw.Offsets.Add(Settings.ManipulatorVisualOffset);

	// Empty Shapes to add new one.
	Settings.Draw.Shapes.Planes.Empty();
	Settings.Draw.Shapes.WireBoxes.Empty();
	Settings.Draw.Shapes.WireCircles.Empty();
	Settings.Draw.Shapes.WireDiamonds.Empty();

	// Add Specific Shape to the shapes
	switch (Settings.ManipulatorDrawType)
	{
	case EManipulatorPropertyDrawType::MDT_BOXWIRE:
	{
		Settings.Draw.Shapes.WireBoxes.Add(Settings.WireBoxSettings);
		break;
	}
	case EManipulatorPropertyDrawType::MDT_DIAMONDWIRE:
	{
		Settings.Draw.Shapes.WireDiamonds.Add(Settings.WireDiamondSettings);
		break;
	}
	case EManipulatorPropertyDrawType::MDT_PLANE:
	{
		Settings.Draw.Shapes.Planes.Add(Settings.PlaneSettings);
		break;
	}
	case EManipulatorPropertyDrawType::MDT_CIRCLE:
	{
		Settings.Draw.Shapes.WireCircles.Add(Settings.CircleSettings);
		break;
	}
	}

	// Constraints
	Settings.Constraints = Settings.ConstraintSettings;
}

FTransform UManipulatorComponent::CombineOffsetTransforms(TArray<FTransform> Offsets)
{
	// Combines all transforms of the input transforms.
	FTransform FinalOffset = FTransform();
	for (FTransform Offset : Offsets)
	{
		FinalOffset = FinalOffset * Offset;
	}
	return FinalOffset;
}

TArray<FManipulatorSettingsMainDrawWireBox> UManipulatorComponent::GetAllShapesOfTypeWireBox()
{
	TArray<FManipulatorSettingsMainDrawWireBox> WireBoxes = Settings.Draw.Shapes.WireBoxes;
	// If all the shapes are empty, then we are going to draw a wire box.
	if (Settings.Draw.Shapes.WireBoxes.Num() == 0 
		&& Settings.Draw.Shapes.Planes.Num() == 0 
		&& Settings.Draw.Shapes.WireCircles.Num() == 0 
		&& Settings.Draw.Shapes.WireDiamonds.Num() == 0)
	{
		FManipulatorSettingsMainDrawWireBox NewWireBox = FManipulatorSettingsMainDrawWireBox();
		NewWireBox.Color = FLinearColor(1, 1, 1, 1);
		WireBoxes.Add(NewWireBox);
		return WireBoxes;
	}
	return WireBoxes;
}

TArray<FManipulatorSettingsMainDrawWireDiamond> UManipulatorComponent::GetAllShapesOfTypeWireDiamond()
{
	return Settings.Draw.Shapes.WireDiamonds;
}

TArray<FManipulatorSettingsMainDrawCircle> UManipulatorComponent::GetAllShapesOfTypeCircle()
{
	return Settings.Draw.Shapes.WireCircles;
}

TArray<FManipulatorSettingsMainDrawPlane> UManipulatorComponent::GetAllShapesOfTypePlane()
{
	return Settings.Draw.Shapes.Planes;
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

	if (InProperty->GetFName() == "Constraints")
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
