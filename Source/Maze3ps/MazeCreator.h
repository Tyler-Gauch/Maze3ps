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

struct Tile {
  bool hasNorthWall = true;
  bool hasSouthWall = true;
  bool hasWestWall = true;
  bool hasEastWall = true;
  bool visited = false;
  int x;
  int y;
};

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
    TSubclassOf<AMazeTile> MazeTileBP;

	// Sets default values for this actor's properties
	AMazeCreator();
  ~AMazeCreator();

  void generate();

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  FVector2D* FindMazeTileIndexByLocation(FVector location);

  AMazeTile * GetMazeTileByIndex(int x, int y);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

  Tile ** tiles;

  AMazeTile ** mazeTiles;

  AMaze3psCharacter* character;

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

  Tile* getNorthTile(Tile * currentTile);

  Tile* getSouthTile(Tile * currentTile);

  Tile* getEastTile(Tile * currentTile);

  Tile* getWestTile(Tile * currentTile);

  Tile* getTileAtIndex(int x, int y);

  int getTileIndex(int x, int y);
	
	
};
