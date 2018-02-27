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

  UPROPERTY(EditAnywhere)
  USkeletalMeshComponent* tileMesh;

  UMazeTileAnimInstance* animInstance;

  void SetTileType(int newTileType);

private:

  UPROPERTY(Replicated)
    int TileType = 0;

  void UpdateAnimInstanceTileType();
};
