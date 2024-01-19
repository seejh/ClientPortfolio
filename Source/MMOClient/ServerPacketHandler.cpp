// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerPacketHandler.h"

#include"MyPlayerController.h"
#include"MyGameInstance.h"

/*---------------------------------------------------------------------------------------------
	Handle Funcs
---------------------------------------------------------------------------------------------*/
bool Handle_INVALID(UMyGameInstance* instance, uint8* buffer, int len)
{
	return true;
}

bool Handle_S_LOGIN(UMyGameInstance* instance, PROTOCOL::S_LOGIN fromPkt)
{
	// login 성공이면
	if (fromPkt.success()) {
		UE_LOG(LogTemp, Error, TEXT("Login to Server OK"));

		// 내 플레이어 인덱스 설정
		instance->_myPlayerIndex = fromPkt.actor().index();
		
		// EnterRoom 요청
		PROTOCOL::C_ENTER_ROOM toPkt;
		toPkt.set_roomnum(1);

		// send
		TSharedPtr<MySendBuffer> sendBuffer = instance->_packetHandler->MakeSendBuffer(toPkt);
		instance->_netSession->Send(sendBuffer);
	}

	return true;
}

bool Handle_S_ENTER_ROOM(UMyGameInstance* instance, PROTOCOL::S_ENTER_ROOM fromPkt)
{
	// EnterRoom 성공이면
	if (fromPkt.success()) {
		
		// 접속한 것이 본인
		if (fromPkt.actor().index() == instance->_myPlayerIndex) {
			// 다음 맵으로 이동 (이동하면 PC가동)
			instance->_loginWidget->GoNextLevel();
		}

		// 접속한 것이 타인 또는 타몬스터
		else {
			// 접속한 플레이어의 정보 추출
			PROTOCOL::ACTOR protoActorInfo = fromPkt.actor();

			// 플레이어
			if (protoActorInfo.actortype() == PROTOCOL::PLAYER) {
				// 플레이어 인포 정의
				PlayerInfo playerInfo;
				playerInfo.SetIndex(protoActorInfo.index());
				playerInfo.SetLocation(protoActorInfo.locationx(), protoActorInfo.locationy(), protoActorInfo.locationz());
				playerInfo.SetRotation(protoActorInfo.rotationyaw(), protoActorInfo.rotationpitch(), protoActorInfo.rotationroll());
				playerInfo.SetHP(protoActorInfo.hp());
				// TODO : set ID

				// 업데이트 스폰 데이터
				instance->_controller->UpdateSpawnData(&playerInfo);
			}

			// 몬스터
			else {
				MonsterInfo monsterInfo;
				monsterInfo.SetIndex(protoActorInfo.index());
				monsterInfo.SetLocation(protoActorInfo.locationx(), protoActorInfo.locationy(), protoActorInfo.locationz());
				monsterInfo.SetRotation(protoActorInfo.rotationyaw(), protoActorInfo.rotationpitch(), protoActorInfo.rotationroll());
				monsterInfo.SetHP(protoActorInfo.hp());

				instance->_controller->UpdateSpawnData(&monsterInfo);
			}
		}
	}

	return true;
}

// 처음 접속할 때 받는 패킷
// 접속한 플레이어 명단을 받으며 나의 정보도 포함, 걸러내야됨
bool Handle_S_PLAYERLIST(UMyGameInstance* instance, PROTOCOL::S_PLAYERLIST fromPkt)
{
	// 목록에서 하나씩
	for (int i = 0; i < fromPkt.actors_size(); i++) {
		// 플레이어 정보 추출
		PROTOCOL::ACTOR protoPlayerInfo = fromPkt.actors(i);
		
		// 추출된 게 자기라면 pass
		if (protoPlayerInfo.index() == instance->_myPlayerIndex) {
			continue;
		}
		
		// 추출된 것이 남이라면
		else {
			// 플레이어 인포 정의
			PlayerInfo playerInfo;
			playerInfo.SetIndex(protoPlayerInfo.index());
			playerInfo.SetLocation(protoPlayerInfo.locationx(), protoPlayerInfo.locationy(), protoPlayerInfo.locationz());
			playerInfo.SetRotation(protoPlayerInfo.rotationyaw(), protoPlayerInfo.rotationpitch(), protoPlayerInfo.rotationroll());
			playerInfo.SetHP(protoPlayerInfo.hp());
			// TODO : set ID

			// 업데이트 스폰 데이터
			instance->_controller->UpdateSpawnData(&playerInfo);
		}
	}

	return true;
}

bool Handle_S_MONSTERLIST(UMyGameInstance* instance, PROTOCOL::S_MONSTERLIST fromPkt)
{
	for (int i = 0; i < fromPkt.actors_size(); i++) {
		PROTOCOL::ACTOR protoMonsterInfo = fromPkt.actors(i);

		MonsterInfo monsterInfo;
		monsterInfo.SetIndex(protoMonsterInfo.index());
		monsterInfo.SetLocation(protoMonsterInfo.locationx(), protoMonsterInfo.locationy(), protoMonsterInfo.locationz());
		monsterInfo.SetRotation(protoMonsterInfo.rotationyaw(), protoMonsterInfo.rotationpitch(), protoMonsterInfo.rotationroll());
		monsterInfo.SetHP(protoMonsterInfo.hp());

		instance->_controller->UpdateSpawnData(&monsterInfo);
	}

	return true;
}

