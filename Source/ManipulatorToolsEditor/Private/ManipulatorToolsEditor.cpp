// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorToolsEditor.h"
#include "ManipulatorToolsEditorEdMode.h"
#include "ManipulatorToolsEditorEdModeStyle.h"
#include "ManipulatorComponentCustomization.h"
#include "PropertyEditorModule.h"
#include "EditorModeManager.h"
#include "MovieSceneSequence.h"

#define LOCTEXT_NAMESPACE "FManipulatorToolsEditorModule"

void FManipulatorToolsEditorModule::StartupModule()
{
	FEditorModeRegistry::Get().RegisterMode<FManipulatorToolsEditorEdMode>(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId, LOCTEXT("ManipulatorToolsEditorEdModeName", "ManipulatorToolsEditorEdMode"), FSlateIcon(FManipulatorToolsEditorEdModeStyle::Get().GetStyleSetName(), "ManipulatorToolsEditorEdMode", "ManipulatorToolsEditorEdMode.Small"), true);
	
	// Stub in for figuring out detail customizations for the struct
	//FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	//PropertyModule.RegisterCustomPropertyTypeLayout("ManipulatorSettings", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FManipulatorSettingsMainCustomization::MakeInstance));'

	// Register for when the sequencer is opened in the editor to grab the reference.
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(FOnSequencerCreated::FDelegate::CreateRaw(this, &FManipulatorToolsEditorModule::HandleSequencerCreated));
}

void FManipulatorToolsEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId);

	ISequencerModule* SequencerModule = FModuleManager::GetModulePtr<ISequencerModule>("Sequencer");
	if (SequencerModule)
	{
		SequencerModule->UnregisterOnSequencerCreated(SequencerCreatedHandle);
	}
}

void FManipulatorToolsEditorModule::HandleSequencerCreated(TSharedRef<ISequencer> InSequencer)
{
	WeakSequencer = InSequencer;

	// TODO Nolan: Hand off the sequencer to the python bridge.
	// Python bridge should allow python to call NotifyMovieSceneDataChanged which should refresh the sequencer display.

	TWeakPtr<ISequencer> LocalSequencer = InSequencer;

	// When a sequence is activated.
	auto HandleActivateSequence = [this, LocalSequencer](FMovieSceneSequenceIDRef Ref)
	{
		if (LocalSequencer.IsValid())
		{
			WeakSequencer = LocalSequencer;

			if (FManipulatorToolsEditorEdMode* ManipulatorToolsEdMode = static_cast<FManipulatorToolsEditorEdMode*>(GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId)))
			{
				ManipulatorToolsEdMode->SetSequencer(WeakSequencer);
			}
		}
	};

	InSequencer->OnActivateSequence().AddLambda(HandleActivateSequence);

	// Call into activation callback to handle initial activation
	FMovieSceneSequenceID SequenceID = MovieSceneSequenceID::Root;
	HandleActivateSequence(SequenceID);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FManipulatorToolsEditorModule, ManipulatorToolsEditor)