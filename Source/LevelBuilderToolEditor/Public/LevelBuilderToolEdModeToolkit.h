// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

class FLevelBuilderToolEdModeToolkit : public FModeToolkit
{
public:

	FLevelBuilderToolEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }
	void CalculateLightMap(float Quality,int min, int max);
private:
	FReply SpawnActorOnClicked();
	FReply SelectedActorOnClicked();
	void OnNewHostTextCommited(const FText& InText, ETextCommit::Type InCommitType);
	FReply SpawnActor();
	TSharedPtr<SWidget> ToolkitWidget;
	TSharedPtr<SEditableTextBox>InputTag;
	FText InputText;
	FString InputString;
	bool ExistingTag;
	FReply CheckErrorMergerInstanceActor();
	FReply PreviewLightmapping();
	FReply MediumLightmapping();
	FReply HighLightmapping();
	FReply EpicLightmapping();
	void AutoLightmapSize();



};
