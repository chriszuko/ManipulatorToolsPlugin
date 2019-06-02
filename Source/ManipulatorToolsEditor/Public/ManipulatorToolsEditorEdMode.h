// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "ManipulatorComponent.h"

/** Hit proxy used for editable properties */
struct HManipulatorProxy : public HHitProxy
{
	DECLARE_HIT_PROXY();

	/** Component this hit proxy will talk to. */
	UManipulatorComponent* ManipulatorComponent;

	/** This property is a transform */
	bool	bPropertyIsTransform;

	// Constructor
	HManipulatorProxy(UManipulatorComponent* ManipulatorComponent) : HHitProxy(HPP_Foreground), ManipulatorComponent(ManipulatorComponent)
	{
	}

	/** Show cursor as cross when over this handle */
	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::Crosshairs;
	}
};

IMPLEMENT_HIT_PROXY(HManipulatorProxy, HHitProxy);

class FManipulatorToolsEditorEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_ManipulatorToolsEditorEdModeId;
public:
	FManipulatorToolsEditorEdMode();
	virtual ~FManipulatorToolsEditorEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	//virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click) override;
	virtual FVector GetWidgetLocation() const override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool AllowWidgetMove() override;
	virtual bool GetSelectedManipulatorComponent( UManipulatorComponent*& OutComponent, FTransform& OutWidgetTransform) const;
	virtual bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData);
	virtual void ActorSelectionChangeNotify() override;
	uint8 CalculateEnumPropertyInputDelta(UManipulatorComponent* ManipulatorComponent, FTransform LocalTM, uint8 EnumInput);
	bool GetBoolPropertyValue(UManipulatorComponent* ManipulatorComponent);
	void ToggleBoolPropertyValue(UManipulatorComponent* ManipulatorComponent);
	FTransform FlipTransformOnX(FTransform Transform, bool FlipXVector, bool FlipYRotation, bool FlipXScale) const;
	FTransform GetManipulatorTransformWithOffsets(UManipulatorComponent* ManipulatorComponent) const;
	UObject* GetObjectToDisplayWidgetsFor(FTransform& OutLocalToWorld, UManipulatorComponent* ManipulatorComponent) const;

	bool UsesToolkits() const override;
	// End of FEdMode interface

	virtual bool Select(AActor* InActor, bool bInSelected) override;

	void SetSequencer(TWeakPtr<ISequencer> InSequencer);
	void OnSequencerTrackSelectionChanged(TArray<UMovieSceneTrack*> InTracks);

	// EditedPropertyName Already Exists in EdMode
	FString EditedManipulatorPropertyName = "";
	FString EditedComponentName = "";
	FString EditedActorName = "";

	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	void UpdatedIsSelectionLocked(bool bNewIsSelectionLocked);
	bool GetIsSelectionLocked() const;
private:
	// 
	bool bIsSelectionLocked = false;

	/** Weak pointer to the last sequencer that was opened */
	TWeakPtr<ISequencer> WeakSequencer;

	void HandleSequencerTrackSelection(const EManipulatorPropertyType& PropertyType, const FName& PropertyName, UManipulatorComponent* ManipulatorComponent);

	void KeyProperty(UObject* ObjectToKey, UProperty* propertyToUse);

	FTransform HandleFinalShapeTransforms(FTransform ShapeTransform, FTransform OverallScale, FTransform WidgetTransform, bool RotateScale = false);

	TArray<HManipulatorProxy*> HitProxies;
	void SelectManipulators(HManipulatorProxy* PropertyProxy);
	void ClearManipulatorSelection();
};
