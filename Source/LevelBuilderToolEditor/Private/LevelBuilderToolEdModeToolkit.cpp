// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelBuilderToolEdModeToolkit.h"
#include "LevelBuilderToolEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorModeManager.h"
#include "MergeInstance.h"
#include "EngineUtils.h"
#include "LevelEditorActions.h"
#include "Misc/ScopedSlowTask.h"
#include "Components/InstancedStaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "FLevelBuilderToolEdModeToolkit"

FLevelBuilderToolEdModeToolkit::FLevelBuilderToolEdModeToolkit()
{
}

FReply FLevelBuilderToolEdModeToolkit::SpawnActor()
{
	ExistingTag = false;
	//检查关卡中是否具有相同Tag
	UWorld* World = GEditor->GetEditorWorldContext().World();
	//For every actor in this level...
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* StaticMeshActor = *It;
		if (StaticMeshActor->ActorHasTag(FName(InputString)))
		{
			ExistingTag = true;
		}

	}
	//判断场景中是否具有改Tag
	if (!ExistingTag)
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();

		// Let editor know that we're about to do something that we want to undo/redo
		GEditor->BeginTransaction(LOCTEXT("MoveActorsTransactionName", "MoveActors"));
		FVector  ObjLocation;
		// For each selected actor
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			if (AActor* LevelActor = Cast<AActor>(*Iter))
			{
				// Register actor in opened transaction (undo/redo)
				LevelActor->Modify();
				// 设置Tag
				FName Tag = FName(InputString);
				LevelActor->Tags.Empty();
				LevelActor->Tags.Add(FName(Tag));
				ObjLocation = LevelActor->GetTransform().GetLocation() + FVector(200.0f, 200.0f, 200.0f);
			}
		}
		FActorSpawnParameters SpawnInfo;
		{
			SpawnInfo.Name = FName(InputString);
		}
		FRotator const ObjRotation = FRotator::ZeroRotator;
		AMergeInstance* TempActor = (AMergeInstance*)GEditor->GetEditorWorldContext().World()->SpawnActor(AMergeInstance::StaticClass(), &ObjLocation, &ObjRotation, SpawnInfo);
		TempActor->SetActorLabel(InputString);
		TempActor->ActorTags = FName(InputString);
		TempActor->bEditMode = false;
		TempActor->PostEditChange();
	}
	else
	{
		// 警告弹出错误提示
		FText DialogText = FText::Format(
			LOCTEXT("ExistingTagDialogText", "{0} actor existing in the level!"),
			FText::FromString(InputString)
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
	}
	// 执行完毕关闭Transaction
	GEditor->EndTransaction();
	return FReply::Handled();
}

// FReply FLevelBuilderToolEdModeToolkit::AutomaticLightmapping()
// {
// 	for (TObjectIterator<AStaticMeshActor> Itr; Itr; ++Itr)
// 	{
// 		AStaticMeshActor* component = *Itr;
// 		float BoundSize = component->GetStaticMeshComponent()->CalcBounds(component->GetStaticMeshComponent()->GetComponentTransform()).BoxExtent.Size();
// 		float scaleCalculation = 512.0 / (BoundSize * 0.05);
// 		int32 NewLightMapSize = (int32)scaleCalculation;
// 		component->GetStaticMeshComponent()->OverriddenLightMapRes = FMath::Clamp(NewLightMapSize, 16, 128);
// 		component->GetStaticMeshComponent()->bOverrideLightMapRes=true;
// 		//FAutoLightmapAdjusterCommands::Register();
// 		
// 	}
// 	return FReply::Handled();
// }

