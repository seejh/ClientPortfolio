// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include"MMOClientCharacter.h"
#include"MyHUDWidget.h"

#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/*-------------------------------------------------------------------------------
	PlayerInfo
-------------------------------------------------------------------------------*/
class PlayerInfo {
public:
	PlayerInfo() : _playerIndex(-1), _location(0.f, 0.f, 0.f), _rotation(0.f, 0.f, 0.f), _velocity(0.f, 0.f, 0.f), _isLogin(false), _hp(100), _ID("test") {

	}
	
	// Setter
	void SetPlayerIndex(uint64 playerIndex);
	void SetLocation(FVector location);
	void SetLocation(float x, float y, float z);
	void SetRotation(FRotator rotation);
	void SetRotation(float yaw, float pitch, float roll);
	void SetVelocity(FVector velocity);
	void SetVelocity(float x, float y, float z);
	void SetHP(float hp);
	void SetActor(AMMOClientCharacter* character);

	// Getter
	uint64 PlayerIndex();
	FVector GetLocation();
	FRotator GetRotation();
	FVector GetVelocity();
	float GetHP();
	AMMOClientCharacter* GetActor();

	void SetLogin(bool login);
	bool IsLogin();
public:
	// 인덱스
	uint64 _playerIndex;

	// 위치정보
	FVector _location;
	FRotator _rotation;
	FVector _velocity;

	// 부가 정보(hp, 아이디, 플래그)
	float _hp;
	FString _ID;
	bool _needSpawn = false;
	bool _isLogin = false;

	AMMOClientCharacter* _actor;
};


/*---------------------------------------------------------------------------------------------
	MyPlayerController
---------------------------------------------------------------------------------------------*/
class UMyGameInstance;

UCLASS()
class MMOCLIENT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMyPlayerController();

	/*---------------------------------------------------------------------------
		override
	---------------------------------------------------------------------------*/
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*---------------------------------------------------------------------------
		ETC
	---------------------------------------------------------------------------*/
	void UpdateMyPos();
	void StartGame();

	void MyPlayerAttack(TArray<PlayerInfo>& playerArray);
	void PlayerAttack(PlayerInfo& attacker);
	void PlayerAttacked(PlayerInfo& victim);

	void MyPlayerChat(FString& chatMessage);

	/*---------------------------------------------------------------------------
		UpdateOutData
	---------------------------------------------------------------------------*/
	//void WorldUpdateOutData(PlayerInfo& playerInfo);
	void SpawnUpdateOutData(PlayerInfo& playerInfo);
	void ChatUpdateOutData(FString& chat);
	void AttackUpdateOutData(TArray<PlayerInfo>& playerInfos);

	/*---------------------------------------------------------------------------
		UpdateInGame(In Tick Methods)
	---------------------------------------------------------------------------*/
	void WorldUpdateInGame(float DeltaSeconds);
	void SpawnUpdateInGame();
	void ChatUpdateInGame();
	void AttackUpdateInGame();

	/*---------------------------------------------------------------------------
		Public Members
	---------------------------------------------------------------------------*/
	// WBP type
	TSubclassOf<UUserWidget> MyHUDWidgetClass;
	// WBP object
	UMyHUDWidget* myHUDWidget;
	
	TSubclassOf<class UObject> _bpCharacter;
public:
	TQueue<FString, EQueueMode::Spsc> _chatQueue;
	TQueue<PlayerInfo, EQueueMode::Spsc> _spawnQueue;
	TQueue<TArray<PlayerInfo>> _attackQueue;

	bool _tickFlag = false;
	FTimerHandle _timerHandle;
	UMyGameInstance* _ownerInstance;
};
