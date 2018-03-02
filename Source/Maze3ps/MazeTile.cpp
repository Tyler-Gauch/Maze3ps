// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeTile.h"
#include "UnrealNetwork.h"

// Sets default values
AMazeTile::AMazeTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

  if (HasAuthority()) {
    this->SetReplicates(true);
    this->SetReplicateMovement(true);
  }

  this->tileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MazeTileMesh"));
  RootComponent = this->tileMesh;
}

void AMazeTile::BeginPlay() {
  Super::BeginPlay();
  this->animInstance = Cast<UMazeTileAnimInstance>(this->tileMesh->GetAnimInstance());

  if (!this->animInstance) {
    UE_LOG(LogTemp, Error, TEXT("Failed to get animinstance"))
    return;
  }
  this->animInstance->TileType = CROSS;
}

bool AMazeTile::Server_SetTileType_Validate(int newTileType) {
  return true;
}

void AMazeTile::Server_SetTileType_Implementation(int newTileType) {
  this->TileType = newTileType;
  this->OnRep_TileType();
}

void AMazeTile::OnRep_TileType() {
  if (!this->animInstance) return;
  this->animInstance->TileType = this->TileType;
}

void AMazeTile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AMazeTile, TileType);
}
