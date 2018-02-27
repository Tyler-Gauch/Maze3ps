// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeCreator.h"
#include <stack>
#include "UnrealMathUtility.h"
#include "MazeTileAnimInstance.h"
#include "EngineUtils.h"
#include "UnrealNetwork.h"
#pragma optimize("", off)

int getMultiDimensionalIndexInSingleDimensionArray(int x, int y, int width) {
  return x + width * y;
}

// Sets default values
AMazeCreator::AMazeCreator(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

  this->tiles = new FStructTile *[this->height * this->width];
  this->mazeTiles = new AMazeTile *[this->height * this->width];
}

AMazeCreator::~AMazeCreator() {
  delete[] this->tiles;
  delete[] this->mazeTiles;
}

void AMazeCreator::DebugPrintMaze() {
  int mapHeight = (3 * this->height);
  int mapWidth = (3 * this->width);

  char * map = new char[mapHeight * mapWidth];

  for (int i = 0; i < mapHeight * mapWidth; i++) {
    map[i] = '&';
  }

  for (int y = 0; y < this->height; y++) {
    for (int x = 0; x < this->width; x++) {
      int currentTileX = 1 + (x * 3);
      int currentTileY = 1 + (y * 3);

      FStructTile* tile = this->getTileAtIndex(x, y);

      int currentTile = getMultiDimensionalIndexInSingleDimensionArray(currentTileX, currentTileY, mapHeight);
      int northTile = getMultiDimensionalIndexInSingleDimensionArray(currentTileX, currentTileY - 1, mapHeight);
      int eastTile = getMultiDimensionalIndexInSingleDimensionArray(currentTileX + 1, currentTileY, mapHeight);
      int southTile = getMultiDimensionalIndexInSingleDimensionArray(currentTileX, currentTileY + 1, mapHeight);
      int westTile = getMultiDimensionalIndexInSingleDimensionArray(currentTileX - 1, currentTileY, mapHeight);

      bool isStartingTile = (currentTileX == 1 && currentTileY == 1);

      if (isStartingTile) {
        map[currentTile] = 'S';
        map[northTile] = 'n';
        map[southTile] = 's';
        map[eastTile] = 'e';
        map[westTile] = 'w';
      } else {
        map[currentTile] = ' ';

        if (tile->hasNorthWall) {
          map[northTile] = '&';
        } else {
          map[northTile] = ' ';
        }

        if (tile->hasEastWall) {
          map[eastTile] = '&';
        } else {
          map[eastTile] = ' ';
        }

        if (tile->hasSouthWall) {
          map[southTile] = '&';
        } else {
          map[southTile] = ' ';
        }

        if (tile->hasWestWall) {
          map[westTile] = '&';
        } else {
          map[westTile] = ' ';
        }

      }


    }
  }

  FString row = TEXT("");

  for (int i = 0; i < mapHeight * mapWidth; i++) {
    row += map[i];

    if (row.Len() % mapWidth == 0) {
      UE_LOG(LogTemp, Warning, TEXT("%s"), *row);
      row = TEXT("");
    }
  }
}

// Called when the game starts or when spawned
void AMazeCreator::BeginPlay() {
  Super::BeginPlay();

  if (HasAuthority()) {
    this->generateInitialTiles();
    this->generate();

    this->hasMaze = true;
    this->InitialStart = false;
  }
}

// Called every frame
void AMazeCreator::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  this->timeSinceLastMazeCreate += DeltaTime;
  this->timePassed += DeltaTime;

  if (this->timeSinceLastMazeCreate > 10.0f && HasAuthority()) {
    this->generateInitialTiles();
    this->generate();
  }  

  if (HasAuthority()) {
    this->renderMaze(DeltaTime);
  }
}

