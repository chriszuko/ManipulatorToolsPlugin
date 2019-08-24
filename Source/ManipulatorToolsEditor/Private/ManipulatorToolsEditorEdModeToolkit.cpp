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
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(this, &FManipulatorToolsEditorEdModeToolkit::OnIsActorSelectionLockedChanged)
				.IsChecked(this, &FManipulatorToolsEditorEdModeToolkit::IsActorSelectionLocked)
				.ToolTipText(LOCTEXT("IsActorSelectionLockedToolTip", "Allows you to not accidentally change your selection as much."))
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("IsActorSelectionLockedCheckbox", "Lock Actor Selection"))
				]
			]
			+ SVerticalBox::Slot()
			.Padding(5)
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(this, &FManipulatorToolsEditorEdModeToolkit::OnUseSafeDeSelectChanged)
				.IsChecked(this, &FManipulatorToolsEditorEdModeToolkit::UseSafeDeSelect)
				.ToolTipText(LOCTEXT("UseSafeDeSelectToolTip", "Won't allow you to deselect from a manipulator to another actor until you do it twice. This can feel odd if you don't realize what is happening."))
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UseSafeDeSelectCheckbox", "Use Safe DeSelect"))
				]
			]
		];
	FModeToolkit::Init(InitToolkitHost);
}

FManipulatorToolsEditorEdMode * FManipulatorToolsEditorEdModeToolkit::GetManipulatorToolsEdMode() const
{
	return static_cast<FManipulatorToolsEditorEdMode*>(GLevelEditorModeTools().GetActiveMode(FManipulatorToolsEditorEdMode::EM_ManipulatorToolsEditorEdModeId));
}

ECheckBoxState FManipulatorToolsEditorEdModeToolkit::IsActorSelectionLocked() const
{
	if (GetManipulatorToolsEdMode())
	{
		return GetManipulatorToolsEdMode()->GetIsActorSelectionLocked() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void FManipulatorToolsEditorEdModeToolkit::OnIsActorSelectionLockedChanged(ECheckBoxState NewCheckedState)
{
	if (GetEditorMode())
	{
		if (NewCheckedState == ECheckBoxState::Checked)
		{
			GetManipulatorToolsEdMode()->UpdateIsActorSelectionLocked(true);
		}
		else
		{
			GetManipulatorToolsEdMode()->UpdateIsActorSelectionLocked(false);
		}
	}
}

ECheckBoxState FManipulatorToolsEditorEdModeToolkit::UseSafeDeSelect() const
{
	if (GetManipulatorToolsEdMode())
	{
		return GetManipulatorToolsEdMode()->GetUseSafeDeSelect() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void FManipulatorToolsEditorEdModeToolkit::OnUseSafeDeSelectChanged(ECheckBoxState NewCheckedState)
{
	if (GetEditorMode())
	{
		if (NewCheckedState == ECheckBoxState::Checked)
		{
			GetManipulatorToolsEdMode()->UpdateUseSafeDeSelect(true);
		}
		else
		{
			GetManipulatorToolsEdMode()->UpdateUseSafeDeSelect(false);
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
