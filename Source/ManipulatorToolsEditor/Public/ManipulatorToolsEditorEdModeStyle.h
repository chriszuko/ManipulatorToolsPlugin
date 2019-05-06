// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"


#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

class FManipulatorToolsEditorEdModeStyle
	: public FSlateStyleSet
{
public:
	FManipulatorToolsEditorEdModeStyle()
		: FSlateStyleSet("ManipulatorToolsEditorEdModeStyle")
	{
		const FVector2D Icon10x10(10.0f, 10.0f);
		const FVector2D Icon16x16(16.0f, 16.0f);
		const FVector2D Icon20x20(20.0f, 20.0f);
		const FVector2D Icon24x24(24.0f, 24.0f);
		const FVector2D Icon32x32(32.0f, 32.0f);
		const FVector2D Icon40x40(40.0f, 40.0f);
		SetContentRoot(IPluginManager::Get().FindPlugin("ManipulatorTools")->GetBaseDir() / TEXT("Content/HardCoded/Icons"));

		const FSlateColor DefaultForeground(FLinearColor(0.72f, 0.72f, 0.72f, 1.f));

		Set("ManipulatorToolsEditorEdMode", new IMAGE_BRUSH("IconEditorMode01_40x", Icon40x40));
		Set("ManipulatorToolsEditorEdMode.Small", new IMAGE_BRUSH("IconEditorMode01_40x", Icon20x20));

		Set("ClassIcon.ManipulatorComponent", new IMAGE_BRUSH("IconEditorMode01_40x", Icon16x16));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}

	static FManipulatorToolsEditorEdModeStyle& Get()
	{
		static FManipulatorToolsEditorEdModeStyle Inst;
		return Inst;
	}
	
	~FManipulatorToolsEditorEdModeStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