void AMazeCreator::generate() {
  if (!HasAuthority()) {
    return;
  }
  // used to keep track of current places we have gone
  // so we can backtrack
  std::stack<FStructTile*> path;
  int x = 1, y = 1;
  bool hasUnvisted = true;
  needsMazeRefresh = true;
  this->timeSinceLastMazeCreate = 0;

  FStructTile* startTile = this->getTileAtIndex(x, y);

  // we use 2 because forward 1 space is a wall!
  FStructTile* currentTile = startTile;
  FStructTile* nextTile = nullptr;

  while (hasUnvisted && path.size() < this->width * this->height + 10) {
    // get the current tile off the top of the stack
    currentTile->visited = true;

    int32 startDirection = FMath::RandRange(1, 4);
    int32 currentDirection = startDirection;
    bool findingDirection = true;

    // search through the possible next directions, N, S, E, W
    // pick one at random if we can go to that direction then use that
    // if we cant then increment the current direction by 1. N->S->E->W->N if
    // we hit the starting direction stop and we have no direction to go to
    // we can't go to a direction if:
    //        A: The tile doesn't exist!
    //        B: The tile is visitied
    while (findingDirection) {
      switch (currentDirection) {
        case NORTH:
            nextTile = this->getNorthTile(currentTile);
            break;
        case SOUTH:
            nextTile = this->getSouthTile(currentTile);
            break;
        case WEST:
            nextTile = this->getWestTile(currentTile);
            break;
        case EAST:
            nextTile = this->getEastTile(currentTile);
            break;
      }

      if (nextTile == nullptr) { // tile doesn't exist
        currentDirection++;
      } else if (nextTile->visited) { // tile is already visited
        currentDirection++;
      } else { // tile is good to go
        findingDirection = false;
        break;
      }

      if (currentDirection > 4) {
        currentDirection = 1;
      }

      // we have no tiles to visit :(
      if (currentDirection == startDirection) {
        findingDirection = false;
        break;
      }
    }

    if (nextTile != nullptr) { // if we had a tile move to that tile
      path.push(currentTile);

      // remove the wall between the tiles
      switch (currentDirection) {
        case NORTH:
          nextTile->hasSouthWall = false;
          currentTile->hasNorthWall = false;
          break;
        case SOUTH:
          nextTile->hasNorthWall = false;
          currentTile->hasSouthWall = false;
          break;
        case WEST:
          nextTile->hasEastWall = false;
          currentTile->hasWestWall = false;
          break;
        case EAST:
          nextTile->hasWestWall = false;
          currentTile->hasEastWall = false;
          break;
      }

      currentTile = nextTile;
    } else if (!path.empty()) {
      currentTile = path.top();
      path.pop();
    } else {
      hasUnvisted = false;
    }
  }

  this->DebugPrintMaze();
}

void AMazeCreator::renderMaze(float DeltaTime) {

  if (this->hasMaze != 1) {
    UE_LOG(LogTemp, Error, TEXT("unable to render maze waiting for generation!"));
    return;
  }

  UWorld* world = GetWorld();

  FVector location(0,0,0);
  for (int y = 0; y < this->height; y++) {
    for (int x = 0; x < this->width; x++) {
      int currentTileIndex = this->getTileIndex(x, y);
      location.X = this->TileWidth * x; //todo use box extents figure it out
      location.Y = this->TileWidth * y;
      AMazeTile* mazeTile = this->mazeTiles[currentTileIndex];

      if (mazeTile != NULL) {
        if (!this->needsMazeRefresh) {
          continue;
        }
      } else if (mazeTile == NULL) {
        mazeTile = (AMazeTile*)world->SpawnActor(MazeTileBP, &location);
      } else {
        continue;
      }

      FStructTile* tileData = this->getTileAtIndex(x, y);
      // determine the state of the tile

      if (!tileData->hasNorthWall && tileData->hasEastWall && tileData->hasWestWall && tileData->hasSouthWall) {
        mazeTile->SetTileType(DEADEND_NORTH);
      } else if (!tileData->hasSouthWall && tileData->hasEastWall && tileData->hasWestWall && tileData->hasNorthWall) {
        mazeTile->SetTileType(DEADEND_SOUTH);
      } else if (!tileData->hasEastWall && tileData->hasSouthWall && tileData->hasWestWall && tileData->hasNorthWall) {
        mazeTile->SetTileType(DEADEND_EAST);
      } else if (!tileData->hasWestWall && tileData->hasSouthWall && tileData->hasEastWall && tileData->hasNorthWall) {
        mazeTile->SetTileType(DEADEND_WEST);
      } else if (!tileData->hasEastWall && !tileData->hasWestWall && tileData->hasNorthWall && tileData->hasSouthWall) {
        mazeTile->SetTileType(HALLWAY_EAST_WEST);
      } else if (tileData->hasEastWall && tileData->hasWestWall && !tileData->hasNorthWall && !tileData->hasSouthWall) {
        mazeTile->SetTileType(HALLWAY_NORTH_SOUTH);
      } else if (tileData->hasSouthWall && !tileData->hasEastWall && !tileData->hasWestWall && !tileData->hasNorthWall) {
        mazeTile->SetTileType(T_NORTH);
      } else if (!tileData->hasSouthWall && !tileData->hasEastWall && !tileData->hasWestWall && tileData->hasNorthWall) {
        mazeTile->SetTileType(T_SOUTH);
      } else if (!tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall && !tileData->hasNorthWall) {
        mazeTile->SetTileType(T_EAST);
      } else if (!tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall && !tileData->hasNorthWall) {
        mazeTile->SetTileType(T_WEST);
      } else if (tileData->hasNorthWall && !tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall) {
        mazeTile->SetTileType(CORNER_EAST_SOUTH);
      } else if (!tileData->hasNorthWall && tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall) {
        mazeTile->SetTileType(CORNER_NORTH_EAST);
      } else if (tileData->hasNorthWall && !tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall) {
        mazeTile->SetTileType(CORNER_SOUTH_WEST);
      } else if (!tileData->hasNorthWall && tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall) {
        mazeTile->SetTileType(CORNER_WEST_NORTH);
      } else {
        mazeTile->SetTileType(CROSS);
      }

      this->mazeTiles[this->getTileIndex(x, y)] = mazeTile;
    }
  }

  this->needsMazeRefresh = false;
}

