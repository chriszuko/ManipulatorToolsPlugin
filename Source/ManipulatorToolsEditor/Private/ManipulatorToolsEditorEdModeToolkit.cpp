// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ManipulatorToolsEditorEdModeToolkit.h"
#include "ManipulatorToolsEditorEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FManipulatorToolsEditorEdModeToolkit"

FManipulatorToolsEditorEdModeToolkit::FManipulatorToolsEditorEdModeToolkit()
{
}

void FManipulatorToolsEditorEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	SAssignNew(ToolkitWidget, SBorder)
		.HAlign(HAlign_Center)
		.Padding(5)
		[
			SNew(SVerticalBox) + SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(5)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("HelperLabel", "Click manipulators to select them. Shift click the actor to re-select the actor."))
			]


			+ SVerticalBox::Slot()
			.Padding(5)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(this, &FManipulatorToolsEditorEdModeToolkit::OnIsActorSelectionLockedChanged)
				.IsChecked(this, &FManipulatorToolsEditorEdModeToolkit::IsActorSelectionLocked)
				.ToolTipText(LOCTEXT("EnableDisableButtonToolTip", "Allows you to not accidentally change your selection as much."))
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IsActorSelectionLockedCheckbox", "Lock Actor Selection"))
				]
			]
		];
	FModeToolkit::Init(InitToolkitHost);
}

ECheckBoxState FManipulatorToolsEditorEdModeToolkit::IsActorSelectionLocked() const
{
	if (FManipulatorToolsEditorEdMode* ManipulatorToolsEdMode = static_cast<FManipulatorToolsEditorEdMode*>(GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId)))
	{
		return ManipulatorToolsEdMode->GetIsActorSelectionLocked() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void FManipulatorToolsEditorEdModeToolkit::OnIsActorSelectionLockedChanged(ECheckBoxState NewCheckedState)
{
	if (FManipulatorToolsEditorEdMode* ManipulatorToolsEdMode = static_cast<FManipulatorToolsEditorEdMode*>(GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId)))
	{
		if (NewCheckedState == ECheckBoxState::Checked)
		{
			ManipulatorToolsEdMode->UpdateIsActorSelectionLocked(true);
		}
		else
		{
			ManipulatorToolsEdMode->UpdateIsActorSelectionLocked(false);
		}
	}
}

FName FManipulatorToolsEditorEdModeToolkit::GetToolkitFName() const
{
	return FName("ManipulatorToolsEditorEdMode");
}

FText FManipulatorToolsEditorEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("ManipulatorToolsEditorEdModeToolkit", "DisplayName", "ManipulatorToolsEditorEdMode Tool");
}

class FEdMode* FManipulatorToolsEditorEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId);
}

#undef LOCTEXT_NAMESPACE
