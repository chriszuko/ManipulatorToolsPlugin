// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Sequencer/Public/ISequencer.h"
#include "Sequencer/Public/ISequencerModule.h"

class FManipulatorToolsEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TWeakPtr<ISequencer> GetSequencer()	{ return WeakSequencer;	}

private:
	/** Handle for tracking ISequencer creation */
	FDelegateHandle SequencerCreatedHandle;

	/** Weak pointer to the last sequencer that was opened */
	TWeakPtr<ISequencer> WeakSequencer;

	void HandleSequencerCreated(TSharedRef<ISequencer> InSequencer);
};
