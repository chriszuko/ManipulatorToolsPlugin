// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorToolsEditorEdMode.h"
#include "ManipulatorToolsEditorEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "Sequencer/Public/ISequencer.h"
#include "MovieSceneSequence.h"
#include "MovieSceneTrack.h"
#include "MovieScene.h"
#include "MovieSceneTransformTrack.h"
#include "MovieSceneVectorTrack.h"
#include "MovieSceneByteTrack.h"
#include "MovieSceneBoolTrack.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "LevelEditorViewport.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/UObjectIterator.h"
#include "Materials/Material.h"
#include "ManipulatorToolsEditor.h"

const FEditorModeID FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId = TEXT("EM_ManipulatorToolsEditorEdMode");

/** Hit proxy used for editable properties */
struct HManipulatorProxy : public HHitProxy
{
	DECLARE_HIT_PROXY();

	/** Component this hit proxy will talk to. */
	UManipulatorComponent* ManipulatorComponent;

	/** This property is a transform */
	bool	bPropertyIsTransform;

	// Constructor
	HManipulatorProxy(UManipulatorComponent* ManipulatorComponent) : HHitProxy(HPP_Foreground) , ManipulatorComponent(ManipulatorComponent)
	{
	}

	/** Show cursor as cross when over this handle */
	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::Crosshairs;
	}
};

IMPLEMENT_HIT_PROXY(HManipulatorProxy, HHitProxy);

// A bunch of functions that EdMode Uses to get properties by name that I don't have access to because 
// EdMode is dumb and doesn't put it in the header so I followed their example and copied it to my cpp. SUCK IT UNREAL.
namespace
{
	/**
	 * Returns a reference to the named property value data in the given container.
	 */
	template<typename T>
	T* GetPropertyValuePtrByName(const UStruct* InStruct, void* InContainer, FString PropertyName, int32 ArrayIndex, UProperty*& OutProperty)
	{
		T* ValuePtr = NULL;

		// Extract the vector ptr recursively using the property name
		int32 DelimPos = PropertyName.Find(TEXT("."));
		if (DelimPos != INDEX_NONE)
		{
			// Parse the property name and (optional) array index
			int32 SubArrayIndex = 0;
			FString NameToken = PropertyName.Left(DelimPos);
			int32 ArrayPos = NameToken.Find(TEXT("["));
			if (ArrayPos != INDEX_NONE)
			{
				FString IndexToken = NameToken.RightChop(ArrayPos + 1).LeftChop(1);
				SubArrayIndex = FCString::Atoi(*IndexToken);

				NameToken = PropertyName.Left(ArrayPos);
			}

			// Obtain the property info from the given structure definition
			UProperty* CurrentProp = FindField<UProperty>(InStruct, FName(*NameToken));
			// Check first to see if this is a simple structure (i.e. not an array of structures)
			UStructProperty* StructProp = Cast<UStructProperty>(CurrentProp);
			if (StructProp != NULL)
			{
				// Recursively call back into this function with the structure property and container value
				ValuePtr = GetPropertyValuePtrByName<T>(StructProp->Struct, StructProp->ContainerPtrToValuePtr<void>(InContainer), PropertyName.RightChop(DelimPos + 1), ArrayIndex, OutProperty);
			}
			else
			{
				// Check to see if this is an array
				UArrayProperty* ArrayProp = Cast<UArrayProperty>(CurrentProp);
				if (ArrayProp != NULL)
				{
					// It is an array, now check to see if this is an array of structures
					StructProp = Cast<UStructProperty>(ArrayProp->Inner);
					if (StructProp != NULL)
					{
						FScriptArrayHelper_InContainer ArrayHelper(ArrayProp, InContainer);
						if (ArrayHelper.IsValidIndex(SubArrayIndex))
						{
							// Recursively call back into this function with the array element and container value
							ValuePtr = GetPropertyValuePtrByName<T>(StructProp->Struct, ArrayHelper.GetRawPtr(SubArrayIndex), PropertyName.RightChop(DelimPos + 1), ArrayIndex, OutProperty);
						}
					}
				}
			}
		}
		else
		{
			UProperty* Prop = FindField<UProperty>(InStruct, FName(*PropertyName));
			if (Prop != NULL)
			{
				if (UArrayProperty* ArrayProp = Cast<UArrayProperty>(Prop))
				{
					check(ArrayIndex != INDEX_NONE);

					// Property is an array property, so make sure we have a valid index specified
					FScriptArrayHelper_InContainer ArrayHelper(ArrayProp, InContainer);
					if (ArrayHelper.IsValidIndex(ArrayIndex))
					{
						ValuePtr = (T*)ArrayHelper.GetRawPtr(ArrayIndex);
					}
				}
				else
				{
					// Property is a vector property, so access directly
					ValuePtr = Prop->ContainerPtrToValuePtr<T>(InContainer);
				}

				OutProperty = Prop;
			}
		}

		return ValuePtr;
	}

