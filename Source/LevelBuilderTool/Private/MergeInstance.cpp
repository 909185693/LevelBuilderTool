// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeInstance.h"
#include "EngineUtils.h"
#include "Components/InstancedStaticMeshComponent.h"


// Sets default values
AMergeInstance::AMergeInstance()
{
	PrimaryActorTick.bCanEverTick = false;

	//����SceneRoot
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	//RootComponent->SetMobility(EComponentMobility::Static);

	//Scene->SetMobility(EComponentMobility::Static);
	//����ʵ��ģ��
	MergeInstanceMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstanceMesh"));
	MergeInstanceMesh->SetupAttachment(RootComponent);
	MergeInstanceMesh->SetMobility(EComponentMobility::Static);
	MergeInstanceMesh->SetStaticMesh(StaticMesh);
	MergeInstanceMesh->bOverrideLightMapRes = true;
	//MergeInstanceMesh->bCastCombineShadow = true;
	//�����ؿ��кϲ����ߵĲο���ģ��
	ReferenceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReferenceComponent"));
	ReferenceComponent->SetMobility(EComponentMobility::Movable);
	ReferenceComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	ReferenceComponent->SetStaticMesh(MeshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	ReferenceComponent->SetMaterial(0, MaterialAsset.Object);
	ReferenceComponent->CastShadow = false;
	ReferenceComponent->bHiddenInGame = true;
	ReferenceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

#if WITH_EDITORONLY_DATA
	ReferenceComponent->bEnableAutoLODGeneration = false;
#endif
	//ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 1, 1));
}

void AMergeInstance::SetMergeInstanceAcotrTag(FName NewActorTags)
{
	ActorTags = NewActorTags;
}

//ֻ���ٱ༭����ִ�еĺ���
void AMergeInstance::GetAllActorWithTag(FName Tag)
{
	if (!Tag.IsNone())
	{	//�ռ�Actorǰ��������
		NeedInstanceActor.Empty();
		CompareStaticMeshActor.Empty();
		Transforms.Empty();
		HasMaterials.Empty();
		ErrorStaticMeshActors.Empty();
		//��ȡ����Tag��Actor
		UWorld* World = this->GetWorld();
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			AStaticMeshActor* StaticMeshActor = *It;
			if (StaticMeshActor->ActorHasTag(Tag))
			{
				NeedInstanceActor.Add(StaticMeshActor);
			}

		}
		//�����ȡ���ؿ������Actor��ִ�л�ȡTransform��Materials,�����ӡ������Ϣ
		if (NeedInstanceActor.Num() > 0)
		{
			StaticMesh = NeedInstanceActor[0]->GetStaticMeshComponent()->GetStaticMesh();
			HasMaterials = NeedInstanceActor[0]->GetStaticMeshComponent()->GetMaterials();
			for (AStaticMeshActor* StaticMeshActorNew : NeedInstanceActor)
			{
				if (StaticMeshActorNew->GetStaticMeshComponent()->GetStaticMesh() == StaticMesh && StaticMeshActorNew->GetStaticMeshComponent()->GetMaterials() == HasMaterials)
				{
					CompareStaticMeshActor.Add(StaticMeshActorNew);
					Transforms.Add(StaticMeshActorNew->GetStaticMeshComponent()->GetComponentTransform());
				}
				else
				{
					ErrorStaticMeshActors.Add(StaticMeshActorNew);
				}
			}
		}
		else {
			check(GEngine != nullptr);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No match Tag for the Level"));
			ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 0, 0));
		}
	}
}

void AMergeInstance::AddInstanceMesh()
{
	MergeInstanceMesh->SetStaticMesh(StaticMesh);
	for (FTransform Transform : Transforms)
	{
		MergeInstanceMesh->AddInstance(Transform);
	}
	int i = 0;
	for (UMaterialInterface* Mat : HasMaterials)
	{
		MergeInstanceMesh->SetMaterial(i, Mat);
		i++;
	}

}