FReply FLevelBuilderToolEdModeToolkit::CheckErrorMergerInstanceActor()
{
	bool IsNoError = false;
	//检查关卡中错误的MergeInstance
	UWorld* World = GEditor->GetEditorWorldContext().World();
	//For every actor in this level...
	for (TActorIterator<AMergeInstance> It(World); It; ++It)
	{
		AMergeInstance* MergeInstance = *It;
		if (MergeInstance->bEditMode)
		{
			MergeInstance->bEditMode = false;
			if (MergeInstance->ErrorStaticMeshActors.Num()>0)
			{
				GEditor->SelectActor(MergeInstance, true, true, false, true);
				break;
			}
		}
		else
		{
			if (MergeInstance->ErrorStaticMeshActors.Num() > 0)
			{
				GEditor->SelectActor(MergeInstance, true, true, false, true);
				break;
			}
			else
			{
				IsNoError = true;
			}
		}
	}
	if (IsNoError)
	{
		check(GEngine != nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Congratulations, no errors!"));
	}
	return FReply::Handled();
}

FReply FLevelBuilderToolEdModeToolkit::PreviewLightmapping()
{
	CalculateLightMap(0.125, 8, 256);
	return FReply::Handled();
}

FReply FLevelBuilderToolEdModeToolkit::MediumLightmapping()
{
	CalculateLightMap(0.25, 16, 512);
	return FReply::Handled();
}

FReply FLevelBuilderToolEdModeToolkit::HighLightmapping()
{
	CalculateLightMap(0.5, 32, 1024);
	return FReply::Handled();
}


FReply FLevelBuilderToolEdModeToolkit::EpicLightmapping()
{
	CalculateLightMap(1.25, 64, 2048);
	return FReply::Handled();
}


void FLevelBuilderToolEdModeToolkit::AutoLightmapSize()
{

}

FReply FLevelBuilderToolEdModeToolkit::SpawnActorOnClicked()
{
	SpawnActor();
	return FReply::Handled();
}

FReply FLevelBuilderToolEdModeToolkit::SelectedActorOnClicked()
{
	//先清除选择
	GEditor->SelectNone(true, true, true);
	//检查关卡中是否具有相同Tag并选择
	UWorld* World = GEditor->GetEditorWorldContext().World();
	//For every actor in this level...
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* StaticMeshActor = *It;
		if (StaticMeshActor->ActorHasTag(FName(InputString)))
		{
			GEditor->SelectActor(StaticMeshActor, true, true, false, true);
		}

	}
	return FReply::Handled();
}


void FLevelBuilderToolEdModeToolkit::OnNewHostTextCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	InputText = InText;
	InputString = InputText.ToString();
}

void FLevelBuilderToolEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	struct Local
	{
		static bool IsWidgetEnabled()
		{
			return GEditor->GetSelectedActors()->Num() != 0;
		}
	};
	SAssignNew(ToolkitWidget, SBorder).HAlign(HAlign_Center).Padding(5)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(5)
		[
			SNew(STextBlock).AutoWrapText(false).Text(LOCTEXT("HelperLabel", "Select StaticMesh actors Convert to InstanceMesh"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).Padding(5).AutoHeight()
		[
			SAssignNew(InputTag, SEditableTextBox).HintText(LOCTEXT("EditableTextBox", "Input Actor Tag"))
			.OnTextCommitted(this, &FLevelBuilderToolEdModeToolkit::OnNewHostTextCommited)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("SpawnActor", "Add InstanceStaticMesh Actor"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::SpawnActorOnClicked)
		.IsEnabled_Static(&Local::IsWidgetEnabled)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectedActor", "Selected Actors Used Tag"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::SelectedActorOnClicked)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("CheckErrorMergerInstanceActor", "Check for the Error MergerInstance Actor"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::CheckErrorMergerInstanceActor)
		]
	+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(10)
		[
			SNew(STextBlock).AutoWrapText(false).Text(LOCTEXT("Build Lightmapping", "Auto Build Lightmapping"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("PreviewLightmapping", "Preview Lightmapping"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::PreviewLightmapping)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("MediumLightmapping", "Medium Lightmapping"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::MediumLightmapping)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("HighLightmapping", "High Lightmapping"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::HighLightmapping)
		]
	+ SVerticalBox::Slot().HAlign(HAlign_Left).AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(LOCTEXT("EpicLightmapping", "Epic Lightmapping"))
		.OnClicked(this, &FLevelBuilderToolEdModeToolkit::EpicLightmapping)
		]

		];
	FModeToolkit::Init(InitToolkitHost);
}

FName FLevelBuilderToolEdModeToolkit::GetToolkitFName() const
{
	return FName("LevelBuilderToolEdMode");
}

FText FLevelBuilderToolEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("LevelBuilderToolEdModeToolkit", "DisplayName", "LevelBuilderToolEdMode Tool");
}

class FEdMode* FLevelBuilderToolEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FLevelBuilderToolEdMode::EM_LevelBuilderToolEdModeId);
}