	/**
	 * Returns the value of the property with the given name in the given Actor instance.
	 */
	template<typename T>
	T GetPropertyValueByName(UObject* Object, FString PropertyName, int32 PropertyIndex)
	{
		T Value;
		UProperty* DummyProperty = NULL;
		if (T* ValuePtr = GetPropertyValuePtrByName<T>(Object->GetClass(), Object, PropertyName, PropertyIndex, DummyProperty))
		{
			Value = *ValuePtr;
		}
		else
		{
			Value = T();
		}
		return Value;
	}

	/**
	 * Sets the property with the given name in the given Actor instance to the given value.
	 */
	template<typename T>
	void SetPropertyValueByName(UObject* Object, FString PropertyName, int32 PropertyIndex, const T& InValue, UProperty*& OutProperty)
	{
		if (T* ValuePtr = GetPropertyValuePtrByName<T>(Object->GetClass(), Object, PropertyName, PropertyIndex, OutProperty))
		{
			*ValuePtr = InValue;
		}
	}
}


FManipulatorToolsEditorEdMode::FManipulatorToolsEditorEdMode()
{
}

FManipulatorToolsEditorEdMode::~FManipulatorToolsEditorEdMode()
{

}

void FManipulatorToolsEditorEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FManipulatorToolsEditorEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	FManipulatorToolsEditorModule& ManipulatorToolsModule = FModuleManager::Get().LoadModuleChecked<FManipulatorToolsEditorModule>("ManipulatorToolsEditor");
	SetSequencer(ManipulatorToolsModule.GetSequencer());
}

void FManipulatorToolsEditorEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();

	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

