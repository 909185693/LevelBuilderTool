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

	//����ʵ����ģ��
	UPROPERTY(EditDefaultsOnly, Category = InstanceMesh)
	UInstancedStaticMeshComponent* MergeInstanceMesh;

	//����ʵ����ģ����ͼ�ο�λ��
	UPROPERTY(EditDefaultsOnly, Category = InstanceMesh)
	UStaticMeshComponent* ReferenceComponent;

	//����Tag
	UPROPERTY(EditAnywhere, Category = Setup)
	FName ActorTags = "None";

	//������¶����ͼ�����ⲿ����
	UFUNCTION(BlueprintCallable)
	void SetMergeInstanceAcotrTag(FName NewActorTags);

	//�����Actor:������Actor��ģ�Ͳ���ͬ���߲��ʲ���ͬ
	UPROPERTY(EditAnywhere, Category = Setup)
	TArray<AStaticMeshActor*> ErrorStaticMeshActors;

	//�༭ģʽ
	UPROPERTY(EditAnywhere, Category = Setup)
	bool bEditMode = true;

	//�Ƿ�ʵ����
	bool bInstanced = false;

	//�ж��Ƿ���Ա�ʵ����
	bool CanInstance;

	//�޳�����
	UPROPERTY(EditAnywhere, Category = Setup)
	float NewDistanceCull;

	//������Ҫʵ������Actor
	UPROPERTY(SaveGame)
	TArray<AStaticMeshActor*> NeedInstanceActor;

	//����������ʵ����ģ��
	UPROPERTY(SaveGame)
	TArray<AStaticMeshActor*> CompareStaticMeshActor;

	//�����������
	UPROPERTY(SaveGame)
	TArray<UMaterialInterface*> HasMaterials;

	//���峡��ģ�͵�Transforms����
	UPROPERTY(SaveGame)
	TArray<FTransform> Transforms;

	//����ԭʼģ��
	UPROPERTY(SaveGame)
	UStaticMesh* StaticMesh;

	//ͨ��Tag��ȡ�����е�Actor
	void GetAllActorWithTag(FName Tag);

	//���ʵ����������InstanceMesh
	void AddInstanceMesh();

	//��Ҫʵ����ģ�͵Ĳ���
	UPROPERTY(SaveGame)
	UMaterialInstance* SetMaterials;

	//������ͼ
 	UPROPERTY(EditAnywhere, Category = Setup)
 	int32 OverrideLightMap = 32;

#if WITH_EDITOR
	//������ͼ�༭���޸Ĵ���ʱ��
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) ;
#endif

};
