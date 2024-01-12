// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AMyPlayerController;
class UMyGameInstance;
class MyRecvBuffer;
class MySendBuffer;
class MMOCLIENT_API NetSession : FRunnable
{
public:
	NetSession(UMyGameInstance* instance);
	~NetSession();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	bool Connect();
	void Send(TSharedPtr<MySendBuffer> sendBuffer);
	void Recv();
public:
	FSocket* _socket;
	FRunnableThread* _thread;

	UMyGameInstance* _instance;

	TSharedPtr<MyRecvBuffer> _recvBuffer;
	TAtomic<bool> _isRun = false;
};