void FManipulatorToolsEditorEdMode::Render(const FSceneView * View, FViewport * Viewport, FPrimitiveDrawInterface * PDI)
{
	// Get the first selected actor, walk through its children components to find the custom manipulators and then draw their proxies and visual representations to edit.
	//AActor* SelectedActor = GetFirstSelectedActorInstance();
	if (Viewport->GetClient()->IsInGameView())
	{
		return;
	}

	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
	for(AActor* SelectedActor : SelectedActors)
	{
		if (SelectedActor != nullptr && Owner->GetSelectedComponents()->GetTop<USceneComponent>() == nullptr)
		{
			TArray<UActorComponent*> Manipulators = SelectedActor->GetComponentsByClass(UManipulatorComponent::StaticClass());
			for (UActorComponent* ActorComponent : Manipulators)
			{
				UManipulatorComponent* ManipulatorComponent = Cast<UManipulatorComponent>(ActorComponent);
				// Visibility also controls whether or not it will draw.
				if (ManipulatorComponent != nullptr && ManipulatorComponent->IsVisible())
				{
					
					// Set Color Based off of selection, bools handle their selection a bit different. 
					FLinearColor DrawColor = ManipulatorComponent->Settings.Draw.BaseColor;
					if ((ManipulatorComponent->GetName() == EditedComponentName && ManipulatorComponent->Settings.Property.NameToEdit == EditedPropertyName && ManipulatorComponent->GetOwner()->GetName() == EditedActorName) || GetBoolPropertyValue(ManipulatorComponent))
					{
						DrawColor = ManipulatorComponent->Settings.Draw.SelectedColor /* * SelectedColorMuliplier*/;
					}

					// Declaration of variables before drawing anything
					FTransform WidgetTransform = GetManipulatorTransformWithOffsets(ManipulatorComponent);

					ESceneDepthPriorityGroup WidgetDepthPriority = ManipulatorComponent->Settings.Draw.Extras.DepthPriorityGroup;
					float WidgetThickness = 1;
					FTransform WidgetOverallSize = FTransform();
					WidgetOverallSize.SetScale3D(FVector(ManipulatorComponent->Settings.Draw.OverallSize, ManipulatorComponent->Settings.Draw.OverallSize, ManipulatorComponent->Settings.Draw.OverallSize));

					//Used for the offset based on zoom
					float WidgetSizeMultiplier = 1;
					if (ManipulatorComponent->Settings.Draw.Extras.UseZoomOffset)
					{
						const float ZoomFactor = FMath::Min<float>(View->ViewMatrices.GetProjectionMatrix().M[0][0], View->ViewMatrices.GetProjectionMatrix().M[1][1]);
						WidgetSizeMultiplier = View->Project(WidgetTransform.GetTranslation()).W * 0.0065f / ZoomFactor;
					}

					// =========================  WIRE BOX  =========================
					for (FManipulatorSettingsMainDrawWireBox WireBox : ManipulatorComponent->GetAllShapesOfTypeWireBox())
					{
						
						// Create Hit Proxy
						PDI->SetHitProxy(new HManipulatorProxy(ManipulatorComponent));
						FTransform WireBoxTransform = WidgetTransform;
						WireBoxTransform = ManipulatorComponent->CombineOffsetTransforms(WireBox.Offsets) * WidgetOverallSize * WireBoxTransform;
						FMatrix WidgetMatrix = WireBoxTransform.ToMatrixWithScale();

						// Set Box Size
						FBox BoxSize = WireBox.BoxSize;
						BoxSize.Min = BoxSize.Min * WireBox.SizeMultiplier;
						BoxSize.Max = BoxSize.Max * WireBox.SizeMultiplier;
						WidgetThickness = WireBox.DrawThickness;
						FLinearColor DrawBoxColor = DrawColor * WireBox.Color;

						// Draw the box
						
						DrawWireBox(PDI, WidgetMatrix, BoxSize, DrawBoxColor, WidgetDepthPriority, WidgetThickness);
					}

					// =========================  WIRE DIAMOND  =========================
					for (FManipulatorSettingsMainDrawWireDiamond WireDiamond : ManipulatorComponent->GetAllShapesOfTypeWireDiamond())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(new HManipulatorProxy(ManipulatorComponent));
						FTransform WireDiamondTransform = WidgetTransform;
						WireDiamondTransform = ManipulatorComponent->CombineOffsetTransforms(WireDiamond.Offsets) * WidgetOverallSize * WireDiamondTransform;
						FMatrix WidgetMatrix = WireDiamondTransform.ToMatrixWithScale();

						float DiamondSize = WireDiamond.Size * WidgetSizeMultiplier;
						WidgetThickness = WireDiamond.DrawThickness;
						FLinearColor DrawDiamondColor = DrawColor * WireDiamond.Color;


						DrawWireDiamond(PDI, WidgetMatrix, DiamondSize, DrawDiamondColor, WidgetDepthPriority, WidgetThickness);

					}

					// =========================  PLANE  =========================
					for (FManipulatorSettingsMainDrawPlane Plane : ManipulatorComponent->GetAllShapesOfTypePlane())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(new HManipulatorProxy(ManipulatorComponent));
						FTransform PlaneTransform = WidgetTransform;
						PlaneTransform = ManipulatorComponent->CombineOffsetTransforms(Plane.Offsets) * WidgetOverallSize * PlaneTransform;
						FMatrix WidgetMatrix = PlaneTransform.ToMatrixWithScale();

						float PlaneSize = Plane.Size;
						float UVMin = Plane.UVMin;
						float UVMax = Plane.UVMax;

						if (Plane.Material == nullptr)
						{
							FString MaterialPath = "/ManipulatorTools/HardCoded/MM_ManipulatorTools_ShapePlane.MM_ManipulatorTools_ShapePlane";
							Plane.Material = (UMaterial*)StaticLoadObject(UMaterial::StaticClass(), NULL, *MaterialPath, NULL, LOAD_None, NULL);
						}
						UMaterialInterface* Material = Plane.Material;
						UMaterialInstanceDynamic* MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Material, NULL);

						FLinearColor DrawPlaneColor = DrawColor * Plane.Color;

						MaterialInstanceDynamic->SetVectorParameterValue(FName("DrawColor"), DrawPlaneColor);
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION > 21
						FMaterialRenderProxy* RenderProxy = MaterialInstanceDynamic->GetRenderProxy();
#else
						FMaterialRenderProxy* RenderProxy = MaterialInstanceDynamic->GetRenderProxy(false);
#endif
						DrawPlane10x10(PDI, WidgetMatrix, PlaneSize, FVector2D(UVMin, UVMin), FVector2D(UVMax, UVMax), RenderProxy, WidgetDepthPriority);
					}

					// =========================  CIRCLE  =========================
					for (FManipulatorSettingsMainDrawCircle Circle : ManipulatorComponent->GetAllShapesOfTypeWireCircle())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(new HManipulatorProxy(ManipulatorComponent));
						FTransform CircleTransform = WidgetTransform;
						CircleTransform = ManipulatorComponent->CombineOffsetTransforms(Circle.Offsets) * WidgetOverallSize * CircleTransform;

						FVector X = CircleTransform.GetRotation().RotateVector(Circle.Rotation.RotateVector(FVector(1, 0, 0)) * CircleTransform.GetScale3D());
						FVector Y = CircleTransform.GetRotation().RotateVector(Circle.Rotation.RotateVector(FVector(0, 1, 0)) * CircleTransform.GetScale3D());
						float Radius = Circle.Radius;
						float NumSides = Circle.NumSides;
						WidgetThickness = Circle.DrawThickness;
						FLinearColor DrawCircleColor = DrawColor * Circle.Color;

						DrawCircle(PDI, CircleTransform.GetLocation(), X, Y, DrawCircleColor, Radius, NumSides, WidgetDepthPriority, WidgetThickness, 0, false);
					}
				}
			}
		}
	}
	FEdMode::Render(View, Viewport, PDI);
}

