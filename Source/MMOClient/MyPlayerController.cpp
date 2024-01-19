// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include"MMOClientCharacter.h"
#include"MyGameInstance.h"
#include"ServerPacketHandler.h"
#include"NetSession.h"
#include"MyBuffer.h"
#include"ThreadHandler.h"
#include"proto/Protocol3.pb.h"
#include"MyMonster.h"

#include "GameFramework/CharacterMovementComponent.h"
#include<Kismet/KismetMathLibrary.h>
#include<Kismet/GameplayStatics.h>
#include<GameFramework/CharacterMovementComponent.h>

/*------------------------------------------------------------
	ActorInfo
-------------------------------------------------------------*/
#pragma region ActorInfo
void ActorInfo::SetIndex(uint64 playerIndex)
{
	_index = playerIndex;
}

void ActorInfo::SetActorType(ActorType type)
{
	_actorType = type;
}

void ActorInfo::SetLocation(FVector location)
{
	_location.X = location.X;
	_location.Y = location.Y;
	_location.Z = location.Z;
}

void ActorInfo::SetLocation(float x, float y, float z)
{
	_location.X = x;
	_location.Y = y;
	_location.Z = z;
}

void ActorInfo::SetRotation(FRotator rotation)
{
	_rotation.Yaw = rotation.Yaw;
	_rotation.Pitch = rotation.Pitch;
	_rotation.Roll = rotation.Roll;
}

void ActorInfo::SetRotation(float yaw, float pitch, float roll)
{
	_rotation.Yaw = yaw;
	_rotation.Pitch = pitch;
	_rotation.Roll = roll;
}

void ActorInfo::SetVelocity(FVector velocity)
{
	_velocity.X = velocity.X;
	_velocity.Y = velocity.Y;
	_velocity.Z = velocity.Z;
}

void ActorInfo::SetVelocity(float x, float y, float z)
{
	_velocity.X = x;
	_velocity.Y = y;
	_velocity.Z = z;
}

void ActorInfo::SetHP(float hp)
{
	_hp = hp;
}

void ActorInfo::SetActor(ACharacter* character)
{
	_actor = character;
}

uint64 ActorInfo::GetIndex()
{
	return _index;
}

ActorType ActorInfo::GetActorType()
{
	return _actorType;
}

FVector ActorInfo::GetLocation()
{
	return _location;
}

FRotator ActorInfo::GetRotation()
{
	return _rotation;
}

FVector ActorInfo::GetVelocity()
{
	return _velocity;
}

float ActorInfo::GetHP()
{
	return _hp;
}

ACharacter* ActorInfo::GetActor()
{
	return _actor;
}

bool ActorInfo::IsActor()
{
	return IsValid(_actor);
}

#pragma endregion


/*----------------------------------------------------------
	PlayerInfo
----------------------------------------------------------*/
#pragma region PlayerInfo
void PlayerInfo::SetLogin(bool login) { _isLogin = login; }
bool PlayerInfo::IsLogin() { return _isLogin; }
#pragma endregion


/*---------------------------------------------------------------------------------------------
	MyPlayerController
---------------------------------------------------------------------------------------------*/
AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UMyHUDWidget> widgetAsset(TEXT("/Game/WB_MyHUDWidget"));
	if (widgetAsset.Succeeded()) {
		MyHUDWidgetClass = widgetAsset.Class;
	}

	UObject* cls = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/BP_Player.BP_Player'"));
	UBlueprint* bp = Cast<UBlueprint>(cls);
	_bpCharacter = (UClass*)bp->GeneratedClass;

	UObject* cls2 = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/BP_Monster.BP_Monster'"));
	UBlueprint* bp2 = Cast<UBlueprint>(cls2);
	_bpMonster = (UClass*)bp2->GeneratedClass;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(MyHUDWidgetClass)) {
		myHUDWidget = Cast<UMyHUDWidget>(CreateWidget(GetWorld(), MyHUDWidgetClass));
		if (IsValid(myHUDWidget)) {
			myHUDWidget->AddToViewport();
			myHUDWidget->SetHPBar(100.f);
		}
	}

	// instance
	_ownerInstance = Cast<UMyGameInstance>(GetGameInstance());
	_ownerInstance->_controller = this;

	// 게임에 접속하면 생성되는 캐릭터 인덱스를 내 플레이어 인덱스로 설정
	AMMOClientCharacter* myCharacter = Cast<AMMOClientCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	myCharacter->_playerIndex = _ownerInstance->_myPlayerIndex;

	PlayerInfo* myPlayerInfo = _ownerInstance->FindPlayer(_ownerInstance->_myPlayerIndex);
	myPlayerInfo->SetActor(myCharacter);
	myPlayerInfo->SetLogin(true);
	
	// TEST
	UE_LOG(LogTemp, Error, TEXT("My Index : %d"), _ownerInstance->_myPlayerIndex);

	// 캐릭터 목록 요청
	PROTOCOL::C_PLAYERLIST toPkt;
	auto sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(toPkt);
	_ownerInstance->_netSession->Send(sendBuffer);

	// 몬스터 목록 요청
	PROTOCOL::C_MONSTERLIST toPkt2;
	auto sendBuffer2 = _ownerInstance->_packetHandler->MakeSendBuffer(toPkt2);
	_ownerInstance->_netSession->Send(sendBuffer2);

	// 현재 Tick 막혀 있음
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	//// 0.2f 주기로 서버로 내 위치 정보를 보내서 동기화
	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPlayerController::UpdateMyPos, 0.2f, true);
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Spawn
	UpdateSpawn();
	
	// Attack
	UpdateAttack();

	// World Update(move)
	UpdateWorld(DeltaSeconds);
	
	// Chat Update
	UpdateChat();
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if(_timerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(_timerHandle);
	
	if(_ownerInstance->_netSession.IsValid())
		_ownerInstance->_netSession->Stop();
}

