// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"proto/MyEnum.pb.h"
#include"proto/MyStruct.pb.h"
#include"proto/MyProtocol.pb.h"
#include"MyBuffer.h"

#include "CoreMinimal.h"

class AMyPlayerController;
class UMyGameInstance;

/*----------------------------
	PacketHeader
----------------------------*/
class PacketHeader {
public:
	short _id;
	short _size;
};

/*----------------------------------
	ServerPacketHandler
---------------------------------*/
enum {
	PacketCount = 3000,
};

enum PacketType {
	C_LOGIN = 1, S_LOGIN,
	C_ENTER_ROOM, S_ENTER_ROOM,
	C_PLAYERLIST, S_PLAYERLIST,
	C_MOVE, S_MOVE,
	C_ATTACK, S_ATTACK,
	C_CHAT, S_CHAT,
};

bool Handle_INVALID(UMyGameInstance* instance, uint8* buffer, int len);
bool Handle_S_LOGIN(UMyGameInstance* instance, PROTOCOL::S_LOGIN fromPkt);
bool Handle_S_ENTER_ROOM(UMyGameInstance* instance, PROTOCOL::S_ENTER_ROOM fromPkt);
bool Handle_S_PLAYERLIST(UMyGameInstance* instance, PROTOCOL::S_PLAYERLIST fromPkt);
bool Handle_S_MOVE(UMyGameInstance* instance, PROTOCOL::S_MOVE fromPkt);
bool Handle_S_ATTACK(UMyGameInstance* instance, PROTOCOL::S_ATTACK fromPkt);
bool Handle_S_CHAT(UMyGameInstance* instance, PROTOCOL::S_CHAT fromPkt);

class MMOCLIENT_API ServerPacketHandler
{
public:
	void Init();

	/*---------------------------------------------------------------------------------------------
		HandlePacket(Recv) - Public
	---------------------------------------------------------------------------------------------*/
	bool HandlePacket(UMyGameInstance* instance, uint8* buffer, int len);
	
	/*---------------------------------------------------------------------------------------------
		MakeSendBuffer(Send) - Public
	---------------------------------------------------------------------------------------------*/
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_LOGIN toPkt);
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_ENTER_ROOM toPkt);
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_PLAYERLIST toPkt);
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_MOVE toPkt);
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_ATTACK toPkt);
	TSharedPtr<MySendBuffer> MakeSendBuffer(PROTOCOL::C_CHAT toPkt);
private:
	/*---------------------------------------------------------------------------------------------
		HandlePacket(Recv) - Private
	---------------------------------------------------------------------------------------------*/
	template<typename PacketType>
	bool HandlePacket(TFunction<bool(UMyGameInstance*, PacketType)> func, UMyGameInstance* instance, uint8* buffer, int len) {
		PacketType pkt;
		pkt.ParseFromArray(buffer + sizeof(PacketHeader), len);

		return func(instance, pkt);
	}

	/*---------------------------------------------------------------------------------------------
		MakeSendBuffer(Send) - Private
	---------------------------------------------------------------------------------------------*/
	template<typename Pkt>
	TSharedPtr<MySendBuffer> MakeSendBuffer(Pkt pkt, int pktId) {
		int packetSize = static_cast<int>(pkt.ByteSizeLong());
		int totalSize = packetSize + sizeof(PacketHeader);

		TSharedPtr<MySendBuffer> sendBuffer = MakeShareable(new MySendBuffer(totalSize));

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		new(header)PacketHeader();

		header->_id = pktId;
		header->_size = packetSize;

		if (!pkt.SerializeToArray(sendBuffer->Buffer() + sizeof(PacketHeader), header->_size)) {
			UE_LOG(LogTemp, Error, TEXT("packetHandler makePacket Error"));
			return nullptr;
		}

		return sendBuffer;
	}
public:
	TFunction<bool(UMyGameInstance*, uint8*, int)> _packetHandleFuncs[PacketCount];
};
