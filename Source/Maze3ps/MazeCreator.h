// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeTile.h"
#include "Maze3psCharacter.h"
#include "MazeCreator.generated.h"

#define NORTH 1
#define SOUTH 2
#define WEST 3
#define EAST 4

USTRUCT()
struct FStructTile {
  GENERATED_BODY()
  UPROPERTY()
  bool hasNorthWall = true;
  UPROPERTY()
  bool hasSouthWall = true;
  UPROPERTY()
  bool hasWestWall = true;
  UPROPERTY()
  bool hasEastWall = true;
  UPROPERTY()
  bool visited = false;
  UPROPERTY()
  int x;
  UPROPERTY()
  int y;
};

// if this fails change to development editor
UCLASS()
class MAZE3PS_API AMazeCreator : public AActor
{
	GENERATED_BODY()
	
public:	

  UPROPERTY(EditAnywhere)
    int height = 100;

  UPROPERTY(EditAnywhere)
    int width = 100;

  UPROPERTY(EditAnywhere)
    float timeToRegenerateMaze = 10.0f;

  UPROPERTY(EditAnywhere)
    TSubclassOf<AMazeTile> MazeTileBP;

	// Sets default values for this actor's properties
  AMazeCreator(const FObjectInitializer& ObjectInitializer);
  ~AMazeCreator();

  void generate();

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  FVector2D* FindMazeTileIndexByLocation(FVector location);

  AMazeTile * GetMazeTileByIndex(int x, int y);

  float hasMaze = 0;

  void DebugPrintMaze();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

  FStructTile ** tiles;

  AMazeTile ** mazeTiles;

  UPROPERTY(EditAnywhere)
    int TileWidth = 900;

  UPROPERTY(EditAnywhere)
    int RenderRadius = 5;

  float timeSinceLastMazeCreate = 0;
  float timePassed = 0;
  bool needsMazeRefresh = false;
  bool InitialStart = true;

  void renderMaze(float DeltaTime);

  void generateInitialTiles();

  FStructTile* getNorthTile(FStructTile* currentTile);

  FStructTile* getSouthTile(FStructTile* currentTile);

  FStructTile* getEastTile(FStructTile* currentTile);

  FStructTile* getWestTile(FStructTile* currentTile);

  FStructTile* getTileAtIndex(int x, int y);

  int getTileIndex(int x, int y);
};