void AMazeCreator::generateInitialTiles() {
  for (int y = 0; y < this->height; y++) {
    for (int x = 0; x < this->width; x++) {
      FStructTile * tile = new FStructTile;
      tile->x = x;
      tile->y = y;
      int tileIndex = this->getTileIndex(x, y);
      this->tiles[tileIndex] = tile;
      if (this->InitialStart) {
        this->mazeTiles[tileIndex] = NULL;
      }
    }
  }  
}

FStructTile* AMazeCreator::getNorthTile(FStructTile* currentTile) {
  return this->getTileAtIndex(currentTile->x, currentTile->y - 1);
}

FStructTile* AMazeCreator::getSouthTile(FStructTile* currentTile) {
  return this->getTileAtIndex(currentTile->x, currentTile->y + 1);
}

FStructTile* AMazeCreator::getEastTile(FStructTile* currentTile) {
  return this->getTileAtIndex(currentTile->x + 1, currentTile->y);
}

FStructTile* AMazeCreator::getWestTile(FStructTile* currentTile) {
  // first check if we have a west tile
  return this->getTileAtIndex(currentTile->x - 1, currentTile->y);
}

FStructTile* AMazeCreator::getTileAtIndex(int x, int y) {
  // make sure the tile we want exists. The getMultiDimensionalIndexInSingleDimensionArray will
  // wrap rows if we ask for an east tile and there isn't one or vice versa.
  if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
    return nullptr;
  }
  // lets make sure this tile is within our bounds
  int index = this->getTileIndex(x, y);
  if (index < 0 || index >= this->width * this->height) {
    return nullptr;
  }

  return this->tiles[index];
}

int AMazeCreator::getTileIndex(int x, int y) {
  return getMultiDimensionalIndexInSingleDimensionArray(x, y, this->width);
}

FVector2D* AMazeCreator::FindMazeTileIndexByLocation(FVector location) {
  return new FVector2D(FMath::CeilToFloat((location.X) / this->TileWidth) - 1, FMath::CeilToFloat((location.Y) / this->TileWidth) - 1);
}

AMazeTile* AMazeCreator::GetMazeTileByIndex(int x, int y) {
  if (x < 0 || y < 0 || x >= this->width || y >= this->height) {
    return NULL;
  }
  return this->mazeTiles[getMultiDimensionalIndexInSingleDimensionArray(x, y, this->width)];
}

#pragma optimize("", on)