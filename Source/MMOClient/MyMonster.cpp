// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMonster.h"

#include"MyAIController.h"

// Sets default values
AMyMonster::AMyMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AMyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMyMonster::MoveToLocation(FVector location)
{
	AMyAIController* controller = Cast<AMyAIController>(GetController());
	if (IsValid(controller)) {
		controller->MoveToLocation(location);
	}
}

void AMyMonster::DoAttack()
{
	PlayAttackAnim();
}

void AMyMonster::DoAttacked(int hp)
{
}

// Called when the game starts or when spawned
void AMyMonster::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

