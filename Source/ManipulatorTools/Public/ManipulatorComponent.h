// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "ManipulatorComponent.generated.h"


template <typename T>
static void SetArrayElement(T Item, TArray<T>& ItemArray, int32 Index)
{
	if (!ItemArray.IsValidIndex(Index))
	{
		ItemArray.SetNum(Index+1);
	}
	ItemArray[Index] = Item;
}

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
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 1);

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
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 1);

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
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 1);

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
	FLinearColor Color = FLinearColor(1.0f, 1.0f, 1.0f, 1);

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

	/** When on, it overrides Use PropertyValueAsInitialOffset and then finds the parent socket its attached too's position to use as the manipualtor's Initial location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseAttachedSocketAsInitialOffset = false;

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

	/** Color When Selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallSize = 1.0f;

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
	UFUNCTION(BlueprintCallable, DisplayName = "Set Offset Transform")
	void SetManipulatorVisualOffset(FTransform ManipulatorVisualOffset, int32 Index = 0);

	// Update the visual offset without navigating through the struct
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "Get Offset Transform")
	FTransform GetVisualOffset(int32 Index = 0, bool OutputCombinedOffsets = false);

	UFUNCTION(BlueprintCallable, DisplayName = "Clear Offset Transforms")
	void ClearVisualOffsets();

	UFUNCTION(BlueprintCallable)
	FTransform CombineOffsetTransforms(TArray<FTransform> Offsets);

	UFUNCTION(BlueprintCallable)
	FString GetManipulatorID();


	// ========= WIRE BOX =========

	// Gets all shapes of wire box.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	TArray<FManipulatorSettingsMainDrawWireBox> GetAllShapesOfTypeWireBox();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	FManipulatorSettingsMainDrawWireBox GetShapeOfTypeWireBox(bool& Success, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "ManipulatorTools|Shapes")
	void SetShapeOfTypeWireBox(int32 Index, FManipulatorSettingsMainDrawWireBox WireBox);


	// ========= WIRE DIAMOND =========

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	TArray<FManipulatorSettingsMainDrawWireDiamond> GetAllShapesOfTypeWireDiamond();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	FManipulatorSettingsMainDrawWireDiamond GetShapeOfTypeWireDiamond(bool& Success, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "ManipulatorTools|Shapes")
	void SetShapeOfTypeWireDiamond(int32 Index, FManipulatorSettingsMainDrawWireDiamond WireDiamond);


	// ========= CIRCLES =========

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	TArray<FManipulatorSettingsMainDrawCircle> GetAllShapesOfTypeWireCircle();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	FManipulatorSettingsMainDrawCircle GetShapeOfTypeWireCircle(bool& Success, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "ManipulatorTools|Shapes")
	void SetShapeOfTypeWireCircle(int32 Index, FManipulatorSettingsMainDrawCircle Circle);


	// ========= PLANES =========

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	TArray<FManipulatorSettingsMainDrawPlane> GetAllShapesOfTypePlane();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ManipulatorTools|Shapes")
	FManipulatorSettingsMainDrawPlane GetShapeOfTypePlane(bool& Success, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "ManipulatorTools|Shapes")
	void SetShapeOfTypePlane(int32 Index, FManipulatorSettingsMainDrawPlane Plane);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

};
