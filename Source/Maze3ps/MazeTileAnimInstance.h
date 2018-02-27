// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MazeTileAnimInstance.generated.h"

#define CROSS               0
#define CORNER_SOUTH_WEST   1
#define CORNER_WEST_NORTH   2
#define CORNER_NORTH_EAST   3
#define CORNER_EAST_SOUTH   4
#define HALLWAY_NORTH_SOUTH 5
#define HALLWAY_EAST_WEST   6
#define DEADEND_SOUTH       7
#define DEADEND_WEST        8
#define DEADEND_NORTH       9
#define DEADEND_EAST        10
#define T_SOUTH             11
#define T_EAST              12
#define T_NORTH             13
#define T_WEST              14

/**
 * 
 */
UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class MAZE3PS_API UMazeTileAnimInstance : public UAnimInstance
{
  GENERATED_BODY()

public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int TileType = 0;
};
