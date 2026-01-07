// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIStudyGameMode.h"
#include "AIStudyCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAIStudyGameMode::AAIStudyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