bool FManipulatorToolsEditorEdMode::HandleClick(FEditorViewportClient * InViewportClient, HHitProxy * HitProxy, const FViewportClick & Click)
{
	// Sets the current edited component to look at when clicked we have to name match because components 
	// get destroyed and recreated on construct making it impossible to just simply hard reference it.
	if (HitProxy != nullptr && HitProxy->IsA(HManipulatorProxy::StaticGetType()))
	{
		HManipulatorProxy* PropertyProxy = (HManipulatorProxy*)HitProxy;
		//Handle Toggling Bool on and Off.
		if (PropertyProxy->ManipulatorComponent->Settings.Property.Type == EManipulatorPropertyType::MT_BOOL)
		{
			ToggleBoolPropertyValue(PropertyProxy->ManipulatorComponent);
		}
		else
		{
			EditedPropertyName = PropertyProxy->ManipulatorComponent->Settings.Property.NameToEdit;
			EditedActorName = PropertyProxy->ManipulatorComponent->GetOwner()->GetName();
			EManipulatorPropertyType propertyType = PropertyProxy->ManipulatorComponent->Settings.Property.Type;
			EditedComponentName = PropertyProxy->ManipulatorComponent->GetName();
			EditedPropertyIndex = PropertyProxy->ManipulatorComponent->Settings.Property.Index;
			HandleSequencerTrackSelection(propertyType, FName(*EditedPropertyName), PropertyProxy->ManipulatorComponent);
		}
		return true;
	}
	// Clear Info when de-selecting a Hit proxy
	else if (HitProxy != nullptr && HitProxy->IsA(HActor::StaticGetType()) && Click.IsShiftDown())
	{
		EditedPropertyName = FString();
		EditedComponentName = FString();
		EditedActorName = FString();
		EditedPropertyIndex = INDEX_NONE;
		bEditedPropertyIsTransform = false;
	}
	FEdMode::HandleClick(InViewportClient, HitProxy, Click);

	return false;
}

FVector FManipulatorToolsEditorEdMode::GetWidgetLocation() const
{
	// Update the widget location so that it doesn't leave you with odd relative offset stuff.
	UManipulatorComponent* ManipulatorComponent = NewObject<UManipulatorComponent>();
	FTransform WidgetTransform = FTransform::Identity;
	if (GetSelectedManipulatorComponent(ManipulatorComponent, WidgetTransform))
	{
		FVector WorldLocation = Owner->PivotLocation;
		// Handle Enum property offsets
		WidgetTransform = GetManipulatorTransformWithOffsets(ManipulatorComponent);
		// Do some crazy magical shit to offset the widget location
		WorldLocation = WidgetTransform.GetLocation();
		return WorldLocation;
	}
	FEdMode::GetWidgetLocation();
	return Owner->PivotLocation;
}

bool FManipulatorToolsEditorEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector & InDrag, FRotator & InRot, FVector & InScale)
{
	// The input delta is what tells the widget how much to adjust its value by based on user input. 
	UManipulatorComponent* ManipulatorComponent;
	FTransform WidgetTransform = FTransform::Identity;
	if (GetSelectedManipulatorComponent(ManipulatorComponent, WidgetTransform) && InViewportClient->GetCurrentWidgetAxis() != EAxisList::None)
	{
		// Get the object to edit properties is the only way I could correctly get something that talked nicely to the get property value by name. 
		UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFor(WidgetTransform, ManipulatorComponent);
		if (ObjectToEditProperties != nullptr)
		{
			// Not sure what this does.. but i kept it.
			GEditor->NoteActorMovement();

			if (!EditedPropertyName.IsEmpty())
			{
				FTransform LocalTM = FTransform::Identity;
				FTransform NewTM = FTransform::Identity;
				FVector LocalLocation = FVector();
				uint8 EnumValue = 0;
				
				FTransform DeltaTransform = FTransform(InRot, InDrag, InScale);
				float CurrentFloat = 0;

				// Get the property values based off of their type on the component and the name on the component.
				switch (ManipulatorComponent->Settings.Property.Type)
				{
				case EManipulatorPropertyType::MT_TRANSFORM:
					// Get Property Here
					LocalTM = GetPropertyValueByName<FTransform>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex);
					break;
				case EManipulatorPropertyType::MT_VECTOR:
					LocalLocation = GetPropertyValueByName<FVector>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex);
					LocalTM = FTransform(LocalLocation);
					break;
				case EManipulatorPropertyType::MT_ENUM:
					EnumValue = GetPropertyValueByName<uint8>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex);
					break;
				}

				// Use the visual offset to correctly translate information on super visually offset widgets.
				if (ManipulatorComponent->Settings.Draw.Extras.UsePropertyValueAsInitialOffset)
				{
					WidgetTransform.SetRotation(WidgetTransform.GetRotation() * ManipulatorComponent->CombineOffsetTransforms(ManipulatorComponent->Settings.Draw.Offsets).GetRotation());
				}

				// Flip Transforms if told to flip X
				LocalTM = FlipTransformOnX(LocalTM, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXLocation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualYRotation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXScale);

				// Calculate world transform
				NewTM = LocalTM * WidgetTransform;

				// Apply delta in world space
				NewTM.SetTranslation(NewTM.GetTranslation() + InDrag);

				NewTM.SetRotation(InRot.Quaternion() * NewTM.GetRotation());
				// Convert new world transform back into local space
				LocalTM = NewTM.GetRelativeTransform(WidgetTransform);
				LocalTM.SetRotation(LocalTM.GetRotation());
				// Apply delta scale
				LocalTM.SetScale3D(LocalTM.GetScale3D() + InScale);

				// Flip Transform Back so the values are correct
				LocalTM = FlipTransformOnX(LocalTM, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXLocation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualYRotation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXScale);

				LocalTM = ManipulatorComponent->ConstrainTransform(LocalTM);



				// Prepare for editing
				ObjectToEditProperties->PreEditChange(NULL);
				UProperty* SetProperty = NULL;

				// Set the property values based off of their type on the component and the name on the component.
				switch (ManipulatorComponent->Settings.Property.Type)
				{
				case EManipulatorPropertyType::MT_TRANSFORM:
					// Get Property Here
					SetPropertyValueByName<FTransform>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex, LocalTM, SetProperty);
					break;
				case EManipulatorPropertyType::MT_VECTOR:
					SetPropertyValueByName<FVector>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex, LocalTM.GetLocation(), SetProperty);
					break;
				case EManipulatorPropertyType::MT_ENUM:
					//Handle Enum Change
					EnumValue = CalculateEnumPropertyInputDelta(ManipulatorComponent, LocalTM, EnumValue);
					SetPropertyValueByName<uint8>(ObjectToEditProperties, EditedPropertyName, EditedPropertyIndex, EnumValue, SetProperty);
					break;
				}

				KeyProperty(ObjectToEditProperties, SetProperty);

				FPropertyChangedEvent PropertyChangeEvent(SetProperty);
				ObjectToEditProperties->PostEditChangeProperty(PropertyChangeEvent);
				return true;
			}
		}
	}

	FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
	return false;
}

bool FManipulatorToolsEditorEdMode::AllowWidgetMove()
{
	return true;
}

bool FManipulatorToolsEditorEdMode::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
{
	// Mostly copied code from EdMode to make Transforms correctly display their custom axis information when editing.
	UManipulatorComponent* ManipulatorComponent = NewObject<UManipulatorComponent>();
	FTransform WidgetTransform = FTransform::Identity;
	if (GetSelectedManipulatorComponent(ManipulatorComponent, WidgetTransform))
	{
		WidgetTransform = GetManipulatorTransformWithOffsets(ManipulatorComponent);
		FTransform DisplayWidgetToWorld;
		FTransform LocalTM;
		UObject* BestSelectedItem = GetObjectToDisplayWidgetsFor(DisplayWidgetToWorld, ManipulatorComponent);
		if (BestSelectedItem && EditedPropertyName != TEXT(""))
		{
			switch (ManipulatorComponent->Settings.Property.Type)
			{
			case EManipulatorPropertyType::MT_TRANSFORM:

				//LocalTM = GetPropertyValueByName<FTransform>(BestSelectedItem, EditedPropertyName, EditedPropertyIndex);
				InMatrix = FRotationMatrix::Make((WidgetTransform).GetRotation());
				return true;
				break;
			case EManipulatorPropertyType::MT_VECTOR:
				InMatrix = FRotationMatrix::Make(WidgetTransform.GetRotation());
				return true;
				break;
			case EManipulatorPropertyType::MT_ENUM:
				InMatrix = FRotationMatrix::Make(WidgetTransform.GetRotation());
				return true;
				break;
			}
		}
	}
	return false;
}

