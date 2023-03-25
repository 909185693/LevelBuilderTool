// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "MergeInstance.generated.h"

UCLASS()
class LEVELBUILDERTOOL_API AMergeInstance : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMergeInstance();

	//定义实例化模型
	UPROPERTY(EditDefaultsOnly, Category = InstanceMesh)
	UInstancedStaticMeshComponent* MergeInstanceMesh;

	//定义实例化模型蓝图参考位置
	UPROPERTY(EditDefaultsOnly, Category = InstanceMesh)
	UStaticMeshComponent* ReferenceComponent;

	//定义Tag
	UPROPERTY(EditAnywhere, Category = Setup)
	FName ActorTags = "None";

	//函数暴露给蓝图方便外部操作
	UFUNCTION(BlueprintCallable)
	void SetMergeInstanceAcotrTag(FName NewActorTags);

	//错误的Actor:和其他Actor的模型不相同或者材质不相同
	UPROPERTY(EditAnywhere, Category = Setup)
	TArray<AStaticMeshActor*> ErrorStaticMeshActors;

	//编辑模式
	UPROPERTY(EditAnywhere, Category = Setup)
	bool bEditMode = true;

	//是否实例化
	bool bInstanced = false;

	//判断是否可以被实例化
	bool CanInstance;

	//剔除距离
	UPROPERTY(EditAnywhere, Category = Setup)
	float NewDistanceCull;

	//场景需要实例化的Actor
	UPROPERTY(SaveGame)
	TArray<AStaticMeshActor*> NeedInstanceActor;

	//符合条件的实例化模型
	UPROPERTY(SaveGame)
	TArray<AStaticMeshActor*> CompareStaticMeshActor;

	//定义材质数组
	UPROPERTY(SaveGame)
	TArray<UMaterialInterface*> HasMaterials;

	//定义场景模型的Transforms数组
	UPROPERTY(SaveGame)
	TArray<FTransform> Transforms;

	//定义原始模型
	UPROPERTY(SaveGame)
	UStaticMesh* StaticMesh;

	//通过Tag获取场景中的Actor
	void GetAllActorWithTag(FName Tag);

	//添加实例化参数到InstanceMesh
	void AddInstanceMesh();

	//需要实例化模型的材质
	UPROPERTY(SaveGame)
	UMaterialInstance* SetMaterials;

	//光照贴图
 	UPROPERTY(EditAnywhere, Category = Setup)
 	int32 OverrideLightMap = 32;

#if WITH_EDITOR
	//定义蓝图编辑器修改触发时间
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) ;
#endif

};