void FLevelBuilderToolEdModeToolkit::CalculateLightMap(float Quality, int min, int max)
{
// 	FScopedSlowTask SlowTask(NumInLevel, LOCTEXT("BatchSetLightmapSize", "Batch Set Lightmap Size Num ..."));
// 	SlowTask.MakeDialog();
	for (TObjectIterator<AActor> Itr; Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		
		if (Actor->GetClass()!= AMergeInstance::StaticClass())
		{
			UStaticMeshComponent* StaticMeshComponent = nullptr;
			UInstancedStaticMeshComponent* InstancedStaticMeshComponent = nullptr;
			TArray<UActorComponent*> StaticMeshActorComponents = Actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
			TArray<UActorComponent*> InstancedStaticMeshActorComponents = Actor->GetComponentsByClass(UInstancedStaticMeshComponent::StaticClass());
			if (StaticMeshActorComponents.Num() > 0)
			{
				for (UActorComponent* StaticMeshActorComponent : StaticMeshActorComponents)
				{
					StaticMeshComponent = Cast<UStaticMeshComponent>(StaticMeshActorComponent);

					if (StaticMeshComponent != nullptr)
					{
						if (StaticMeshComponent->GetStaticMesh() != nullptr)
						{
							float ResizeBoundSize = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentTransform()).BoxExtent.Size();
							float BoundSize = StaticMeshComponent->GetStaticMesh()->GetBounds().BoxExtent.Size();
							float scaling = ResizeBoundSize / BoundSize;
							int32 LightMapUVDensity = int32(StaticMeshComponent->GetStaticMesh()->GetLightmapUVDensity());
							float scaleCalculation = scaling * LightMapUVDensity * 0.5;
							int32 NewLightMapSize = FMath::Max(int32(Quality * scaleCalculation)+ 3 & ~3, 4); 
							StaticMeshComponent->bOverrideLightMapRes = true;
							StaticMeshComponent->OverriddenLightMapRes = FMath::Clamp(NewLightMapSize, min, max);
							StaticMeshComponent->VisibilityId;
						}
					}
				}
				Actor->InvalidateLightingCache();
				Actor->PostEditChange();
				
			}
			if (InstancedStaticMeshActorComponents.Num() > 0)
			{
				for (UActorComponent* InstancedStaticMeshActorComponent : InstancedStaticMeshActorComponents)
				{
					InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(InstancedStaticMeshActorComponent);
					if (InstancedStaticMeshComponent != nullptr)
					{
						if (InstancedStaticMeshComponent->GetStaticMesh() != nullptr)
						{
							float ResizeBoundSize = InstancedStaticMeshComponent->CalcBounds(InstancedStaticMeshComponent->GetComponentTransform()).BoxExtent.Size();
							float AcotrScale = InstancedStaticMeshComponent->GetComponentTransform().GetScale3D().Size() / 6.0;
							float BoundSize = InstancedStaticMeshComponent->GetStaticMesh()->GetBounds().BoxExtent.Size();
							float scaling = ResizeBoundSize / BoundSize;
							int32 LightMapUVDensity = int32(InstancedStaticMeshComponent->GetStaticMesh()->GetLightmapUVDensity());
							float scaleCalculation = scaling * LightMapUVDensity * 0.5 * AcotrScale;
							int32 NewLightMapSize = FMath::Max(int32(Quality * scaleCalculation) + 3 & ~3, 4);
							InstancedStaticMeshComponent->bOverrideLightMapRes = true;
							InstancedStaticMeshComponent->OverriddenLightMapRes = FMath::Clamp(NewLightMapSize, min, max);
						}

					}
				}
				Actor->InvalidateLightingCache();
				Actor->PostEditChange();
			}
		}
		else
		{
			AMergeInstance* TempMergeInstance = Cast<AMergeInstance>(Actor);
			if (TempMergeInstance->MergeInstanceMesh->GetStaticMesh()!=nullptr)
			{
				float ResizeBoundSize = TempMergeInstance->MergeInstanceMesh->CalcBounds(TempMergeInstance->MergeInstanceMesh->GetComponentTransform()).BoxExtent.Size();
				float AcotrScale = TempMergeInstance->MergeInstanceMesh->GetComponentTransform().GetScale3D().Size() / 6.0;
				float BoundSize = TempMergeInstance->MergeInstanceMesh->GetStaticMesh()->GetBounds().BoxExtent.Size();
				float scaling = ResizeBoundSize / BoundSize;
				int32 LightMapUVDensity = int32(TempMergeInstance->MergeInstanceMesh->GetStaticMesh()->GetLightmapUVDensity());
				float scaleCalculation = scaling * LightMapUVDensity * 0.5 * AcotrScale;
				int32 NewLightMapSize = FMath::Max(int32(Quality * scaleCalculation) + 3 & ~3, 4);
				TempMergeInstance->MergeInstanceMesh->bOverrideLightMapRes = true;
				TempMergeInstance->MergeInstanceMesh->OverriddenLightMapRes = FMath::Clamp(NewLightMapSize, min, max);
			}
			TempMergeInstance->InvalidateLightingCache();
			TempMergeInstance->PostEditChange();
		}
	}
}
#undef LOCTEXT_NAMESPACE
