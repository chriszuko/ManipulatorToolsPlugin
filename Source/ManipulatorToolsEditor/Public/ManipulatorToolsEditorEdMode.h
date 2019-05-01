// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "Sequencer/Public/ISequencer.h"
#include "Sequencer/Public/ISequencerModule.h"
#include "ManipulatorComponent.h"


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

	void SetSequencer(TWeakPtr<ISequencer> InSequencer);

	// EditedPropertyName Already Exists in EdMode
	FString EditedManipulatorPropertyName = "";
	FString EditedComponentName = "";
	FString EditedActorName = "";
	void HandleSelectedColorMultiplier(float DeltaTime);

	float SelectedColorMuliplier = 1;
	bool SelectedColorMultiplierDirection = false;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;


private:

	/** Weak pointer to the last sequencer that was opened */
	TWeakPtr<ISequencer> WeakSequencer;

	void HandleSequencerTrackSelection(const EManipulatorPropertyType& propertyType, const FName& propertyName, UManipulatorComponent* ManipulatorComponent);

	void KeyProperty(UObject* ObjectToKey, UProperty* propertyToUse);
};
