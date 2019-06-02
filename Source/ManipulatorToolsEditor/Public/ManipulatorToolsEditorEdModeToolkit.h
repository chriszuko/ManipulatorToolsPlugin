// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

class FManipulatorToolsEditorEdModeToolkit : public FModeToolkit
{
public:

	FManipulatorToolsEditorEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

	//bool bIsSelectionLocked = false;
	void OnIsSelectionLockedChanged(ECheckBoxState NewCheckedState);
	ECheckBoxState IsSelectionLocked() const;

private:

	TSharedPtr<SWidget> ToolkitWidget;
};
