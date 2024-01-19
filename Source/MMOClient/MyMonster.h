// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyMonster.generated.h"

UCLASS()
class MMOCLIENT_API AMyMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyMonster();
	void MoveToLocation(FVector location);

	uint64 _monsterIndex = 0;
	float _hp;

	UFUNCTION(BlueprintImplementableEvent)
		void PlayAttackAnim();

	void DoAttack();
	void DoAttacked(int hp);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
