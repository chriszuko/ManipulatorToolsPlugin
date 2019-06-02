// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorToolsEditor.h"
#include "ManipulatorToolsEditorEdMode.h"
#include "ManipulatorToolsEditorEdModeStyle.h"
#include "PropertyEditorModule.h"
#include "EditorModeManager.h"
#include "MovieSceneSequence.h"

#define LOCTEXT_NAMESPACE "FManipulatorToolsEditorModule"

void FManipulatorToolsEditorModule::StartupModule()
{
	FEditorModeRegistry::Get().RegisterMode<FManipulatorToolsEditorEdMode>(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId, LOCTEXT("ManipulatorToolsEditorEdModeName", "ManipulatorToolsEditorEdMode"), FSlateIcon(FManipulatorToolsEditorEdModeStyle::Get().GetStyleSetName(), "ManipulatorToolsEditorEdMode", "ManipulatorToolsEditorEdMode.Small"), true);

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

	//InSequencer->GetSelectionChangedTracks().AddLambda([LocalSequencer](TArray<UMovieSceneTrack*> InTracks)
	auto HandleOnTrackSelectionChanged = [this, LocalSequencer](TArray<UMovieSceneTrack*> InTracks)
	{
		if (LocalSequencer.IsValid())
		{
			if (FManipulatorToolsEditorEdMode* ManipulatorToolsEdMode = static_cast<FManipulatorToolsEditorEdMode*>(GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId)))
			{
				ManipulatorToolsEdMode->OnSequencerTrackSelectionChanged(InTracks);
			}
		}
	};
	
	InSequencer->OnActivateSequence().AddLambda(HandleActivateSequence);
	InSequencer->GetSelectionChangedTracks().AddLambda(HandleOnTrackSelectionChanged);

	// Call into activation callback to handle initial activation
	FMovieSceneSequenceID SequenceID = MovieSceneSequenceID::Root;
	TArray<UMovieSceneTrack*> InTracks;
	HandleActivateSequence(SequenceID);
	HandleOnTrackSelectionChanged(InTracks);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FManipulatorToolsEditorModule, ManipulatorToolsEditor)