void AMyPlayerController::UpdateMyPos()
{
	// 내 캐릭터 찾고 정보 추출
	auto myPlayer = Cast<AMMOClientCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	uint64 playerIndex = myPlayer->_playerIndex;
	auto location = myPlayer->GetActorLocation();
	auto rotation = myPlayer->GetActorRotation();
	auto velocity = myPlayer->GetVelocity();

	// 패킷에 담고
	PROTOCOL::C_MOVE movePkt;
	movePkt.mutable_actor()->set_index(playerIndex);
	movePkt.mutable_actor()->set_actortype(PROTOCOL::PLAYER);
	movePkt.mutable_actor()->set_locationx(location.X);
	movePkt.mutable_actor()->set_locationy(location.Y);
	movePkt.mutable_actor()->set_locationz(location.Z);
	movePkt.mutable_actor()->set_rotationyaw(rotation.Yaw);
	movePkt.mutable_actor()->set_rotationpitch(rotation.Pitch);
	movePkt.mutable_actor()->set_rotationroll(rotation.Roll);
	movePkt.mutable_actor()->set_velocityx(velocity.X);
	movePkt.mutable_actor()->set_velocityy(velocity.Y);
	movePkt.mutable_actor()->set_velocityz(velocity.Z);

	// Send
	TSharedPtr<MySendBuffer> sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(movePkt);
	_ownerInstance->_netSession->Send(sendBuffer);
}

void AMyPlayerController::StartGame()
{
	_tickFlag = true;
}

void AMyPlayerController::MyPlayerAttack(TArray<ActorInfo>& playerArray)
{
	PROTOCOL::C_ATTACK pkt;
	
	// Attacker
	pkt.mutable_attacker()->set_index(_ownerInstance->_myPlayerIndex);
	pkt.mutable_attacker()->set_actortype(PROTOCOL::PLAYER);

	// Victims
	for (int i = 0; i < playerArray.Num(); i++) {
		PROTOCOL::ACTOR* protoVictim = pkt.add_victims();
		protoVictim->set_index(playerArray[i].GetIndex());
		
		if (playerArray[i].GetActorType() == ActorType::PLAYER)
			protoVictim->set_actortype(PROTOCOL::PLAYER);
		else
			protoVictim->set_actortype(PROTOCOL::MONSTER);
	}

	TSharedPtr<MySendBuffer> sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(pkt);
	_ownerInstance->_netSession->Send(sendBuffer);
}

void AMyPlayerController::ActorAttack(ActorInfo& attacker)
{
	UE_LOG(LogTemp, Error, TEXT("ActorAttack() called"));

	// Attacker = Player
	if (attacker.GetActorType() == ActorType::PLAYER) {
		PlayerInfo* playerInfo = _ownerInstance->FindPlayer(attacker.GetIndex());
		if (playerInfo->IsActor()) {
			AMMOClientCharacter* attackPlayer = Cast<AMMOClientCharacter>(playerInfo->GetActor());
			FRotator tmpRotation = attackPlayer->GetActorRotation();
			tmpRotation.Yaw = playerInfo->GetRotation().Yaw;
			attackPlayer->SetActorRotation(tmpRotation);
			attackPlayer->DoAttack();
		}
	}
	
	// Attacker = Monster
	else {
		MonsterInfo* monsterInfo = _ownerInstance->FindMonster(attacker.GetIndex());
		if (monsterInfo->IsActor()) {
			AMyMonster* attackMonster = Cast<AMyMonster>(monsterInfo->GetActor());
			attackMonster->DoAttack();
		}
	}
}

