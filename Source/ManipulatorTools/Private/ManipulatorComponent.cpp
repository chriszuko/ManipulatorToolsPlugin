// Fill out your copyright notice in the Description page of Project Settings.

#include "ManipulatorComponent.h"

// Sets default values for this component's properties
UManipulatorComponent::UManipulatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


FTransform UManipulatorComponent::ConstrainTransform(FTransform Transform)
{
	if (Settings.Property.Type == EManipulatorPropertyType::MT_TRANSFORM || Settings.Property.Type == EManipulatorPropertyType::MT_VECTOR)
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

void UManipulatorComponent::SetManipulatorVisualOffset(FTransform ManipulatorVisualOffset, int32 Index)
{
	SetArrayElement(ManipulatorVisualOffset, Settings.Draw.Offsets, Index);
}

FTransform UManipulatorComponent::GetVisualOffset(int32 Index, bool OutputCombinedOffsets)
{
	if (OutputCombinedOffsets == true)
	{
		return CombineOffsetTransforms(Settings.Draw.Offsets);
	}
	else if(Settings.Draw.Offsets.IsValidIndex(Index))
	{
		return Settings.Draw.Offsets[Index];
	}

	return FTransform();
}

void UManipulatorComponent::ClearVisualOffsets()
{
	Settings.Draw.Offsets.Empty();
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

FString UManipulatorComponent::GetManipulatorID()
{
	FString ManipulatorID;
	ManipulatorID = this->GetOwner()->GetName() + "_" + this->GetName() + "_" + Settings.Property.NameToEdit + "_" + FString::FromInt(Settings.Property.Index);
	return ManipulatorID;
}

// ========= WIRE BOX =========

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

FManipulatorSettingsMainDrawWireBox UManipulatorComponent::GetShapeOfTypeWireBox(bool& Success, int32 Index)
{
	if(Settings.Draw.Shapes.WireBoxes.IsValidIndex(Index))
	{
		Success = true;
		return Settings.Draw.Shapes.WireBoxes[Index];
	}
	else
	{
		Success = false;
		return FManipulatorSettingsMainDrawWireBox();
	}
}

void UManipulatorComponent::SetShapeOfTypeWireBox(int32 Index, FManipulatorSettingsMainDrawWireBox WireBox)
{
	SetArrayElement(WireBox, Settings.Draw.Shapes.WireBoxes, Index);
}

// ========= WIRE DIAMOND =========

TArray<FManipulatorSettingsMainDrawWireDiamond> UManipulatorComponent::GetAllShapesOfTypeWireDiamond()
{
	return Settings.Draw.Shapes.WireDiamonds;
}

FManipulatorSettingsMainDrawWireDiamond UManipulatorComponent::GetShapeOfTypeWireDiamond(bool& Success, int32 Index)
{
	if (Settings.Draw.Shapes.WireDiamonds.IsValidIndex(Index))
	{
		Success = true;
		return Settings.Draw.Shapes.WireDiamonds[Index];
	}
	else
	{
		Success = false;
		return FManipulatorSettingsMainDrawWireDiamond();
	}
}

void UManipulatorComponent::SetShapeOfTypeWireDiamond(int32 Index, FManipulatorSettingsMainDrawWireDiamond WireDiamond)
{
	SetArrayElement(WireDiamond, Settings.Draw.Shapes.WireDiamonds, Index);
}

// ========= CIRCLES =========

TArray<FManipulatorSettingsMainDrawCircle> UManipulatorComponent::GetAllShapesOfTypeWireCircle()
{
	return Settings.Draw.Shapes.WireCircles;
}

FManipulatorSettingsMainDrawCircle UManipulatorComponent::GetShapeOfTypeWireCircle(bool& Success, int32 Index)
{
	if (Settings.Draw.Shapes.WireCircles.IsValidIndex(Index))
	{
		Success = true;
		return Settings.Draw.Shapes.WireCircles[Index];
	}
	else
	{
		Success = false;
		return FManipulatorSettingsMainDrawCircle();
	}
}

void UManipulatorComponent::SetShapeOfTypeWireCircle(int32 Index, FManipulatorSettingsMainDrawCircle WireCircle)
{
	SetArrayElement(WireCircle, Settings.Draw.Shapes.WireCircles, Index);
}

// ========= PLANES =========

TArray<FManipulatorSettingsMainDrawPlane> UManipulatorComponent::GetAllShapesOfTypePlane()
{
	return Settings.Draw.Shapes.Planes;
}

FManipulatorSettingsMainDrawPlane UManipulatorComponent::GetShapeOfTypePlane(bool& Success, int32 Index)
{
	if (Settings.Draw.Shapes.Planes.IsValidIndex(Index))
	{
		Success = true;
		return Settings.Draw.Shapes.Planes[Index];
	}
	else
	{
		Success = false;
		return FManipulatorSettingsMainDrawPlane();
	}
}

void UManipulatorComponent::SetShapeOfTypePlane(int32 Index, FManipulatorSettingsMainDrawPlane Plane)
{
	SetArrayElement(Plane, Settings.Draw.Shapes.Planes, Index);
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
		return Settings.Property.Type == EManipulatorPropertyType::MT_ENUM;
	}

	if (InProperty->GetFName() == "Constraints")
	{
		if (Settings.Property.Type == EManipulatorPropertyType::MT_TRANSFORM || Settings.Property.Type == EManipulatorPropertyType::MT_VECTOR)
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