void FManipulatorToolsEditorEdMode::ActorSelectionChangeNotify()
{
}

uint8 FManipulatorToolsEditorEdMode::CalculateEnumPropertyInputDelta(UManipulatorComponent* ManipulatorComponent, FTransform LocalTM, uint8 EnumInput)
{
	//Use the direction vector * Step to calulate the offset position of the current enum.
	FManipulatorSettingsMainPropertyTypeEnum EnumSettings = ManipulatorComponent->Settings.Property.EnumSettings;
	float AxisCheck = 0;
	float EnumAsFloat = EnumInput;
	switch (ManipulatorComponent->Settings.Property.EnumSettings.Direction)
	{
	case EManipulatorPropertyEnumDirection::MT_X:
		AxisCheck = LocalTM.GetLocation().X;
		break;
	case EManipulatorPropertyEnumDirection::MT_Y:
		AxisCheck = LocalTM.GetLocation().Y;
		break;
	case EManipulatorPropertyEnumDirection::MT_Z:
		AxisCheck = LocalTM.GetLocation().Z;
		break;
	}

	//add and clamp output.
	EnumAsFloat = EnumAsFloat + (FMath::GridSnap(AxisCheck, EnumSettings.StepSize) / EnumSettings.StepSize);
	if (EnumAsFloat > EnumSettings.EnumSize - 1)
	{
		EnumAsFloat = EnumSettings.EnumSize - 1;
	}
	else if (EnumAsFloat < 0)
	{
		EnumAsFloat = 0;
	}
	return uint8(EnumAsFloat);
}

bool FManipulatorToolsEditorEdMode::GetBoolPropertyValue(UManipulatorComponent* ManipulatorComponent)
{
	// Used to tell bools when to change color.
	bool Output = false;
	if (ManipulatorComponent->Settings.Property.Type == EManipulatorPropertyType::MT_BOOL)
	{
		FTransform WidgetTransform;
		UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFor(WidgetTransform, ManipulatorComponent);
		if (ObjectToEditProperties != nullptr)
		{
			Output = GetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);
		}
	}
	return Output;
}

void FManipulatorToolsEditorEdMode::ToggleBoolPropertyValue(UManipulatorComponent* ManipulatorComponent)
{
	// Toggles a bool on and off when clicked. 
	if (ManipulatorComponent->Settings.Property.Type == EManipulatorPropertyType::MT_BOOL)
	{
		bool CurrentBool = false;
		FTransform WidgetTransform;
		UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFor(WidgetTransform, ManipulatorComponent);
		if (ObjectToEditProperties != nullptr)
		{
			CurrentBool = GetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);

			// Set Bool Value
			ObjectToEditProperties->PreEditChange(NULL);
			UProperty* SetProperty = NULL;
			SetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index, !CurrentBool, SetProperty);

			KeyProperty(ObjectToEditProperties, SetProperty);

			FPropertyChangedEvent PropertyChangeEvent(SetProperty);
			ObjectToEditProperties->PostEditChangeProperty(PropertyChangeEvent);
		}
	}
}

FTransform FManipulatorToolsEditorEdMode::FlipTransformOnX(FTransform Transform, bool FlipXVector, bool FlipYRotation, bool FlipXScale) const
{

	
	// Flip Yaw Of Rotation
	if (FlipYRotation)
	{
		//FRotator Rotator = Transform.GetRotation().Rotator();
		FQuat Rotation = Transform.GetRotation();
		FVector ForwardVector = Rotation.GetForwardVector();
		ForwardVector = ForwardVector * FVector(1,1,-1);
		ForwardVector.Normalize();

		FVector RightVector = Rotation.GetRightVector();
		RightVector.Normalize();
		FVector UpVector = Rotation.GetUpVector();
		UpVector.Normalize();

		FMatrix RotMatrix(ForwardVector, RightVector, UpVector, FVector::ZeroVector);
		Transform.SetRotation(RotMatrix.Rotator().Quaternion());
	}

	Transform.NormalizeRotation();
	// Flip Location
	if (FlipXVector)
	{
		Transform.SetLocation(Transform.GetLocation()*FVector(-1, 1, 1));
	}

	// Flip Scale
	if (FlipXScale)
	{
		Transform.SetScale3D(Transform.GetScale3D() * FVector(-1, 1, 1));
	}

	return Transform;
}

