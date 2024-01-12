// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

void UMyGameInstance::Init()
{
    // player list 준비
    for (uint64 i = 0; i < 50; i++) {
        PlayerInfo playerInfo;
        playerInfo._playerIndex = i;
        _intPlayerInfoMap.Add(i, playerInfo);
    }

    // NetSession, ServerPacketHandler 가동
    _netSession = MakeShareable(new NetSession(this));
    _packetHandler = MakeShareable(new ServerPacketHandler());
    _packetHandler->Init();

    // connect session
    if (!_netSession->Connect()) {
        return;
    }
}

bool UMyGameInstance::TryLogin(FString id, FString pw)
{
    UE_LOG(LogTemp, Error, TEXT("Try Login"));

    PROTOCOL::C_LOGIN loginPkt;
    loginPkt.set_id(TCHAR_TO_ANSI(*id));
    loginPkt.set_pw(TCHAR_TO_ANSI(*pw));

    TSharedPtr<MySendBuffer> sendBuffer = _packetHandler->MakeSendBuffer(loginPkt);
    _netSession->Send(sendBuffer);

    return true;
}