bool Handle_S_MOVE(UMyGameInstance* instance, PROTOCOL::S_MOVE fromPkt)
{
	// 추출
	PROTOCOL::ACTOR protoActor = fromPkt.actor();
	
	// PLAYER
	if (protoActor.actortype() == PROTOCOL::PLAYER) {
		PlayerInfo* playerInfo = instance->FindPlayer(protoActor.index());
		if (playerInfo) {
			playerInfo->SetLocation(protoActor.locationx(), protoActor.locationy(), protoActor.locationz());
			playerInfo->SetRotation(protoActor.rotationyaw(), protoActor.rotationpitch(), protoActor.rotationroll());
			playerInfo->SetVelocity(protoActor.velocityx(), protoActor.velocityy(), protoActor.velocityz());
		}
	}

	// MONSTER
	else {
		MonsterInfo* monsterInfo = instance->FindMonster(protoActor.index());
		if (monsterInfo) {
			monsterInfo->SetLocation(protoActor.locationx(), protoActor.locationy(), protoActor.locationz());
		}
	}
	
	return true;
}

// 나의 공격도 서버로 부터 응답받고 처리하는 방식
bool Handle_S_ATTACK(UMyGameInstance* instance, PROTOCOL::S_ATTACK fromPkt)
{
	UE_LOG(LogTemp, Error, TEXT("Handle_S_ATTACK()"));
	TArray<ActorInfo> playerInfoArray;
	
	// 공격자
	PROTOCOL::ACTOR protoAttacker = fromPkt.attacker();
	
	// 공격자가 플레이어
	if (protoAttacker.actortype() == PROTOCOL::PLAYER) {
		PlayerInfo info;
		info.SetIndex(protoAttacker.index());
		info._rotation.Yaw = protoAttacker.rotationyaw();
		playerInfoArray.Add(info);
	}
	else {
		MonsterInfo info;
		info.SetIndex(protoAttacker.index());
		info._rotation.Yaw = protoAttacker.rotationyaw();
		playerInfoArray.Add(info);
	}

	// 피격자
	for (int i = 0; i < fromPkt.victims_size(); i++) {
		PROTOCOL::ACTOR protoVictim = fromPkt.victims(i);
		if (protoVictim.actortype() == PROTOCOL::PLAYER) {
			PlayerInfo info;
			info.SetIndex(protoVictim.index());
			info.SetHP(protoVictim.hp());
			playerInfoArray.Add(info);
		}
		else {
			MonsterInfo info;
			info.SetIndex(protoVictim.index());
			info.SetHP(protoVictim.hp());
			playerInfoArray.Add(info);
		}
	}

	// 
	instance->_controller->UpdateAttackData(playerInfoArray);

	return true;
}

bool Handle_S_CHAT(UMyGameInstance* instance, PROTOCOL::S_CHAT fromPkt)
{	
	// PLAYER-1:채팅 내용
	FString chat("Player-");
	chat.Append(FString::FromInt(fromPkt.actor().index()));
	chat.Append(": ");
	chat.Append(fromPkt.text().c_str());

	instance->_controller->UpdateChatData(chat);

	return true;
}

/*----------------------------------
	ServerPacketHandler
-----------------------------------*/
void ServerPacketHandler::Init()
{
	for (int i = 0; i < PacketCount; i++)
		_packetHandleFuncs[i] = Handle_INVALID;

	_packetHandleFuncs[S_LOGIN] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_LOGIN>(Handle_S_LOGIN, instance, buffer, len);
	};
	_packetHandleFuncs[S_ENTER_ROOM] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_ENTER_ROOM>(Handle_S_ENTER_ROOM, instance, buffer, len);
	};
	_packetHandleFuncs[S_PLAYERLIST] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_PLAYERLIST>(Handle_S_PLAYERLIST, instance, buffer, len);
	};
	_packetHandleFuncs[S_MONSTERLIST] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_MONSTERLIST>(Handle_S_MONSTERLIST, instance, buffer, len);
	};
	_packetHandleFuncs[S_MOVE] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_MOVE>(Handle_S_MOVE, instance, buffer, len);
	};
	_packetHandleFuncs[S_ATTACK] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_ATTACK>(Handle_S_ATTACK, instance, buffer, len);
	};
	_packetHandleFuncs[S_CHAT] = [this](UMyGameInstance* instance, uint8* buffer, int len) {
		return HandlePacket<PROTOCOL::S_CHAT>(Handle_S_CHAT, instance, buffer, len);
	};
}

/*---------------------------------------------------------------------------------------------
	HandlePacket - public
---------------------------------------------------------------------------------------------*/
bool ServerPacketHandler::HandlePacket(UMyGameInstance* instance, uint8* buffer, int len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	return _packetHandleFuncs[header->_id](instance, buffer, len);
}

/*---------------------------------------------------------------------------------------------
	 MakeSendBuffer - public
---------------------------------------------------------------------------------------------*/
TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_LOGIN toPkt)
{
	return MakeSendBuffer(toPkt, C_LOGIN);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_ENTER_ROOM toPkt)
{
	return MakeSendBuffer(toPkt, C_ENTER_ROOM);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_PLAYERLIST toPkt)
{
	return MakeSendBuffer(toPkt, C_PLAYERLIST);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_MONSTERLIST toPkt)
{
	return MakeSendBuffer(toPkt, C_MONSTERLIST);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_MOVE toPkt)
{
	return MakeSendBuffer(toPkt, C_MOVE);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_ATTACK toPkt)
{
	return MakeSendBuffer(toPkt, C_ATTACK);
}

TSharedPtr<MySendBuffer> ServerPacketHandler::MakeSendBuffer(PROTOCOL::C_CHAT toPkt)
{
	return MakeSendBuffer(toPkt, C_CHAT);
}
