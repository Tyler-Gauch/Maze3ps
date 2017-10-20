// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeCreator.h"
#include <stack>
#include "UnrealMathUtility.h"
#include <string>
#include "Kismet/KismetSystemLibrary.h"
#include "MazeTileAnimInstance.h"
#include "EngineUtils.h"
#pragma optimize("", off)

int getMultiDimensionalIndexInSingleDimensionArray(int x, int y, int width) {
  return x + width * y;
}

// Sets default values
AMazeCreator::AMazeCreator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  this->tiles = new Tile *[this->height * this->width];
  this->mazeTiles = new AMazeTile *[this->height * this->width];
}

AMazeCreator::~AMazeCreator() {
  delete[] this->tiles;
  delete[] this->mazeTiles;
}

// Called when the game starts or when spawned
void AMazeCreator::BeginPlay() {
  Super::BeginPlay();

  // find our player
  for (TActorIterator<AMaze3psCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
    AMaze3psCharacter* character = *ActorItr;
    this->character = character;
  }

  this->generateInitialTiles();
  this->generate();

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
      Tile * tile = this->getTileAtIndex(x, y);

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

  this->InitialStart = false;
}

// Called every frame
void AMazeCreator::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  this->timeSinceLastMazeCreate += DeltaTime;
  this->timePassed += DeltaTime;

  if (this->timeSinceLastMazeCreate > 10.0f) {
    this->generateInitialTiles();
    this->generate();
  }


  this->renderMaze(DeltaTime);
  
}

void AMazeCreator::generate() {
  // used to keep track of current places we have gone
  // so we can backtrack
  std::stack<Tile*> path;
  int x = 1, y = 1;
  bool hasUnvisted = true;
  needsMazeRefresh = true;
  this->timeSinceLastMazeCreate = 0;

  Tile * startTile = this->getTileAtIndex(x, y);

  // we use 2 because forward 1 space is a wall!
  Tile * currentTile = startTile;
  Tile * nextTile = NULL;

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

      if (nextTile == NULL) { // tile doesn't exist
        currentDirection++;
      } else if (nextTile->visited) { // tile is already visited
        nextTile = NULL;
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

    if (nextTile != NULL) { // if we had a tile move to that tile
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
      nextTile = NULL;
    } else if (!path.empty()) {
      currentTile = path.top();
      path.pop();
    } else {
      hasUnvisted = false;
    }
  }
}