//ֻ���ٱ༭����ִ�еĺ������޸�Actor����
#if WITH_EDITOR
void AMergeInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (MergeInstanceMesh->GetInstanceCount()>0)
	{
		bInstanced = true;
	}
	//���þ����޳��͹�����ͼ��С
	MergeInstanceMesh->LDMaxDrawDistance = NewDistanceCull;
	MergeInstanceMesh->SetCachedMaxDrawDistance(NewDistanceCull);
	MergeInstanceMesh->OverriddenLightMapRes = OverrideLightMap;
	MergeInstanceMesh->bOverrideLightMapRes = true;
	MergeInstanceMesh->PostEditChange();
	//�༭ģʽ
	if (bEditMode)
	{
		//֮ǰʵ��������Ѿ������ʱ���ݣ��༭ģʽ��ʱ���ж�NeedInstanceActor�Ƿ�Ϊ�գ�Ϊ�վ�Ҫ�ռ��¾��и�Tag��ģ��
		if (NeedInstanceActor.Num() == 0)
		{
			GetAllActorWithTag(ActorTags);
		}
		else
		{
			NeedInstanceActor.Empty();
			GetAllActorWithTag(ActorTags);
		}

		//�༭ģʽ��ʾģ�ͺ�ȥ����Ϸ����
		for (AStaticMeshActor* StaticMeshActor : NeedInstanceActor)
		{
			StaticMeshActor->GetStaticMeshComponent()->SetVisibility(true);
			StaticMeshActor->SetActorHiddenInGame(false);
			StaticMeshActor->SetIsTemporarilyHiddenInEditor(false);
			StaticMeshActor->bHiddenEd = false;
		}
		//��ʾԭʼģ����ʾ�������������
		NeedInstanceActor.Empty();
		CompareStaticMeshActor.Empty();
		Transforms.Empty();
		HasMaterials.Empty();
		ErrorStaticMeshActors.Empty();
		//ɾ��ʵ����ģ��
		int32 InstanceCount = MergeInstanceMesh->GetInstanceCount();
		for (int i = 0;i <= InstanceCount;i++)
		{
			MergeInstanceMesh->RemoveInstance(i);
			if (MergeInstanceMesh->GetInstanceCount() > 0)
			{
				MergeInstanceMesh->RemoveInstance(0);
			}

		}
		//ʵ����״̬����ʾ״̬�ı�
		bInstanced = false;
		ReferenceComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
		ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 1, 1));
	}
	//�ϲ�ʵ����
	if (!bEditMode)
	{
		//���ж��Ƿ����Ѿ�ʵ��������״̬
		if (!bInstanced)
		{
			//��ʼ�����飬�ռ�Actor
			NeedInstanceActor = {};
			GetAllActorWithTag(ActorTags);
			//�ж�ʵ�������������������ռ���Actor����ͬһ��ģ����ִ��ʵ����
			if (NeedInstanceActor.Num() == CompareStaticMeshActor.Num() && NeedInstanceActor.Num() > 0)
			{	
				AddInstanceMesh();
				if (MergeInstanceMesh->GetInstanceCount()==NeedInstanceActor.Num())
				{
					//�ϲ��ɹ������óɹ�״̬
					bInstanced = true;

					//�ϲ��ɹ�������ԭ���ĵ���ģ��
					for (AStaticMeshActor* StaticMeshActor : NeedInstanceActor)
					{
						StaticMeshActor->GetStaticMeshComponent()->SetVisibility(false);
						StaticMeshActor->SetActorHiddenInGame(true);
						StaticMeshActor->SetIsTemporarilyHiddenInEditor(true);
						StaticMeshActor->bHiddenEd = true;
					}
					//ʵ�����ɹ���״̬����ʾ״̬�ı�
					ReferenceComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
					ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(0, 1, 0));
					//�ϲ��ɹ�������ģ���ռ�����
					NeedInstanceActor.Empty();
					CompareStaticMeshActor.Empty();
					Transforms.Empty();
					HasMaterials.Empty();
					ErrorStaticMeshActors.Empty();
				}
				else
				{
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Merge Error,Please check and try again!"));
					ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 0, 0));
				}
			}
			else
			{
				//δ�ϲ��ɹ���ʾ������ɫ����ɫ
				check(GEngine != nullptr);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Merge Error,There are different StaticMeshActor! See ErrorStaticMeshActors"));
				ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 0, 0));
			}
			
		}

	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
