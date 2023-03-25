// Fill out your copyright notice in the Description page of Project Settings.


#include "MergeInstance.h"
#include "EngineUtils.h"
#include "Components/InstancedStaticMeshComponent.h"


// Sets default values
AMergeInstance::AMergeInstance()
{
	PrimaryActorTick.bCanEverTick = false;

	//创建SceneRoot
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	//RootComponent->SetMobility(EComponentMobility::Static);

	//Scene->SetMobility(EComponentMobility::Static);
	//创建实例模型
	MergeInstanceMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstanceMesh"));
	MergeInstanceMesh->SetupAttachment(RootComponent);
	MergeInstanceMesh->SetMobility(EComponentMobility::Static);
	MergeInstanceMesh->SetStaticMesh(StaticMesh);
	MergeInstanceMesh->bOverrideLightMapRes = true;
	//MergeInstanceMesh->bCastCombineShadow = true;
	//创建关卡中合并工具的参考点模型
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

//只能再编辑器中执行的函数
void AMergeInstance::GetAllActorWithTag(FName Tag)
{
	if (!Tag.IsNone())
	{	//收集Actor前重置数组
		NeedInstanceActor.Empty();
		CompareStaticMeshActor.Empty();
		Transforms.Empty();
		HasMaterials.Empty();
		ErrorStaticMeshActors.Empty();
		//获取符合Tag的Actor
		UWorld* World = this->GetWorld();
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			AStaticMeshActor* StaticMeshActor = *It;
			if (StaticMeshActor->ActorHasTag(Tag))
			{
				NeedInstanceActor.Add(StaticMeshActor);
			}

		}
		//如果获取到关卡的相关Actor则执行获取Transform和Materials,否则打印错误信息
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

//只能再编辑器中执行的函数：修改Actor参数
#if WITH_EDITOR
void AMergeInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (MergeInstanceMesh->GetInstanceCount()>0)
	{
		bInstanced = true;
	}
	//设置距离剔除和光照贴图大小
	MergeInstanceMesh->LDMaxDrawDistance = NewDistanceCull;
	MergeInstanceMesh->SetCachedMaxDrawDistance(NewDistanceCull);
	MergeInstanceMesh->OverriddenLightMapRes = OverrideLightMap;
	MergeInstanceMesh->bOverrideLightMapRes = true;
	MergeInstanceMesh->PostEditChange();
	//编辑模式
	if (bEditMode)
	{
		//之前实例化完成已经清除临时数据，编辑模式的时候判断NeedInstanceActor是否为空，为空就要收集下具有改Tag的模型
		if (NeedInstanceActor.Num() == 0)
		{
			GetAllActorWithTag(ActorTags);
		}
		else
		{
			NeedInstanceActor.Empty();
			GetAllActorWithTag(ActorTags);
		}

		//编辑模式显示模型和去掉游戏隐藏
		for (AStaticMeshActor* StaticMeshActor : NeedInstanceActor)
		{
			StaticMeshActor->GetStaticMeshComponent()->SetVisibility(true);
			StaticMeshActor->SetActorHiddenInGame(false);
			StaticMeshActor->SetIsTemporarilyHiddenInEditor(false);
			StaticMeshActor->bHiddenEd = false;
		}
		//显示原始模型显示后重置相关数组
		NeedInstanceActor.Empty();
		CompareStaticMeshActor.Empty();
		Transforms.Empty();
		HasMaterials.Empty();
		ErrorStaticMeshActors.Empty();
		//删除实例化模型
		int32 InstanceCount = MergeInstanceMesh->GetInstanceCount();
		for (int i = 0;i <= InstanceCount;i++)
		{
			MergeInstanceMesh->RemoveInstance(i);
			if (MergeInstanceMesh->GetInstanceCount() > 0)
			{
				MergeInstanceMesh->RemoveInstance(0);
			}

		}
		//实例化状态和显示状态改变
		bInstanced = false;
		ReferenceComponent->SetWorldScale3D(FVector(3.0f, 3.0f, 3.0f));
		ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 1, 1));
	}
	//合并实例化
	if (!bEditMode)
	{
		//先判断是否是已经实例化过的状态
		if (!bInstanced)
		{
			//初始化数组，收集Actor
			NeedInstanceActor = {};
			GetAllActorWithTag(ActorTags);
			//判断实例化满足的条件：如果收集的Actor都是同一种模型则执行实例化
			if (NeedInstanceActor.Num() == CompareStaticMeshActor.Num() && NeedInstanceActor.Num() > 0)
			{	
				AddInstanceMesh();
				if (MergeInstanceMesh->GetInstanceCount()==NeedInstanceActor.Num())
				{
					//合并成功后设置成功状态
					bInstanced = true;

					//合并成功后隐藏原来的单个模型
					for (AStaticMeshActor* StaticMeshActor : NeedInstanceActor)
					{
						StaticMeshActor->GetStaticMeshComponent()->SetVisibility(false);
						StaticMeshActor->SetActorHiddenInGame(true);
						StaticMeshActor->SetIsTemporarilyHiddenInEditor(true);
						StaticMeshActor->bHiddenEd = true;
					}
					//实例化成功后状态和显示状态改变
					ReferenceComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
					ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(0, 1, 0));
					//合并成功后重置模型收集数组
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
				//未合并成功显示错误颜色，红色
				check(GEngine != nullptr);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Merge Error,There are different StaticMeshActor! See ErrorStaticMeshActors"));
				ReferenceComponent->SetVectorParameterValueOnMaterials("Color", FVector(1, 0, 0));
			}
			
		}

	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
