// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorToolsEditorEdMode.h"
#include "ManipulatorToolsEditorEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "ISequencer.h"
#include "MovieSceneSequence.h"
#include "MovieSceneTrack.h"
#include "MovieScene.h"
#include "MovieSceneTransformTrack.h"
#include "MovieSceneVectorTrack.h"
#include "MovieSceneByteTrack.h"
#include "MovieSceneBoolTrack.h"
#include "MovieScenePropertyTrack.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "LevelEditorViewport.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/UObjectIterator.h"
#include "Materials/Material.h"
#include "ManipulatorToolsEditor.h"

const FEditorModeID FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId = TEXT("EM_ManipulatorToolsEditorEdMode");

/* ---------- FEdMode Interface ---------- */

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
		ClearManipulatorSelection();
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
	// Update Sequencer Tracks
	SelectedManipulators = NewSelectedManipulators;
	SequencerUpdateTrackSelection();

	// Update Visuals
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
					FManipulatorData* ManipulatorData = GetManipulatorData(ManipulatorComponent);

					// Set Color Based off of selection, bools handle their selection a bit different. 
					FLinearColor DrawColor = ManipulatorComponent->Settings.Draw.BaseColor;
					if (ManipulatorComponent->GetManipulatorID() == ManipulatorData->ID || GetBoolPropertyValueFromManipulator(ManipulatorComponent))
					{
						DrawColor = ManipulatorComponent->Settings.Draw.SelectedColor;
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

					// Make HitProxy
					HManipulatorProxy* HitProxy = new HManipulatorProxy(ManipulatorComponent);

					// ==========  WIRE BOX  ==========
					for (FManipulatorSettingsMainDrawWireBox WireBox : ManipulatorComponent->GetAllShapesOfTypeWireBox())
					{
						
						// Create Hit Proxy
						PDI->SetHitProxy(HitProxy);
						FTransform WireBoxTransform = WidgetTransform;
						WireBoxTransform = HandleFinalShapeTransforms(ManipulatorComponent->CombineOffsetTransforms(WireBox.Offsets), WidgetOverallSize, WireBoxTransform);
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

					// ==========  WIRE DIAMOND  ==========
					for (FManipulatorSettingsMainDrawWireDiamond WireDiamond : ManipulatorComponent->GetAllShapesOfTypeWireDiamond())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(HitProxy);
						FTransform WireDiamondTransform = WidgetTransform;
						WireDiamondTransform = HandleFinalShapeTransforms(ManipulatorComponent->CombineOffsetTransforms(WireDiamond.Offsets), WidgetOverallSize, WireDiamondTransform);
						FMatrix WidgetMatrix = WireDiamondTransform.ToMatrixWithScale();

						float DiamondSize = WireDiamond.Size * WidgetSizeMultiplier;
						WidgetThickness = WireDiamond.DrawThickness;
						FLinearColor DrawDiamondColor = DrawColor * WireDiamond.Color;


						DrawWireDiamond(PDI, WidgetMatrix, DiamondSize, DrawDiamondColor, WidgetDepthPriority, WidgetThickness);

					}

					// ==========  PLANE  ==========
					for (FManipulatorSettingsMainDrawPlane Plane : ManipulatorComponent->GetAllShapesOfTypePlane())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(HitProxy);
						FTransform PlaneTransform = WidgetTransform;
						PlaneTransform = HandleFinalShapeTransforms(ManipulatorComponent->CombineOffsetTransforms(Plane.Offsets), WidgetOverallSize, PlaneTransform, true);
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

					// ==========  CIRCLE  ==========
					for (FManipulatorSettingsMainDrawCircle Circle : ManipulatorComponent->GetAllShapesOfTypeWireCircle())
					{
						// Create Hit Proxy
						PDI->SetHitProxy(HitProxy);
						FTransform CircleTransform = WidgetTransform;
						CircleTransform = HandleFinalShapeTransforms(ManipulatorComponent->CombineOffsetTransforms(Circle.Offsets), WidgetOverallSize, CircleTransform);

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
			ToggleBoolPropertyValueFromManipulator(PropertyProxy->ManipulatorComponent);
			ResetDeSelectCounter();
		}
		else
		{
			if (Click.IsControlDown())
			{
				ToggleSelectedManipulator(PropertyProxy->ManipulatorComponent);
			}
			else if (Click.IsShiftDown())
			{
				AddNewSelectedManipulator(PropertyProxy->ManipulatorComponent);
			}
			else
			{
				ClearManipulatorSelection();
				AddNewSelectedManipulator(PropertyProxy->ManipulatorComponent);
			}
			AllowTrackSelectionUpdate = true;
			ResetDeSelectCounter();
		}
		return true;
	}
	// Clear Info when de-selecting a Hit proxy
	else if (HitProxy != nullptr && HitProxy->IsA(HActor::StaticGetType()) && Click.IsShiftDown())
	{
		ClearManipulatorSelection();
	}
	else if (DeSelectCounter > 0)
	{
		ReduceDeSelectCounter();
		return true;
	}
	FEdMode::HandleClick(InViewportClient, HitProxy, Click);

	return false;
}