void AMazeCreator::renderMaze(float DeltaTime) {

  if (this->character == nullptr) {
    //UE_LOG(LogTemp, Error, TEXT("Character was null unable to render maze!"));
    return;
  }
  FVector2D* startTile = this->FindMazeTileIndexByLocation(this->character->GetActorLocation());
  UWorld* world = GetWorld();

  // we want to get a radius of 5 tiles around the player to render
  // but we want to remove any tile within 10-5 spaces away
  int startX = FMath::Clamp((int)startTile->X - this->RenderRadius, 0, this->width - 1);
  int startY = FMath::Clamp((int)startTile->Y - this->RenderRadius, 0, this->height - 1);
  int endX = FMath::Clamp((int)startTile->X + this->RenderRadius, 0, this->width - 1);
  int endY = FMath::Clamp((int)startTile->Y + this->RenderRadius, 0, this->height - 1);
  int cleanUpStartX = FMath::Clamp((int)startX - this->RenderRadius, 0, this->width - 1);
  int cleanUpStartY = FMath::Clamp((int)startY - this->RenderRadius, 0, this->height - 1);
  int cleanUpEndX = FMath::Clamp((int)endX + this->RenderRadius, 0, this->width - 1);
  int cleanUpEndY = FMath::Clamp((int)endY + this->RenderRadius, 0, this->height - 1);

  /*UE_LOG(
    LogTemp,
    Warning,
    TEXT("(%d, %d), (%d, %d), (%d, %d), (%d, %d), (%d, %d)"),
    (int)startTile->X, (int)startTile->Y,
    startX, endX,
    startY, endY,
    cleanUpStartX, cleanUpEndX,
    cleanUpStartY, cleanUpEndY
  );*/

  FVector location(0,0,0);
  for (int y = cleanUpStartY; y <= cleanUpEndY; y++) {
    for (int x = cleanUpStartX; x < cleanUpEndX; x++) {
      int currentTileIndex = this->getTileIndex(x, y);
      location.X = this->TileWidth * x; //todo use box extents figure it out
      location.Y = this->TileWidth * y;
      AMazeTile* mazeTile = this->mazeTiles[currentTileIndex];

      if (mazeTile != NULL) {

        if (x < startX || x > endX || y < startY || y > endY) {
          mazeTile->Destroy();
          this->mazeTiles[currentTileIndex] = NULL;
          continue;
        }

        if (!this->needsMazeRefresh) {
          continue;
        }
      } else if (mazeTile == NULL && (x >= startX && x <= endX && y >= startY && y <= endY)) {
        mazeTile = (AMazeTile*)world->SpawnActor(MazeTileBP, &location);
      } else {
        continue;
      }

      UMazeTileAnimInstance * animInstance = Cast<UMazeTileAnimInstance>(mazeTile->tileMesh->GetAnimInstance());
      if (animInstance) {
        Tile * tileData = this->getTileAtIndex(x, y);
        // determine the state of the tile

        if (!tileData->hasNorthWall && tileData->hasEastWall && tileData->hasWestWall && tileData->hasSouthWall) {
          animInstance->TileType = DEADEND_NORTH;
        } else if (!tileData->hasSouthWall && tileData->hasEastWall && tileData->hasWestWall && tileData->hasNorthWall) {
          animInstance->TileType = DEADEND_SOUTH;
        } else if (!tileData->hasEastWall && tileData->hasSouthWall && tileData->hasWestWall && tileData->hasNorthWall) {
          animInstance->TileType = DEADEND_EAST;
        } else if (!tileData->hasWestWall && tileData->hasSouthWall && tileData->hasEastWall && tileData->hasNorthWall) {
          animInstance->TileType = DEADEND_WEST;
        } else if (!tileData->hasEastWall && !tileData->hasWestWall && tileData->hasNorthWall && tileData->hasSouthWall) {
          animInstance->TileType = HALLWAY_EAST_WEST;
        } else if (tileData->hasEastWall && tileData->hasWestWall && !tileData->hasNorthWall && !tileData->hasSouthWall) {
          animInstance->TileType = HALLWAY_NORTH_SOUTH;
        } else if (tileData->hasSouthWall && !tileData->hasEastWall && !tileData->hasWestWall && !tileData->hasNorthWall) {
          animInstance->TileType = T_NORTH;
        } else if (!tileData->hasSouthWall && !tileData->hasEastWall && !tileData->hasWestWall && tileData->hasNorthWall) {
          animInstance->TileType = T_SOUTH;
        } else if (!tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall && !tileData->hasNorthWall) {
          animInstance->TileType = T_EAST;
        } else if (!tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall && !tileData->hasNorthWall) {
          animInstance->TileType = T_WEST;
        } else if (tileData->hasNorthWall && !tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall) {
          animInstance->TileType = CORNER_EAST_SOUTH;
        } else if (!tileData->hasNorthWall && tileData->hasSouthWall && !tileData->hasEastWall && tileData->hasWestWall) {
          animInstance->TileType = CORNER_NORTH_EAST;
        } else if (tileData->hasNorthWall && !tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall) {
          animInstance->TileType = CORNER_SOUTH_WEST;
        } else if (!tileData->hasNorthWall && tileData->hasSouthWall && tileData->hasEastWall && !tileData->hasWestWall) {
          animInstance->TileType = CORNER_WEST_NORTH;
        } else {
          animInstance->TileType = CROSS;
        }
      } else {
        UE_LOG(LogTemp, Error, TEXT("AnimInstance was not available"));
      }
      this->mazeTiles[this->getTileIndex(x, y)] = mazeTile;
    }
  }

  this->needsMazeRefresh = false;
}

void AMazeCreator::generateInitialTiles() {
  for (int y = 0; y < this->height; y++) {
    for (int x = 0; x < this->width; x++) {
      Tile * tile = new Tile;
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

Tile* AMazeCreator::getNorthTile(Tile * currentTile) {
  return this->getTileAtIndex(currentTile->x, currentTile->y - 1);
}

Tile* AMazeCreator::getSouthTile(Tile * currentTile) {
  return this->getTileAtIndex(currentTile->x, currentTile->y + 1);
}

Tile* AMazeCreator::getEastTile(Tile * currentTile) {
  return this->getTileAtIndex(currentTile->x + 1, currentTile->y);
}

Tile* AMazeCreator::getWestTile(Tile * currentTile) {
  // first check if we have a west tile
  return this->getTileAtIndex(currentTile->x - 1, currentTile->y);
}

Tile* AMazeCreator::getTileAtIndex(int x, int y) {
  // make sure the tile we want exists. The getMultiDimensionalIndexInSingleDimensionArray will
  // wrap rows if we ask for an east tile and there isn't one or vice versa.
  if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
    return NULL;
  }
  
  // lets make sure this tile is within our bounds
  int index = this->getTileIndex(x, y);
  if (index < 0 || index >= this->width * this->height) {
    return NULL;
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