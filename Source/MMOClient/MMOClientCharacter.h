// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MMOClientCharacter.generated.h"

UCLASS(config=Game)
class AMMOClientCharacter : public ACharacter
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AMMOClientCharacter();

	// 공격
	UFUNCTION(BlueprintCallable)
		void SetAttack(bool flag);
	UFUNCTION(BlueprintCallable)
		bool GetAttack();
	// 공격 모션
	UFUNCTION(BlueprintImplementableEvent)
		void PlayAttackAnim();

	// 피격
	UFUNCTION(BlueprintCallable)
		void SetAttacked(bool flag);
	UFUNCTION(BlueprintCallable)
		bool GetAttacked();
	// 피격 모션
	UFUNCTION(BlueprintImplementableEvent)
		void PlayAttackedAnim();

	// HP
	float GetMaxHP();
	float GetHP();
	void SetHP(float hp);
	
	float MAXHP = 100.f;
	float _hp = 100.f;

	// 
	void DoAttack();
	void DoAttacked(int hp);

protected:
	// 액션 매핑
	void Attack();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
protected:
	void OnResetVR();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:
	uint64 _playerIndex = -1;
	bool _isAttack = false;
	bool _isAttacked = false;
};

