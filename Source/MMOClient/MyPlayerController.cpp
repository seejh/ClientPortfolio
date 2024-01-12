// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include"MMOClientCharacter.h"
#include"MyGameInstance.h"
#include"ServerPacketHandler.h"
#include"NetSession.h"
#include"MyBuffer.h"
#include"ThreadHandler.h"
#include"proto/MyProtocol.pb.h"

#include "GameFramework/CharacterMovementComponent.h"
#include<Kismet/KismetMathLibrary.h>
#include<Kismet/GameplayStatics.h>

/*------------------------------------------------------------
	PlayerInfo
-------------------------------------------------------------*/
#pragma region PlayerInfo
void PlayerInfo::SetPlayerIndex(uint64 playerIndex)
{
	_playerIndex = playerIndex;
}

void PlayerInfo::SetLocation(FVector location)
{
	_location.X = location.X;
	_location.Y = location.Y;
	_location.Z = location.Z;
}

void PlayerInfo::SetLocation(float x, float y, float z)
{
	_location.X = x;
	_location.Y = y;
	_location.Z = z;
}

void PlayerInfo::SetRotation(FRotator rotation)
{
	_rotation.Yaw = rotation.Yaw;
	_rotation.Pitch = rotation.Pitch;
	_rotation.Roll = rotation.Roll;
}

void PlayerInfo::SetRotation(float yaw, float pitch, float roll)
{
	_rotation.Yaw = yaw;
	_rotation.Pitch = pitch;
	_rotation.Roll = roll;
}

void PlayerInfo::SetVelocity(FVector velocity)
{
	_velocity.X = velocity.X;
	_velocity.Y = velocity.Y;
	_velocity.Z = velocity.Z;
}

void PlayerInfo::SetVelocity(float x, float y, float z)
{
	_velocity.X = x;
	_velocity.Y = y;
	_velocity.Z = z;
}

void PlayerInfo::SetHP(float hp)
{
	_hp = hp;
}

void PlayerInfo::SetActor(AMMOClientCharacter* character)
{
	_actor = character;
}

uint64 PlayerInfo::PlayerIndex()
{
	return _playerIndex;
}

FVector PlayerInfo::GetLocation()
{
	return _location;
}

FRotator PlayerInfo::GetRotation()
{
	return _rotation;
}

FVector PlayerInfo::GetVelocity()
{
	return _velocity;
}

float PlayerInfo::GetHP()
{
	return _hp;
}

AMMOClientCharacter* PlayerInfo::GetActor()
{
	return _actor;
}

void PlayerInfo::SetLogin(bool login)
{
	_isLogin = login;
}

bool PlayerInfo::IsLogin()
{
	return _isLogin;
}

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

	// ���ӿ� �����ϸ� �����Ǵ� ĳ���� �ε����� �� �÷��̾� �ε����� ����
	AMMOClientCharacter* myCharacter = Cast<AMMOClientCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	myCharacter->_playerIndex = _ownerInstance->_myPlayerIndex;
	PlayerInfo* myPlayerInfo = _ownerInstance->_intPlayerInfoMap.Find(_ownerInstance->_myPlayerIndex);
	myPlayerInfo->SetActor(myCharacter);
	myPlayerInfo->SetLogin(true);

	// TEST
	UE_LOG(LogTemp, Error, TEXT("My Index : %d"), _ownerInstance->_myPlayerIndex);

	// ĳ���� ��� ��û
	PROTOCOL::C_PLAYERLIST toPkt;
	auto sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(toPkt);
	_ownerInstance->_netSession->Send(sendBuffer);

	// ���� Tick ���� ����
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	//// 0.2f �ֱ�� ������ �� ��ġ ������ ������ ����ȭ
	GetWorldTimerManager().SetTimer(_timerHandle, this, &AMyPlayerController::UpdateMyPos, 0.2f, true);
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Spawn
	SpawnUpdateInGame();
	
	// Attack
	AttackUpdateInGame();

	// World Update(move)
	WorldUpdateInGame(DeltaSeconds);
	
	// Chat Update
	ChatUpdateInGame();
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
	// �� ĳ���� ã�� ���� ����
	auto myPlayer = Cast<AMMOClientCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	uint64 playerIndex = myPlayer->_playerIndex;
	auto location = myPlayer->GetActorLocation();
	auto rotation = myPlayer->GetActorRotation();
	auto velocity = myPlayer->GetVelocity();

	// ��Ŷ�� ���
	PROTOCOL::C_MOVE movePkt;
	movePkt.mutable_player()->set_playerid(playerIndex);
	movePkt.mutable_player()->set_locationx(location.X);
	movePkt.mutable_player()->set_locationy(location.Y);
	movePkt.mutable_player()->set_locationz(location.Z);
	movePkt.mutable_player()->set_rotationyaw(rotation.Yaw);
	movePkt.mutable_player()->set_rotationpitch(rotation.Pitch);
	movePkt.mutable_player()->set_rotationroll(rotation.Roll);
	movePkt.mutable_player()->set_velocityx(velocity.X);
	movePkt.mutable_player()->set_velocityy(velocity.Y);
	movePkt.mutable_player()->set_velocityz(velocity.Z);

	// Send
	TSharedPtr<MySendBuffer> sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(movePkt);
	_ownerInstance->_netSession->Send(sendBuffer);
}

void AMyPlayerController::StartGame()
{
	_tickFlag = true;
}

