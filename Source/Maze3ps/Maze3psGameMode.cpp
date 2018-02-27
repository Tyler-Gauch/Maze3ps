// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Maze3psGameMode.h"
#include "Maze3psCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMaze3psGameMode::AMaze3psGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/MainCharacterTesting"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
