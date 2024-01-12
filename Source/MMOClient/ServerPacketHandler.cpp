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
		instance->_myPlayerIndex = fromPkt.player().playerid();
		
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
		if (fromPkt.player().playerid() == instance->_myPlayerIndex) {
			// 다음 맵으로 이동 (이동하면 PC가동)
			instance->_loginWidget->GoNextLevel();
		}

		// 접속한 것이 타인
		else {
			// 접속한 플레이어의 정보 추출
			PROTOCOL::PLAYER player = fromPkt.player();

			// 플레이어 인포 정의
			PlayerInfo playerInfo;
			playerInfo.SetPlayerIndex(player.playerid());
			playerInfo.SetLocation(player.locationx(), player.locationy(), player.locationz());
			playerInfo.SetRotation(player.rotationyaw(), player.rotationpitch(), player.rotationroll());
			playerInfo.SetHP(player.hp());
			// TODO : set ID

			// 업데이트 아웃 데이터(스폰)
			instance->_controller->SpawnUpdateOutData(playerInfo);
		}
	}

	return true;
}

// 처음 접속할 때 받는 패킷
// 접속한 플레이어 명단을 받으며 나의 정보도 포함, 걸러내야됨
bool Handle_S_PLAYERLIST(UMyGameInstance* instance, PROTOCOL::S_PLAYERLIST fromPkt)
{
	// 목록에서 하나씩
	for (int i = 0; i < fromPkt.players_size(); i++) {
		// 플레이어 정보 추출
		PROTOCOL::PLAYER playerInfoProto = fromPkt.players(i);
		
		// 추출된 게 자기라면 pass
		if (playerInfoProto.playerid() == instance->_myPlayerIndex) {
			continue;
		}
		
		// 추출된 것이 남이라면
		else {
			// 플레이어 인포 정의
			PlayerInfo info;
			info.SetPlayerIndex(playerInfoProto.playerid());
			info.SetLocation(playerInfoProto.locationx(), playerInfoProto.locationy(), playerInfoProto.locationz());
			info.SetRotation(playerInfoProto.rotationyaw(), playerInfoProto.rotationpitch(), playerInfoProto.rotationroll());
			info.SetHP(playerInfoProto.hp());
			// TODO : set ID

			// 업데이트 아웃 데이터(스폰)
			instance->_controller->SpawnUpdateOutData(info);
		}
	}

	return true;
}

bool Handle_S_MOVE(UMyGameInstance* instance, PROTOCOL::S_MOVE fromPkt)
{
	// 추출
	PROTOCOL::PLAYER player = fromPkt.player();

	// 플레이어 인포 정의
	PlayerInfo info;
	info.SetPlayerIndex(player.playerid());
	info.SetLocation(player.locationx(), player.locationy(), player.locationz());
	info.SetRotation(player.rotationyaw(), player.rotationpitch(), player.rotationroll());
	info.SetVelocity(player.velocityx(), player.velocityy(), player.velocityz());

	// TEST
	PlayerInfo* playerInfo = instance->_intPlayerInfoMap.Find(info.PlayerIndex());
	if (playerInfo->IsLogin()) {
		playerInfo->SetLocation(info.GetLocation());
		playerInfo->SetRotation(info.GetRotation());
		playerInfo->SetVelocity(info.GetVelocity());
	}

	// 업데이트 아웃 데이터
	//instance->_controller->WorldUpdateOutData(info);

	return true;
}

// 나의 공격도 서버로 부터 응답받고 처리하는 방식
bool Handle_S_ATTACK(UMyGameInstance* instance, PROTOCOL::S_ATTACK fromPkt)
{
	TArray<PlayerInfo> playerInfoArray;
	
	// 공격자
	PROTOCOL::PLAYER attackPlayer = fromPkt.attacker();
	
	PlayerInfo attacker;
	attacker.SetPlayerIndex(attackPlayer.playerid());
	
	playerInfoArray.Add(attacker);

	// 피격자
	for (int i = 0; i < fromPkt.victims_size(); i++) {
		PROTOCOL::PLAYER attackedPlayer = fromPkt.victims(i);
		
		PlayerInfo victim;
		victim.SetPlayerIndex(attackedPlayer.playerid());
		victim.SetHP(attackedPlayer.hp());

		playerInfoArray.Add(victim);
	}

	// 
	instance->_controller->AttackUpdateOutData(playerInfoArray);

	return true;
}

bool Handle_S_CHAT(UMyGameInstance* instance, PROTOCOL::S_CHAT fromPkt)
{	
	// PLAYER-1:채팅 내용
	FString chat("Player-");
	chat.Append(FString::FromInt(fromPkt.player().playerid()));
	chat.Append(": ");
	chat.Append(fromPkt.text().c_str());

	instance->_controller->ChatUpdateOutData(chat);

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