FTransform FManipulatorToolsEditorEdMode::GetManipulatorTransformWithOffsets(UManipulatorComponent * ManipulatorComponent) const
{
	// Enum Offsets
	FVector EnumPropertyOffset = FVector(0, 0, 0);
	uint8 EnumValue = 0;

	// Visual Offset and Relative Offset
	FTransform RelativeTransform = FTransform::Identity;
	FTransform WidgetTransform = FTransform::Identity;
	UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFor(WidgetTransform, ManipulatorComponent);

	// Handle offsets per property type bools are ignored in here because they are essentially world buttons.
	switch (ManipulatorComponent->Settings.Property.Type)
	{
	case EManipulatorPropertyType::MT_ENUM:
		if (ObjectToEditProperties != nullptr)
		{
			EnumValue = GetPropertyValueByName<uint8>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);

			//Use the direction vector * Step to calulate the offset position of the current enum.
			FManipulatorSettingsMainPropertyTypeEnum Settings = ManipulatorComponent->Settings.Property.EnumSettings;

			switch (ManipulatorComponent->Settings.Property.EnumSettings.Direction)
			{
			case EManipulatorPropertyEnumDirection::MT_X:
				EnumPropertyOffset.X = Settings.StepSize * EnumValue;
				break;
			case EManipulatorPropertyEnumDirection::MT_Y:
				EnumPropertyOffset.Y = Settings.StepSize * EnumValue;
				break;
			case EManipulatorPropertyEnumDirection::MT_Z:
				EnumPropertyOffset.Z = Settings.StepSize * EnumValue;
				break;
			}
		}
		break;
	case EManipulatorPropertyType::MT_TRANSFORM:
	{
		RelativeTransform = GetPropertyValueByName<FTransform>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);
		break;
	}
	case EManipulatorPropertyType::MT_VECTOR:
	{
		RelativeTransform = FTransform(GetPropertyValueByName<FVector>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index));
		break;
	}
	}

	// Constrain the relative transform via the manipulator components settings.
	RelativeTransform = ManipulatorComponent->ConstrainTransform(RelativeTransform);
	RelativeTransform = FlipTransformOnX(RelativeTransform, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXLocation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualYRotation, ManipulatorComponent->Settings.Draw.Extras.FlipVisualXScale);

	if (!ManipulatorComponent->Settings.Draw.Extras.UsePropertyValueAsInitialOffset)
	{
		RelativeTransform = FTransform::Identity;
	}

	RelativeTransform.NormalizeRotation();

	// Compose Relative Transform, Enum Offset, Visual Offset and Actor Transform together to get the final Widget Transform.
	WidgetTransform = RelativeTransform * FTransform(EnumPropertyOffset) * ManipulatorComponent->CombineOffsetTransforms(ManipulatorComponent->Settings.Draw.Offsets) * ManipulatorComponent->GetOwner()->GetActorTransform();
	WidgetTransform.NormalizeRotation();
	return WidgetTransform;
}

UObject * FManipulatorToolsEditorEdMode::GetObjectToDisplayWidgetsFor(FTransform & OutLocalToWorld, UManipulatorComponent* ManipulatorComponent) const
{
	// Determine what is selected, preferring a component over an actor
	USceneComponent* SelectedComponent = ManipulatorComponent->GetOwner()->GetRootComponent();
	UObject* BestSelectedItem = ManipulatorComponent->GetOwner();
	OutLocalToWorld = SelectedComponent->GetComponentToWorld();
	return BestSelectedItem; 
}