void AMyPlayerController::MyPlayerAttack(TArray<PlayerInfo>& playerArray)
{
	PROTOCOL::C_ATTACK pkt;
	
	// attacker
	pkt.mutable_attacker()->set_playerid(_ownerInstance->_myPlayerIndex);

	// victims
	for (int i = 0; i < playerArray.Num(); i++) {
		PROTOCOL::PLAYER* victim = pkt.add_victims();
		victim->set_playerid(playerArray[i].PlayerIndex());
	}

	TSharedPtr<MySendBuffer> sendBuffer = _ownerInstance->_packetHandler->MakeSendBuffer(pkt);
	_ownerInstance->_netSession->Send(sendBuffer);
}

void AMyPlayerController::PlayerAttack(PlayerInfo& attacker)
{
	UE_LOG(LogTemp, Error, TEXT("PlayerAttack() called"));
	PlayerInfo* playerInfo = _ownerInstance->_intPlayerInfoMap.Find(attacker.PlayerIndex());
	if (playerInfo->IsLogin()) {
		AMMOClientCharacter* attackActor = playerInfo->GetActor();
		if (IsValid(attackActor))
			attackActor->DoAttack();
	}
}

void AMyPlayerController::PlayerAttacked(PlayerInfo& victim)
{
	UE_LOG(LogTemp, Error, TEXT("PlayerAttacked() called"));
	PlayerInfo* playerInfo = _ownerInstance->_intPlayerInfoMap.Find(victim.PlayerIndex());
	if (playerInfo->IsLogin()) {
		AMMOClientCharacter* victimActor = playerInfo->GetActor();
		if (IsValid(victimActor)) {
			playerInfo->SetHP(victim.GetHP());
			victimActor->DoAttacked(victim.GetHP());
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
void AMyPlayerController::SpawnUpdateOutData(PlayerInfo& playerInfo)
{
	// TEST
	UE_LOG(LogTemp, Error, TEXT("SpawnUpdateOutData() - %d"), playerInfo.PlayerIndex());

	PlayerInfo* info = _ownerInstance->_intPlayerInfoMap.Find(playerInfo._playerIndex);
	if (info->IsLogin()) {
		// TODO : Error ���� ���Ѿ� �ϴµ� ������ �Ǿ� ����, ������ ����
		UE_LOG(LogTemp, Error, TEXT("Logined"));
		return;
	}

	_spawnQueue.Enqueue(playerInfo);
}

void AMyPlayerController::ChatUpdateOutData(FString& chat)
{
	_chatQueue.Enqueue(chat);
}

void AMyPlayerController::AttackUpdateOutData(TArray<PlayerInfo>& playerInfos)
{
	_attackQueue.Enqueue(playerInfos);
}

/*---------------------------------------------------------------------------
		UpdateInGame(In Tick Methods)
---------------------------------------------------------------------------*/
void AMyPlayerController::WorldUpdateInGame(float DeltaSeconds)
{
	TArray<AActor*> spawnedChars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMMOClientCharacter::StaticClass(), spawnedChars);
	
	// �ΰ��� ���͵��� ������
	for (auto& player : spawnedChars) {
		AMMOClientCharacter* worldChars = Cast<AMMOClientCharacter>(player);

		// �ش� ���Ͱ� ������ �����ε� �α��� ������
		PlayerInfo* info = _ownerInstance->_intPlayerInfoMap.Find(worldChars->_playerIndex);
		if (info->IsLogin()) {
			// �̵�
			// �� ĳ���̸� �н�, �� ĳ���̶�� �̵�
			if (worldChars->_playerIndex == _ownerInstance->_myPlayerIndex)
				continue;

			worldChars->AddMovementInput(info->GetVelocity());
		}
	}
}

void AMyPlayerController::SpawnUpdateInGame()
{
	// recv�����忡�� ���� ������ ����ť�� ����, ���ӽ����忡�� ���� ���� ������Ʈ �� ����
	while (!_spawnQueue.IsEmpty()) {
		// ����ť���� ���� ����
		PlayerInfo tmpInfo;
		_spawnQueue.Dequeue(tmpInfo);

		// �ܺ� ������ ������Ʈ1
		PlayerInfo* mapInfo = _ownerInstance->_intPlayerInfoMap.Find(tmpInfo.PlayerIndex());
		mapInfo->_playerIndex = tmpInfo._playerIndex;
		mapInfo->SetLocation(tmpInfo.GetLocation());
		mapInfo->SetRotation(tmpInfo.GetRotation());
		mapInfo->SetHP(tmpInfo.GetHP());
		// TODO : ID set

		// �ΰ��� ���� ������Ʈ
		AMMOClientCharacter* spawnChar = GetWorld()->SpawnActor<AMMOClientCharacter>(_bpCharacter, mapInfo->GetLocation(), mapInfo->GetRotation());
		spawnChar->SpawnDefaultController();
		spawnChar->_playerIndex = mapInfo->_playerIndex;
		spawnChar->_hp = mapInfo->_hp;
		// TODO : ID set

		// �ܺ� ������ ������Ʈ2(�α���, �ΰ��� ���� ������)
		mapInfo->SetLogin(true);
		mapInfo->SetActor(spawnChar);
	}
}

void AMyPlayerController::ChatUpdateInGame()
{
	while (!_chatQueue.IsEmpty()) {
		FString chatMessage;
		if (_chatQueue.Dequeue(chatMessage)) {
			myHUDWidget->AddChatMessage(chatMessage);
		}
	}
}

void AMyPlayerController::AttackUpdateInGame()
{
	while (!_attackQueue.IsEmpty()) {
		TArray<PlayerInfo> players;
		_attackQueue.Dequeue(players);

		// attacker - �����ڰ� ���� �ƴϿ��� ���� ������ ó��(�����: �� ��� �� ����)
		if(_ownerInstance->_myPlayerIndex != players[0].PlayerIndex())
			PlayerAttack(players[0]);
		
		// victims
		for (int i = 1; i < players.Num(); i++) {
			PlayerAttacked(players[i]);
		}
	}
}
