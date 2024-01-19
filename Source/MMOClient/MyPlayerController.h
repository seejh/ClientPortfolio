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
enum ActorType { PLAYER, MONSTER };

class ActorInfo {
public:
	ActorInfo() : _actorType(ActorType::PLAYER), _index(-1), _location(0.f, 0.f, 0.f), _rotation(0.f, 0.f, 0.f), _velocity(0.f, 0.f, 0.f), _isLogin(false), _hp(100), _ID("test"), _actor(nullptr) {}
	ActorInfo(ActorType type) : _actorType(type), _index(-1), _location(0.f, 0.f, 0.f), _rotation(0.f, 0.f, 0.f), _velocity(0.f, 0.f, 0.f), _isLogin(false), _hp(100), _ID("test"), _actor(nullptr) {}

	// Setter
	void SetIndex(uint64 playerIndex);
	void SetActorType(ActorType type);
	void SetLocation(FVector location);
	void SetLocation(float x, float y, float z);
	void SetRotation(FRotator rotation);
	void SetRotation(float yaw, float pitch, float roll);
	void SetVelocity(FVector velocity);
	void SetVelocity(float x, float y, float z);
	void SetHP(float hp);
	void SetActor(ACharacter* character);

	// Getter
	uint64 GetIndex();
	ActorType GetActorType();
	FVector GetLocation();
	FRotator GetRotation();
	FVector GetVelocity();
	float GetHP();
	ACharacter* GetActor();

	bool IsActor();
public:
	// 인덱스
	uint64 _index;
	ActorType _actorType;

	// 위치정보
	FVector _location;
	FRotator _rotation;
	FVector _velocity;

	// 부가 정보(hp, 아이디, 플래그)
	float _hp;
	FString _ID;
	bool _needSpawn = false;
	bool _isLogin = false;

	ACharacter* _actor = nullptr;
};

class PlayerInfo : public ActorInfo {
public:
	PlayerInfo() : ActorInfo(ActorType::PLAYER) {}

	void SetLogin(bool login);
	bool IsLogin();
};

class MonsterInfo : public ActorInfo {
public:
	MonsterInfo() : ActorInfo(ActorType::MONSTER) {}
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

	void MyPlayerAttack(TArray<ActorInfo>& playerArray);
	void ActorAttack(ActorInfo& attacker);
	void ActorAttacked(ActorInfo& victim);

	void MyPlayerChat(FString& chatMessage);

	/*---------------------------------------------------------------------------
		UpdateOutData
	---------------------------------------------------------------------------*/
	//void WorldUpdateOutData(PlayerInfo& playerInfo);
	void UpdateSpawnData(ActorInfo* playerInfo);
	void UpdateChatData(FString& chat);
	void UpdateAttackData(TArray<ActorInfo>& playerInfos);

	/*---------------------------------------------------------------------------
		UpdateInGame(In Tick Methods)
	---------------------------------------------------------------------------*/
	void UpdateWorld(float DeltaSeconds);
	void UpdateSpawn();
	void UpdateChat();
	void UpdateAttack();

	/*---------------------------------------------------------------------------
		Public Members
	---------------------------------------------------------------------------*/
	// WBP type
	TSubclassOf<UUserWidget> MyHUDWidgetClass;
	// WBP object
	UMyHUDWidget* myHUDWidget;
	
	TSubclassOf<class UObject> _bpCharacter;
	TSubclassOf<class UObject> _bpMonster;
public:
	TQueue<FString, EQueueMode::Spsc> _chatQueue;
	TQueue<ActorInfo, EQueueMode::Spsc> _spawnQueue;
	TQueue<TArray<ActorInfo>> _attackQueue;

	bool _tickFlag = false;
	FTimerHandle _timerHandle;
	UMyGameInstance* _ownerInstance;
};
