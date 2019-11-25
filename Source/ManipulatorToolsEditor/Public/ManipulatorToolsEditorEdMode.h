// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "ManipulatorComponent.h"

struct FManipulatorData
{
	FString ID = FString();
	FString PropertyName = FString();
	int PropertyIndex = INDEX_NONE;
	FString ComponentName = FString();
	FString ActorName = FString();
	FString ActorSequencerName = FString();
	EManipulatorPropertyType PropertyType = EManipulatorPropertyType::MT_BOOL;
	uint32 ActorUniqueID;
};

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

// A bunch of functions that EdMode Uses to get properties by name that I don't have access to because 
// EdMode is dumb and doesn't put it in the header so I followed their example and copied it.
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

class FManipulatorToolsEditorEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_ManipulatorToolsEditorEdModeId;
public:
	FManipulatorToolsEditorEdMode();
	virtual ~FManipulatorToolsEditorEdMode();

	/** FEdMode interface */
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click) override;
	virtual FVector GetWidgetLocation() const override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool AllowWidgetMove() override;
	virtual bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData);
	virtual void ActorSelectionChangeNotify() override;
	bool UsesToolkits() const override;
	virtual bool Select(AActor* InActor, bool bInSelected) override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	/** End of FEdMode interface */

	/** Sequencer */
	void SetSequencer(TWeakPtr<ISequencer> InSequencer);
	void OnSequencerTrackSelectionChanged(TArray<UMovieSceneTrack*> InTracks);

	/** Selection Locking for Actors */
	void UpdateIsActorSelectionLocked(bool bNewIsActorSelectionLocked);
	bool GetIsActorSelectionLocked() const;

	void UpdateUseSafeDeSelect(bool bNewUseSafeDeSelect);
	bool GetUseSafeDeSelect() const;

	/** EditedPropertyName Already Exists in EdMode */
	FString EditedManipulatorPropertyName = "";
	FString EditedComponentName = "";
	FString EditedActorName = "";

private:
	/** Data */
	bool bIsActorSelectionLocked = false;
	bool bUseSafeDeSelect = false;
	TArray<FManipulatorData*> SelectedManipulators;
	TArray<FManipulatorData*> NewSelectedManipulators;
	//TArray<FString> SelectedManipulators;

	/** ManipulatorComponents */
	virtual bool GetSelectedManipulatorComponent(FManipulatorData* ManipulatorData, UManipulatorComponent*& OutComponent) const;
	FTransform GetManipulatorTransformWithOffsets(UManipulatorComponent* ManipulatorComponent) const;
	FTransform GetManipulatorTransformWithOffsets(UManipulatorComponent* ManipulatorComponent, FTransform& WidgetTransformNoPropertyOffset) const;
	UManipulatorComponent* FindManipulatorComponentInActor(FString PropertyName, FString ActorName);
	bool GetBoolPropertyValueFromManipulator(UManipulatorComponent* ManipulatorComponent);
	void ToggleBoolPropertyValueFromManipulator(UManipulatorComponent* ManipulatorComponent);
	UObject* GetObjectToDisplayWidgetsFromManipulator(UManipulatorComponent* ManipulatorComponent) const;
	uint8 HandleEnumPropertyInputDelta(UManipulatorComponent* ManipulatorComponent, FTransform LocalTM, uint8 EnumInput);

	/** Combines Transforms of the shapes together with an option to rotate the scale vector.*/
	FTransform HandleFinalShapeTransforms(FTransform ShapeTransform, FTransform OverallScale, FTransform WidgetTransform, bool RotateScale = false);
	FTransform FlipTransformOnX(FTransform Transform, bool FlipXVector, bool FlipYRotation, bool FlipXScale) const;

	/** Proxies */
	TArray<HManipulatorProxy*> HitProxies;
	void AddNewSelectedManipulator(UManipulatorComponent* ManipulatorComponent);
	void ToggleSelectedManipulator(UManipulatorComponent* ManipulatorComponent);
	void RemoveSelectedManipulator(UManipulatorComponent* ManipulatorComponent);
	bool IsManipulatorSelected(UManipulatorComponent* ManipulatorComponent);
	void FindAndAddNewManipulatorSelection(FString PropertyName, FString ActorSequencerName);

	FManipulatorData* GetManipulatorData(UManipulatorComponent* ManipulatorComponent);
	void ClearManipulatorSelection();

	/** Weak pointer to the last sequencer that was opened */
	TWeakPtr<ISequencer> WeakSequencer;
	void SequencerUpdateTrackSelection();
	bool AllowTrackSelectionUpdate = false;
	int32 DeSelectCounter = 0;
	void ResetDeSelectCounter();
	void ReduceDeSelectCounter();
	void SequencerKeyProperty(UObject* ObjectToKey, UProperty* propertyToUse);
};
