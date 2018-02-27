// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeTile.h"
#include "UnrealNetwork.h"

// Sets default values
AMazeTile::AMazeTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  if (HasAuthority()) {
    this->SetReplicates(true);
    this->SetReplicateMovement(true);
  }

  this->tileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MazeTileMesh"));
  RootComponent = this->tileMesh;
  this->animInstance = Cast<UMazeTileAnimInstance>(this->tileMesh->GetAnimInstance());

  if (!this->animInstance) return;
  this->animInstance->TileType = CROSS;
}

void AMazeTile::SetTileType(int newTileType) {
  this->TileType = newTileType;

  if (HasAuthority()) {
    this->UpdateAnimInstanceTileType();
  }
}

void AMazeTile::UpdateAnimInstanceTileType() {
  if (!this->animInstance) return;
  this->animInstance->TileType = this->TileType;
}

void AMazeTile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AMazeTile, TileType);
}
