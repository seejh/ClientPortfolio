// Copyright Epic Games, Inc. All Rights Reserved.

#include "MMOClientGameMode.h"
#include "MMOClientCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMMOClientGameMode::AMMOClientGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode BP_Player not null"));
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
