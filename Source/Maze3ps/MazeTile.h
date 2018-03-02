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

  UFUNCTION(reliable, server, WithValidation)
  void Server_SetTileType(int newTileType);
  bool Server_SetTileType_Validate(int newTileType);
  void Server_SetTileType_Implementation(int newTileType);

  virtual void BeginPlay() override;

private:

  UPROPERTY(ReplicatedUsing=OnRep_TileType)
    int TileType = 0;

  UFUNCTION()
  void OnRep_TileType();
};
