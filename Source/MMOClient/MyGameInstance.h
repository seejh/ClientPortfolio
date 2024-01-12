// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include"NetSession.h"
#include"ServerPacketHandler.h"
#include"MyPlayerController.h"
#include"LoginWidget.h"

#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MMOCLIENT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	bool TryLogin(FString id, FString pw);
public:
	ULoginWidget* _loginWidget;

	TSharedPtr<NetSession> _netSession;
	TSharedPtr<ServerPacketHandler> _packetHandler;
	AMyPlayerController* _controller;

	uint64 _myPlayerIndex = 0;
	TMap<uint64, PlayerInfo> _intPlayerInfoMap;
};