void AMyPlayerController::ActorAttacked(ActorInfo& victim)
{
	UE_LOG(LogTemp, Error, TEXT("ActorAttacked() called"));

	// Victim = Player
	if (victim.GetActorType() == ActorType::PLAYER) {
		PlayerInfo* playerInfo = _ownerInstance->FindPlayer(victim.GetIndex());
		if (playerInfo->IsActor()) {
			AMMOClientCharacter* victimCharacter = Cast<AMMOClientCharacter>(playerInfo->GetActor());
			playerInfo->SetHP(victim.GetHP());
			victimCharacter->DoAttacked(victim.GetHP());
		}
	}

	// Victim = Monster
	else {
		MonsterInfo* monsterInfo = _ownerInstance->FindMonster(victim.GetIndex());
		if (monsterInfo->IsActor()) {
			AMyMonster* victimMonster = Cast<AMyMonster>(monsterInfo->GetActor());
			monsterInfo->SetHP(victim.GetHP());
			victimMonster->DoAttacked(victim.GetHP());
		}
	}
}

void AMyPlayerController::MyPlayerChat(FString& chatMessage)
{
	PROTOCOL::C_CHAT pkt;
	pkt.set_text(TCHAR_TO_ANSI(*chatMessage));
	
	auto sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(pkt);
	_ownerInstance->_netSession->Send(sendBuffer);
}

/*---------------------------------------------------------------------------
		UpdateOutData
---------------------------------------------------------------------------*/
void AMyPlayerController::UpdateSpawnData(ActorInfo* actorInfo)
{
	// Player
	if (actorInfo->GetActorType() == ActorType::PLAYER) {
		PlayerInfo* playerInfo = _ownerInstance->FindPlayer(actorInfo->GetIndex());
		if (playerInfo && !playerInfo->IsActor()) 
			_spawnQueue.Enqueue(*actorInfo);
	}

	// Monster
	else {
		MonsterInfo* monsterInfo = _ownerInstance->FindMonster(actorInfo->GetIndex());
		if (monsterInfo && !monsterInfo->IsActor()) 
			_spawnQueue.Enqueue(*actorInfo);
	}
}

void AMyPlayerController::UpdateChatData(FString& chat)
{
	_chatQueue.Enqueue(chat);
}

void AMyPlayerController::UpdateAttackData(TArray<ActorInfo>& playerInfos)
{
	_attackQueue.Enqueue(playerInfos);
}

/*---------------------------------------------------------------------------
		UpdateInGame(In Tick Methods)
---------------------------------------------------------------------------*/
void AMyPlayerController::UpdateWorld(float DeltaSeconds)
{
	// 인게임 액터들
	TArray<AActor*> spawnedPlayerArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMMOClientCharacter::StaticClass(), spawnedPlayerArray);
	
	// 하나씩 가져와서
	for (auto& spawnedPlayer : spawnedPlayerArray) {
		AMMOClientCharacter* playerCharacter = Cast<AMMOClientCharacter>(spawnedPlayer);

		// 해당 액터가 데이터 상으로도 로그인 중인지
		PlayerInfo* playerInfo = _ownerInstance->FindPlayer(playerCharacter->_playerIndex);
		if (playerInfo && playerInfo->IsLogin()) {
			// 이동
			// 내 캐릭이면 패스, 남 캐릭이라면 이동
			if (playerCharacter->_playerIndex == _ownerInstance->_myPlayerIndex)
				continue;
			
			float speed = FVector::DotProduct(playerInfo->GetVelocity(), playerInfo->GetRotation().Vector());
			//worldChars->AddMovementInput(info->GetVelocity());
			
			UCharacterMovementComponent* movementComponent = Cast<UCharacterMovementComponent>(playerCharacter->GetMovementComponent());
			FVector Direction = (playerInfo->GetLocation() - playerCharacter->GetActorLocation()).GetSafeNormal();

			playerCharacter->AddMovementInput(Direction, speed);
			//movementComponent->MoveSmooth(Direction * speed, DeltaSeconds);

			playerCharacter->SetActorRotation(playerInfo->GetRotation());
			//FRotator rotation = FMath::RInterpTo(worldChars->GetActorRotation(), info->GetRotation(), DeltaSeconds, SmoothTargetViewRotationSpeed);
			//SetControlRotation(rotation);
		}
	}

	TArray<AActor*> spawnedMonsterArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyMonster::StaticClass(), spawnedMonsterArray);

	for (auto& spawnedMonster : spawnedMonsterArray) {
		AMyMonster* monsterCharacter = Cast<AMyMonster>(spawnedMonster);
		
		MonsterInfo* monsterInfo = _ownerInstance->FindMonster(monsterCharacter->_monsterIndex);
		
		if (monsterInfo && monsterInfo->IsActor()) {
			monsterCharacter->MoveToLocation(monsterInfo->GetLocation());
		}
	}
}

