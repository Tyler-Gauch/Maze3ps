// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeTileAnimInstance.h"
#include "MazeTile.generated.h"

UCLASS()
class MAZE3PS_API AMazeTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMazeTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere)
  USkeletalMeshComponent* tileMesh;

  UMazeTileAnimInstance* animInstance;
};
