// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "ManipulatorComponent.generated.h"

UENUM(BlueprintType)
enum class EManipulatorPropertyType : uint8
{
	MT_TRANSFORM 	UMETA(DisplayName = "Transform"),
	MT_VECTOR 		UMETA(DisplayName = "Vector"),
	MT_ENUM         UMETA(DisplayName = "Enum"),
	MT_BOOL			UMETA(DisplayName = "Bool")
};

UENUM(BlueprintType)
enum class EManipulatorPropertyEnumDirection : uint8
{
	MT_X 	UMETA(DisplayName = "X"),
	MT_Y    UMETA(DisplayName = "Y"),
	MT_Z    UMETA(DisplayName = "Z")
};

UENUM(BlueprintType)
enum class EManipulatorPropertyDrawType : uint8
{
	MDT_BOXWIRE 	 UMETA(DisplayName = "Wire Box"),
	MDT_DIAMONDWIRE  UMETA(DisplayName = "Wire Diamond"),
	MDT_PLANE        UMETA(DisplayName = "Plane"),
	MDT_CIRCLE		 UMETA(DisplayName = "Circle")
};

// Specific Settings for Wire Boxes
USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawWireBox
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DrawThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SizeMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBox BoxSize = FBox(FVector(-5, -5, -5), FVector(5, 5, 5));

	/** Offset Visually*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> Offsets;
};

// Specific Settings for Wire Diamonds
USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawWireDiamond
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DrawThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 10.0f;

	/** Offset Visually*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> Offsets;
};

// Specific Settings for Planes
USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawPlane
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UVMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UVMax = 1.0f;

	/** Offset Visually*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> Offsets;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawCircle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector XVector = FVector(1, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector YVector = FVector(0, 1, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DrawThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NumSides = 24.0f;

	/** Offset Visually*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	TArray<FTransform> Offsets;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainConstraints
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseLocationConstraint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D XLocationMinMax = FVector2D(-5, 5);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D YLocationMinMax = FVector2D(-5, 5);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ZLocationMinMax = FVector2D(-5, 5);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseScaleConstraint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D XScaleMinMax = FVector2D(0.01, 2);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D YScaleMinMax = FVector2D(0.01, 2);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D ZScaleMinMax = FVector2D(0.01, 2);
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainPropertyTypeEnum
{
	GENERATED_USTRUCT_BODY()

	/** They will always manipulate along X Axis*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StepSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EManipulatorPropertyEnumDirection Direction = EManipulatorPropertyEnumDirection::MT_X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 EnumSize = 10;
};

// Being Moved
USTRUCT(BlueprintType)
struct FManipulatorSettingsMainAdvanced
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualXLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualYRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualXScale = false;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawShapes
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FManipulatorSettingsMainDrawWireBox> WireBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FManipulatorSettingsMainDrawWireDiamond> WireDiamonds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FManipulatorSettingsMainDrawCircle> WireCircles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FManipulatorSettingsMainDrawPlane> Planes;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDrawExtras
{
	GENERATED_USTRUCT_BODY()

	/** Sets whether a manipulator will draw like any other object or through every object*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESceneDepthPriorityGroup> DepthPriorityGroup = ESceneDepthPriorityGroup::SDPG_Foreground;

	/** By Default, Vectors and Transforms use their data as the first visual offset for the manipulator. Turning it off means you have to handle it manually for specific cases. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UsePropertyValueAsInitialOffset = true;

	/** Zoom Offset allows the manipulator to keep its size relative to camera distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseZoomOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualXLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualYRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlipVisualXScale = false;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainDraw
{
	GENERATED_USTRUCT_BODY()

	/** Default Color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BaseColor = FLinearColor(0.15f, 0.5f, 0.7f, 1);

	/** Color When Selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SelectedColor = FLinearColor(0.5f, 0.8f, 1, 1);

	/** In order from top to bottom if you want to have easy control of your offsets*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> Offsets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainDrawShapes Shapes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainDrawExtras Extras;
};

USTRUCT(BlueprintType)
struct FManipulatorSettingsMainProperty
{
	GENERATED_USTRUCT_BODY()

	/** Property name that you created in your blueprint to link to and edit automatically If the name matches and is the correct type then it will automatically connect to it without any additional support needed from your blueprint.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NameToEdit = "None";

	/** Make sure the property type matches the property name you are trying to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EManipulatorPropertyType Type = EManipulatorPropertyType::MT_TRANSFORM;

	/** For Arrays you can specifically tell which index to point to, this is ignored if your property is not an array. This would typically be used only if you were auto creating manipulators and attaching them to an array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index = 0;

	/** Settings specifically for Enum Properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainPropertyTypeEnum EnumSettings;
};

// Overall Settings ( A struct format allows for an easy way to copy data from one to another )
USTRUCT(BlueprintType)
struct FManipulatorSettingsMain
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainProperty Property;

	/** This is the main struct for all display settings. You can add as many types of display to the same manipulator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainDraw Draw;

	/** Constrains Manipulator Vectors and Transforms based off of a min max value on location and scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainConstraints Constraints;

	/** Property name that you created in your blueprint to link to and edit automatically If the name matches and is the correct type then it will automatically connect to it without any additional support needed from your blueprint.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PropertyNameToEdit = "None";

	/** Make sure the property type matches the property name you are trying to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EManipulatorPropertyType PropertyType = EManipulatorPropertyType::MT_TRANSFORM;

	/** For Arrays you can specifically tell which index to point to, this is ignored if your property is not an array. This would typically be used only if you were auto creating manipulators and attaching them to an array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PropertyIndex = 0;

	/** These are the different allowed draw types for manipulators, Each one has their own individual types of settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "PropertyDrawType")
	EManipulatorPropertyDrawType ManipulatorDrawType = EManipulatorPropertyDrawType::MDT_BOXWIRE;

	/** Sets whether a manipulator will draw like any other object or through every object*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ESceneDepthPriorityGroup> DepthPriorityGroup = ESceneDepthPriorityGroup::SDPG_Foreground;

	/** Default Color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.15f, 0.5f, 0.7f, 1);

	/** Color When Selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SelectedColor = FLinearColor(0.5f, 0.8f, 1, 1);

	/** Zoom Offset allows the manipulator to keep its size relative to camera distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DrawUsingZoomOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IgnorePropertyValueForVisualOffset = false;

	/** Settings specifically for Enum Properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainPropertyTypeEnum PropertyEnumSettings;

	/** Allows you to offset the visual information of the manipulator, very useful for things where you want to keep the data relative, but then move the manipulator away from a visual element it may be blocking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "VisualOffset")
	FTransform ManipulatorVisualOffset;

	/** Specific settings for Wire Box Draw Type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "SettingsDrawWireBox")
	FManipulatorSettingsMainDrawWireBox WireBoxSettings;

	/** Specific Settings for Wire Diamond Draw Type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "SettingsDrawDiamond")
	FManipulatorSettingsMainDrawWireDiamond WireDiamondSettings;

	/** Specific Settings for Draw Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "SettingsDrawPlane")
	FManipulatorSettingsMainDrawPlane PlaneSettings;

	/** Specific Settings for Draw Circle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "SettingsDrawCircle")
	FManipulatorSettingsMainDrawCircle CircleSettings;

	/** Specific Settings for Draw Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainAdvanced AdvancedSettings;

	/** Constrains Manipulator Vectors and Transforms based off of a min max value on location and scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FManipulatorSettingsMainConstraints ConstraintSettings;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), hidecategories = ("Rendering" , "Physics" , "ComponentReplication" , "LOD", "AssetUserData", "Collision", "Cooking", "Activation"))
class MANIPULATORTOOLS_API UManipulatorComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UManipulatorComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FManipulatorSettingsMain Settings;

	// This is handled automatically by the editor mode, however you can use this to modify other transforms easily.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FTransform ConstrainTransform(FTransform Transform);

	// Easy Way to set the colors without navigating through the struct
	UFUNCTION(BlueprintCallable)
	void SetColors(FLinearColor Color, FLinearColor SelectedColor);

	// Update the visual offset without navigating through the struct
	UFUNCTION(BlueprintCallable, DisplayName = "SetVisualOffset")
	void SetManipulatorVisualOffset(FTransform ManipulatorVisualOffset);

	// Easy Way to set the colors without navigating through the struct
	UFUNCTION(BlueprintCallable)
	void TransferOldSettingsToNewFormat();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

};