FVector FManipulatorToolsEditorEdMode::GetWidgetLocation() const
{
	// Update the widget location so that it doesn't leave you with odd relative offset stuff.
	UManipulatorComponent* ManipulatorComponent = NewObject<UManipulatorComponent>();
	FTransform WidgetTransform = FTransform::Identity;
	if (SelectedManipulators.Num() > 0)
	{
		if (GetSelectedManipulatorComponent(SelectedManipulators.Last(), ManipulatorComponent, WidgetTransform))
		{
			FVector WorldLocation = Owner->PivotLocation;
			// Handle Enum property offsets
			WidgetTransform = GetManipulatorTransformWithOffsets(ManipulatorComponent);
			// Do some crazy magical shit to offset the widget location
			WorldLocation = WidgetTransform.GetLocation();
			return WorldLocation;
		}
	}
	FEdMode::GetWidgetLocation();
	return Owner->PivotLocation;
}

bool FManipulatorToolsEditorEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector & InDrag, FRotator & InRot, FVector & InScale)
{

	// The input delta is what tells the widget how much to adjust its value by based on user input. 
	UManipulatorComponent* ManipulatorComponent;
	FTransform WidgetTransform = FTransform::Identity;
	for (FManipulatorData* ManipulatorData : SelectedManipulators)
	{
		if (GetSelectedManipulatorComponent(ManipulatorData, ManipulatorComponent, WidgetTransform) && InViewportClient->GetCurrentWidgetAxis() != EAxisList::None)
		{
			// Get the object to edit properties is the only way I could correctly get something that talked nicely to the get property value by name. 
			UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFromManipulator(WidgetTransform, ManipulatorComponent);
			if (ObjectToEditProperties != nullptr && ManipulatorComponent != nullptr)
			{
				// Not sure what this does.. but i kept it.
				GEditor->NoteActorMovement();

				if (!ManipulatorData->PropertyName.IsEmpty())
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
						LocalTM = GetPropertyValueByName<FTransform>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex);
						break;
					case EManipulatorPropertyType::MT_VECTOR:
						LocalLocation = GetPropertyValueByName<FVector>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex);
						LocalTM = FTransform(LocalLocation);
						break;
					case EManipulatorPropertyType::MT_ENUM:
						EnumValue = GetPropertyValueByName<uint8>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex);
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
						SetPropertyValueByName<FTransform>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex, LocalTM, SetProperty);
						break;
					case EManipulatorPropertyType::MT_VECTOR:
						SetPropertyValueByName<FVector>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex, LocalTM.GetLocation(), SetProperty);
						break;
					case EManipulatorPropertyType::MT_ENUM:
						//Handle Enum Change
						EnumValue = HandleEnumPropertyInputDelta(ManipulatorComponent, LocalTM, EnumValue);
						SetPropertyValueByName<uint8>(ObjectToEditProperties, ManipulatorData->PropertyName, ManipulatorData->PropertyIndex, EnumValue, SetProperty);
						break;
					}

					SequencerKeyProperty(ObjectToEditProperties, SetProperty);

					FPropertyChangedEvent PropertyChangeEvent(SetProperty);
					ObjectToEditProperties->PostEditChangeProperty(PropertyChangeEvent);
					ResetDeSelectCounter();
					if (ManipulatorData == SelectedManipulators.Last())
					{
						return true;
					}
				}
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
	if (SelectedManipulators.Num() > 0)
	{
		if (GetSelectedManipulatorComponent(SelectedManipulators.Last(), ManipulatorComponent, WidgetTransform))
		{
			WidgetTransform = GetManipulatorTransformWithOffsets(ManipulatorComponent);
			FTransform DisplayWidgetToWorld;
			FTransform LocalTM;
			UObject* BestSelectedItem = GetObjectToDisplayWidgetsFromManipulator(DisplayWidgetToWorld, ManipulatorComponent);
			if (BestSelectedItem && ManipulatorComponent->Settings.Property.NameToEdit != TEXT(""))
			{
				switch (ManipulatorComponent->Settings.Property.Type)
				{
				case EManipulatorPropertyType::MT_TRANSFORM:
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
	}
	return false;
}

void FManipulatorToolsEditorEdMode::ActorSelectionChangeNotify()
{
}

bool FManipulatorToolsEditorEdMode::UsesToolkits() const
{
	return true;
}

void FManipulatorToolsEditorEdMode::Tick(FEditorViewportClient * ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);
}

bool FManipulatorToolsEditorEdMode::Select(AActor * InActor, bool bInSelected)
{
	return GetIsActorSelectionLocked();
}

/* ---------- Public Sequencer ----------*/

void FManipulatorToolsEditorEdMode::SetSequencer(TWeakPtr<ISequencer> InSequencer)
{
	WeakSequencer = InSequencer;
	AllowTrackSelectionUpdate = true;
	if (UsesToolkits())
	{
		// TODO: Reference ControlRigEditMode for this. Getting this setup may be a more correct way of doing auto key.
		//StaticCastSharedPtr<SControlRigEditModeTools>(Toolkit->GetInlineContent())->SetSequencer(InSequencer);
	}
}

void FManipulatorToolsEditorEdMode::OnSequencerTrackSelectionChanged(TArray<UMovieSceneTrack*> InTracks)
{
	if (WeakSequencer != nullptr)
	{
		TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();
		UMovieSceneSequence* SequenceScene = Sequencer->GetFocusedMovieSceneSequence();
		UMovieScene* Scene = SequenceScene->GetMovieScene();
		auto Bindings = Scene->GetBindings();

		bool ClearSelection = true;

		for (UMovieSceneTrack* Track : InTracks)
		{
			FString PropertyName = FString();
			FString ActorSequencerName = FString();

			UMovieScenePropertyTrack* PropertyTrack = Cast<UMovieScenePropertyTrack>(Track);
			if (PropertyTrack != nullptr)
			{
				PropertyName = PropertyTrack->GetPropertyPath();
				FGuid MatchingGuid;
				if (Scene->FindTrackBinding(*Track, MatchingGuid))
				{
					for (auto Binding : Bindings)
					{
						if (Binding.GetObjectGuid() == MatchingGuid)
						{
							if (ClearSelection)
							{
								ClearManipulatorSelection();
								ClearSelection = false;
							}
							ActorSequencerName = Binding.GetName();
							FindAndAddNewManipulatorSelection(PropertyName, ActorSequencerName);
							continue;
						}
					}
				}
			}
		}
	}
}

/* ---------- Private Sequencer ----------*/

void FManipulatorToolsEditorEdMode::ResetDeSelectCounter()
{
	if (bUseSafeDeSelect)
	{
		DeSelectCounter = 1;
	}
	else
	{
		DeSelectCounter = 0;
	}

}

void FManipulatorToolsEditorEdMode::ReduceDeSelectCounter()
{
	if (bUseSafeDeSelect)
	{
		DeSelectCounter = DeSelectCounter - 1;
	}
	else
	{
		DeSelectCounter = 0;
	}
}

void FManipulatorToolsEditorEdMode::SequencerKeyProperty(UObject* ObjectToKey, UProperty* propertyToUse)
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

void FManipulatorToolsEditorEdMode::SequencerUpdateTrackSelection()
{
	// Handle updating the selected track when selecting a manipulator.
	if (WeakSequencer != nullptr && AllowTrackSelectionUpdate)
	{
		AllowTrackSelectionUpdate = false;
		WeakSequencer.Pin()->EmptySelection();
		for (FManipulatorData* ManipulatorData : SelectedManipulators)
		{
			TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();
			UMovieSceneSequence* SequenceScene = Sequencer->GetFocusedMovieSceneSequence();
			UMovieScene* Scene = SequenceScene->GetMovieScene();
			auto Bindings = Scene->GetBindings();
			for (auto Binding : Bindings)
			{

				if (ManipulatorData->ActorSequencerName == Binding.GetName())
				{
					TArray<TSubclassOf<UMovieSceneTrack>> TrackClasses;
					TrackClasses.Add(UMovieSceneBoolTrack::StaticClass());
					TrackClasses.Add(UMovieSceneVectorTrack::StaticClass());
					TrackClasses.Add(UMovieSceneTransformTrack::StaticClass());
					TrackClasses.Add(UMovieSceneByteTrack::StaticClass());

					for (TSubclassOf<UMovieSceneTrack> TrackClass : TrackClasses)
					{
						UMovieSceneTrack* Track = Scene->FindTrack(TrackClass, Binding.GetObjectGuid(), FName(*ManipulatorData->PropertyName));
						if (Track != nullptr)
						{
							WeakSequencer.Pin()->SelectTrack(Track);
							continue;
						}
					}
				}
			}
		}
	}
}

/* ---------- Public Actor Selection ----------*/

void FManipulatorToolsEditorEdMode::UpdateIsActorSelectionLocked(bool bNewIsActorSelectionLocked)
{
	bIsActorSelectionLocked = bNewIsActorSelectionLocked;
}

bool FManipulatorToolsEditorEdMode::GetIsActorSelectionLocked() const
{
	return bIsActorSelectionLocked;
}

void FManipulatorToolsEditorEdMode::UpdateUseSafeDeSelect(bool bNewUseSafeDeSelect)
{
	bUseSafeDeSelect = bNewUseSafeDeSelect;
	ResetDeSelectCounter();
}

bool FManipulatorToolsEditorEdMode::GetUseSafeDeSelect() const
{
	return bUseSafeDeSelect;
}

/* ---------- Private Manipulator Components ----------*/

bool FManipulatorToolsEditorEdMode::GetSelectedManipulatorComponent(FManipulatorData* ManipulatorData, UManipulatorComponent*& OutComponent, FTransform& OutWidgetTransform) const
{
	// Finds the first actor then walks through the components to find the currently selected component based off of component name and property to edit. 
	// Outputs false if at any point any of the out information is null or fails.
	// TODO: Update for Multi-select
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
	for (AActor* SelectedActor : SelectedActors)
	{
		if (SelectedActor != nullptr)
		{
			TArray<UActorComponent*> Manipulators = SelectedActor->GetComponentsByClass(UManipulatorComponent::StaticClass());
			for (UActorComponent* ActorComponent : Manipulators)
			{
				UManipulatorComponent* ManipulatorComponent = Cast<UManipulatorComponent>(ActorComponent);
				if (ManipulatorComponent != nullptr)
				{
					if (ManipulatorComponent->GetManipulatorID() == ManipulatorData->ID)
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

FTransform FManipulatorToolsEditorEdMode::GetManipulatorTransformWithOffsets(UManipulatorComponent * ManipulatorComponent) const
{
	// Enum Offsets
	FVector EnumPropertyOffset = FVector(0, 0, 0);
	uint8 EnumValue = 0;

	// Visual Offset and Relative Offset
	FTransform RelativeTransform = FTransform::Identity;
	FTransform WidgetTransform = FTransform::Identity;
	UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFromManipulator(WidgetTransform, ManipulatorComponent);

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

UManipulatorComponent * FManipulatorToolsEditorEdMode::FindManipulatorComponentInActor(FString PropertyName, FString ActorName)
{
	return nullptr;
}

bool FManipulatorToolsEditorEdMode::GetBoolPropertyValueFromManipulator(UManipulatorComponent* ManipulatorComponent)
{
	// Used to tell bools when to change color.
	bool Output = false;
	if (ManipulatorComponent->Settings.Property.Type == EManipulatorPropertyType::MT_BOOL)
	{
		FTransform WidgetTransform;
		UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFromManipulator(WidgetTransform, ManipulatorComponent);
		if (ObjectToEditProperties != nullptr)
		{
			Output = GetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);
		}
	}
	return Output;
}

void FManipulatorToolsEditorEdMode::ToggleBoolPropertyValueFromManipulator(UManipulatorComponent* ManipulatorComponent)
{
	// Toggles a bool on and off when clicked. 
	if (ManipulatorComponent->Settings.Property.Type == EManipulatorPropertyType::MT_BOOL)
	{
		bool CurrentBool = false;
		FTransform WidgetTransform;
		UObject* ObjectToEditProperties = GetObjectToDisplayWidgetsFromManipulator(WidgetTransform, ManipulatorComponent);
		if (ObjectToEditProperties != nullptr)
		{
			CurrentBool = GetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index);

			// Set Bool Value
			ObjectToEditProperties->PreEditChange(NULL);
			UProperty* SetProperty = NULL;
			SetPropertyValueByName<bool>(ObjectToEditProperties, ManipulatorComponent->Settings.Property.NameToEdit, ManipulatorComponent->Settings.Property.Index, !CurrentBool, SetProperty);

			SequencerKeyProperty(ObjectToEditProperties, SetProperty);

			FPropertyChangedEvent PropertyChangeEvent(SetProperty);
			ObjectToEditProperties->PostEditChangeProperty(PropertyChangeEvent);
		}
	}
}

UObject * FManipulatorToolsEditorEdMode::GetObjectToDisplayWidgetsFromManipulator(FTransform & OutLocalToWorld, UManipulatorComponent* ManipulatorComponent) const
{
	// Determine what is selected, preferring a component over an actor
	USceneComponent* SelectedComponent = ManipulatorComponent->GetOwner()->GetRootComponent();
	UObject* BestSelectedItem = ManipulatorComponent->GetOwner();
	OutLocalToWorld = SelectedComponent->GetComponentToWorld();
	return BestSelectedItem; 
}

uint8 FManipulatorToolsEditorEdMode::HandleEnumPropertyInputDelta(UManipulatorComponent* ManipulatorComponent, FTransform LocalTM, uint8 EnumInput)
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

/* ---------- Private Transform Manipulation ----------*/

FTransform FManipulatorToolsEditorEdMode::HandleFinalShapeTransforms(FTransform ShapeTransform, FTransform OverallScale, FTransform WidgetTransform, bool RotateScale)
{
	if (RotateScale)
	{
		WidgetTransform.SetScale3D(ShapeTransform.GetRotation().Inverse().RotateVector(WidgetTransform.GetScale3D()));
	}
	return ShapeTransform * OverallScale * WidgetTransform;
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

void FManipulatorToolsEditorEdMode::AddNewSelectedManipulator(UManipulatorComponent* ManipulatorComponent)
{
	if (ManipulatorComponent != nullptr && ManipulatorComponent->GetOwner() != nullptr)
	{
		if (!IsManipulatorSelected(ManipulatorComponent))
		{
			FManipulatorData* NewData = new FManipulatorData();
			NewData->ID = ManipulatorComponent->GetManipulatorID();
			NewData->ActorName = ManipulatorComponent->GetName();
			NewData->ActorSequencerName = ManipulatorComponent->GetOwner()->GetActorLabel();
			NewData->ComponentName = ManipulatorComponent->GetName();
			NewData->PropertyName = ManipulatorComponent->Settings.Property.NameToEdit;
			NewData->PropertyIndex = ManipulatorComponent->Settings.Property.Index;
			NewData->PropertyType = ManipulatorComponent->Settings.Property.Type;
			NewData->ActorUniqueID = ManipulatorComponent->GetOwner()->GetUniqueID();
			if (NewData->PropertyType != EManipulatorPropertyType::MT_BOOL)
			{
				NewSelectedManipulators.Add(NewData);
			}
		}
	}
}

void FManipulatorToolsEditorEdMode::ToggleSelectedManipulator(UManipulatorComponent * ManipulatorComponent)
{
	if (ManipulatorComponent != nullptr)
	{
		if (IsManipulatorSelected(ManipulatorComponent))
		{
			RemoveSelectedManipulator(ManipulatorComponent);
		}
		else
		{
			AddNewSelectedManipulator(ManipulatorComponent);
		}
	}

}

void FManipulatorToolsEditorEdMode::RemoveSelectedManipulator(UManipulatorComponent * ManipulatorComponent)
{
	if (ManipulatorComponent != nullptr && IsManipulatorSelected(ManipulatorComponent))
	{
		for (int32 i = 0; i < NewSelectedManipulators.Num(); i++)
		{
			if (NewSelectedManipulators.IsValidIndex(i))
			{
				if (NewSelectedManipulators[i]->ID == ManipulatorComponent->GetManipulatorID())
				{
					NewSelectedManipulators.RemoveAt(i);
					return;
				}
			}
		}
	}
}

bool FManipulatorToolsEditorEdMode::IsManipulatorSelected(UManipulatorComponent * ManipulatorComponent)
{
	// Check if Manipulator ID already exists
	if (ManipulatorComponent != nullptr)
	{
		for (FManipulatorData* SelectedManipulator : NewSelectedManipulators)
		{
			if (SelectedManipulator->ID == ManipulatorComponent->GetManipulatorID())
			{
				return true;
			}
		}
	}
	return false;
}

void FManipulatorToolsEditorEdMode::FindAndAddNewManipulatorSelection(FString PropertyName, FString ActorSequencerName)
{
	// Find the correct manipulator so we can correctly make a new selection data for the manipulator. 
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
	for (AActor* SelectedActor : SelectedActors)
	{
		if (SelectedActor != nullptr && Owner->GetSelectedComponents()->GetTop<USceneComponent>() == nullptr)
		{
			if (SelectedActor->GetActorLabel() == ActorSequencerName)
			{
				TArray<UActorComponent*> Manipulators = SelectedActor->GetComponentsByClass(UManipulatorComponent::StaticClass());
				for (UActorComponent* ActorComponent : Manipulators)
				{
					UManipulatorComponent* ManipulatorComponent = Cast<UManipulatorComponent>(ActorComponent);
					if (ManipulatorComponent != nullptr)
					{
						if (ManipulatorComponent->Settings.Property.NameToEdit == PropertyName)
						{
							AddNewSelectedManipulator(ManipulatorComponent);
							return;
						}
					}
				}
			}
		}
	}
}

FManipulatorData * FManipulatorToolsEditorEdMode::GetManipulatorData(UManipulatorComponent * ManipulatorComponent)
{
	FManipulatorData* ManipulatorData = new FManipulatorData();
	if (ManipulatorComponent != nullptr)
	{
		for (FManipulatorData* SelectedManipulator : NewSelectedManipulators)
		{
			if (SelectedManipulator->ID == ManipulatorComponent->GetManipulatorID())
			{
				return SelectedManipulator;
			}
		}
	}
	return ManipulatorData;
}

void FManipulatorToolsEditorEdMode::ClearManipulatorSelection()
{
	NewSelectedManipulators.Empty();
	bEditedPropertyIsTransform = false;
}