bool FManipulatorToolsEditorEdMode::GetSelectedManipulatorComponent( UManipulatorComponent*& OutComponent, FTransform& OutWidgetTransform) const
{
	// Finds the first actor then walks through the components to find the currently selected component based off of component name and property to edit. 
	// Outputs false if at any point any of the out information is null or fails.
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
	for (AActor* SelectedActor : SelectedActors)
	{
		if (SelectedActor != nullptr && SelectedActor->GetName() == EditedActorName)
		{
			TArray<UActorComponent*> Manipulators = SelectedActor->GetComponentsByClass(UManipulatorComponent::StaticClass());
			for (UActorComponent* ActorComponent : Manipulators)
			{
				UManipulatorComponent* ManipulatorComponent = Cast<UManipulatorComponent>(ActorComponent);
				if (ManipulatorComponent != nullptr)
				{
					if (ManipulatorComponent->Settings.Property.NameToEdit == EditedPropertyName && ManipulatorComponent->GetName() == EditedComponentName)
					{
						OutComponent = ManipulatorComponent;
						OutWidgetTransform = ManipulatorComponent->GetComponentToWorld();
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool FManipulatorToolsEditorEdMode::UsesToolkits() const
{
	return true;
}

void FManipulatorToolsEditorEdMode::KeyProperty(UObject* ObjectToKey, UProperty* propertyToUse)
{
	if (WeakSequencer != nullptr)
	{
		TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();

		auto AutoKeyMode = Sequencer->GetAutoChangeMode();

		if (AutoKeyMode != EAutoChangeMode::None)
		{
			TArray<UObject*> ObjectsToKey;
			ObjectsToKey.Add(ObjectToKey);

			FPropertyPath PropertyPath;
			PropertyPath.AddProperty(FPropertyInfo(propertyToUse));

			// May change ManualKeyForced to ManualKey.
			FKeyPropertyParams KeyPropertyParams(ObjectsToKey, PropertyPath, ESequencerKeyMode::AutoKey);
			Sequencer->KeyProperty(KeyPropertyParams);
		}
	}
}

void FManipulatorToolsEditorEdMode::HandleSequencerTrackSelection(const EManipulatorPropertyType& propertyType, const FName& propertyName, UManipulatorComponent* ManipulatorComponent)
{		
	// Handle updating the selected track when selecting a manipulator.
	if (WeakSequencer != nullptr)
	{
		TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();
		UMovieSceneSequence* sequenceScene = Sequencer->GetFocusedMovieSceneSequence();
		UMovieScene* scene = sequenceScene->GetMovieScene();
		auto bindings = scene->GetBindings();
		AActor* SelectedActor = ManipulatorComponent->GetOwner();
		for (auto binding : bindings)
		{
			FTransform WidgetTransform = FTransform::Identity;
			UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFor(WidgetTransform, ManipulatorComponent);
			if (ObjectToEditProperties != nullptr && ObjectToEditProperties->GetName().Contains(binding.GetName()))
			{
				UClass* trackClass = nullptr;
				switch (propertyType)
				{
				case EManipulatorPropertyType::MT_TRANSFORM:
				{
					trackClass = UMovieSceneTransformTrack::StaticClass();
					break;
				}
				case EManipulatorPropertyType::MT_VECTOR:
				{
					trackClass = UMovieSceneVectorTrack::StaticClass();
					break;
				}
				case EManipulatorPropertyType::MT_ENUM:
				{
					trackClass = UMovieSceneByteTrack::StaticClass();
					break;
				}
				case EManipulatorPropertyType::MT_BOOL:
				{
					trackClass = UMovieSceneBoolTrack::StaticClass();
					break;
				}
				}

				if (trackClass != nullptr)
				{
					UMovieSceneTrack* track = scene->FindTrack(trackClass, binding.GetObjectGuid(), propertyName);
					if (track != nullptr)
					{
						WeakSequencer.Pin()->EmptySelection();
						// TODO: This can likely be updated to use the below to clean this up. Just need to get the propertyPath.
						//TArray<FString> propertyNames;
						//propertyNames.Add(propertyName.ToString());
						//WeakSequencer.Pin()->SelectByPropertyPaths(propertyNames);
						WeakSequencer.Pin()->SelectTrack(track);
					}
				}
			}
		}
	}

}

void FManipulatorToolsEditorEdMode::HandleSelectedColorMultiplier(float DeltaTime)
{
	float AddedTime = 0;
	float Speed = DeltaTime * 0.8f;
	float Min = 0.95f;
	float Max = 1.2f;
	// Simple linear animation for making the manipulator flash when selected
	if (SelectedColorMultiplierDirection)
	{
		AddedTime = AddedTime + Speed;
	}
	else
	{
		AddedTime = AddedTime - Speed;
	}
	SelectedColorMuliplier = SelectedColorMuliplier + AddedTime;

	FMath::Clamp(SelectedColorMuliplier, Min, Max);

	if (SelectedColorMuliplier >= Max || SelectedColorMuliplier <= Min)
	{
		SelectedColorMultiplierDirection = !SelectedColorMultiplierDirection;
	}
}

void FManipulatorToolsEditorEdMode::Tick(FEditorViewportClient * ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);
	HandleSelectedColorMultiplier(DeltaTime);
}

void FManipulatorToolsEditorEdMode::SetSequencer(TWeakPtr<ISequencer> InSequencer)
{
	WeakSequencer = InSequencer;
	if (UsesToolkits())
	{
		// TODO: Reference ControlRigEditMode for this. Getting this setup may be a more correct way of doing auto key.
		//StaticCastSharedPtr<SControlRigEditModeTools>(Toolkit->GetInlineContent())->SetSequencer(InSequencer);
	}
}