void AMyPlayerController::UpdateSpawn()
{
	// recv스레드에서 인포 정보를 스폰큐에 적재, 게임스레드에서 인포 정보 업데이트 후 스폰
	while (!_spawnQueue.IsEmpty()) {
		ActorInfo* actorInfo = _spawnQueue.Peek();
		
		if (actorInfo->GetActorType() == ActorType::PLAYER) {
			PlayerInfo tmpPlayerInfo;
			_spawnQueue.Dequeue(tmpPlayerInfo);

			// 플레이어 리스트의 정보 업데이트
			PlayerInfo* playerInfo = _ownerInstance->FindPlayer(tmpPlayerInfo.GetIndex());
			if (!playerInfo->IsActor()) {
				playerInfo->SetIndex(tmpPlayerInfo.GetIndex());
				playerInfo->SetLocation(tmpPlayerInfo.GetLocation());
				playerInfo->SetRotation(tmpPlayerInfo.GetRotation());
				playerInfo->SetHP(tmpPlayerInfo.GetHP());
			}
			
			// 
			AMMOClientCharacter* spawnChar = GetWorld()->SpawnActor<AMMOClientCharacter>(_bpCharacter, playerInfo->GetLocation(), playerInfo->GetRotation());
			spawnChar->SpawnDefaultController();
			spawnChar->_playerIndex = playerInfo->GetIndex();
			spawnChar->_hp = playerInfo->GetHP();

			playerInfo->SetLogin(true);
			playerInfo->SetActor(spawnChar);
		}
		else {
			MonsterInfo tmpMonsterInfo;
			_spawnQueue.Dequeue(tmpMonsterInfo);

			//
			MonsterInfo* monsterInfo = _ownerInstance->FindMonster(tmpMonsterInfo.GetIndex());
			if (!monsterInfo->IsActor()) {
				monsterInfo->SetIndex(tmpMonsterInfo.GetIndex());
				monsterInfo->SetLocation(tmpMonsterInfo.GetLocation());
				monsterInfo->SetRotation(tmpMonsterInfo.GetRotation());
				monsterInfo->SetHP(tmpMonsterInfo.GetHP());
			}

			// 몬스터 스폰
			AMyMonster* spawnMon = GetWorld()->SpawnActor<AMyMonster>(_bpMonster, monsterInfo->GetLocation(), monsterInfo->GetRotation());
			spawnMon->SpawnDefaultController();
			spawnMon->_monsterIndex = monsterInfo->GetIndex();
			spawnMon->_hp = monsterInfo->GetHP();

			monsterInfo->SetActor(spawnMon);
		}
	}
}

void AMyPlayerController::UpdateChat()
{
	while (!_chatQueue.IsEmpty()) {
		FString chatMessage;
		if (_chatQueue.Dequeue(chatMessage)) {
			myHUDWidget->AddChatMessage(chatMessage);
		}
	}
}

void AMyPlayerController::UpdateAttack()
{
	while (!_attackQueue.IsEmpty()) {
		TArray<ActorInfo> actorInfos;
		_attackQueue.Dequeue(actorInfos);

		// attacker - 공격자가 내가 아니여야 공격 로직을 처리(현방식: 선 모션 후 보고)
		// 공격자가 
		if(!(_ownerInstance->_myPlayerIndex == actorInfos[0].GetIndex() && actorInfos[0].GetActorType() == ActorType::PLAYER))
			ActorAttack(actorInfos[0]);
		
		// victims
		for (int i = 1; i < actorInfos.Num(); i++) {
			ActorAttacked(actorInfos[i]);
		}
	}
}


