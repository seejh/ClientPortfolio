// Fill out your copyright notice in the Description page of Project Settings.


#include "NetSession.h"

#include<Sockets.h>
#include<Runtime/Networking/Public/Interfaces/IPv4/IPv4Address.h>
#include<Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h>
#include<SocketSubsystem.h>

#include"MyGameInstance.h"
#include"MyPlayerController.h"
#include"MyBuffer.h"
#include"ServerPacketHandler.h"

/*--------------------------------------
    NetSession
---------------------------------------*/
NetSession::NetSession(UMyGameInstance* instance) : _instance(instance)
{
    _recvBuffer = MakeShareable(new MyRecvBuffer(2000));
}

NetSession::~NetSession()
{
}

bool NetSession::Init()
{
    return true;
}

uint32 NetSession::Run()
{
    while (_isRun.Load() == true) {
        Recv();
    }

    _socket->Shutdown(ESocketShutdownMode::ReadWrite);

    return 0;
}

void NetSession::Stop()
{
    _isRun.Store(false);
}

void NetSession::Exit()
{
    _thread->WaitForCompletion();
    _thread->Kill();
    delete _thread;
    _thread = nullptr;
}

bool NetSession::Connect()
{
    _socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT(""), false);

    FIPv4Address ipAddr;
    FIPv4Address::Parse(TEXT("127.0.0.1"), ipAddr);
    FIPv4Endpoint endPoint = FIPv4Endpoint(ipAddr, 7777);

    //_socket->SetNoDelay(true);
    _socket->SetNonBlocking(true);
    _socket->Connect(*endPoint.ToInternetAddr());

    ESocketConnectionState state = _socket->GetConnectionState();
    if (state != ESocketConnectionState::SCS_Connected) {
        UE_LOG(LogTemp, Error, TEXT("Socket Connect Error"));
        return false;
    }

    UE_LOG(LogTemp, Error, TEXT("Socket Connect OK"));
    _isRun.Store(true);
    _thread = FRunnableThread::Create(this, TEXT(""));

    return true;
}

void NetSession::Send(TSharedPtr<MySendBuffer> sendBuffer)
{
    int32 sendBytes;
    _socket->Send(sendBuffer->Buffer(), sendBuffer->Size(), sendBytes);
    if (sendBytes <= 0) {
        // TODO : error
        UE_LOG(LogTemp, Error, TEXT("send Error Len <= 0"));
    }
    else {
        // UE_LOG(LogTemp, Error, TEXT("send OK Len : %d"), sendBytes);
    }
}

void NetSession::Recv()
{
    int32 recvBytes;
    _socket->Recv(_recvBuffer->BufferWritePos(), _recvBuffer->FreeSize(), recvBytes);

    if (recvBytes > 0) {
        _recvBuffer->OnWrite(recvBytes);
        if (_recvBuffer->DataSize() >= sizeof(PacketHeader)) {
            PacketHeader* header = reinterpret_cast<PacketHeader*>(_recvBuffer->BufferReadPos());
            int packetSize = header->_size;
            int totalSize = packetSize + sizeof(PacketHeader);

            if (_recvBuffer->DataSize() >= totalSize) {
                _instance->_packetHandler->HandlePacket(_instance, _recvBuffer->BufferReadPos(), packetSize);
                _recvBuffer->OnRead(totalSize);
                _recvBuffer->Reset();
            }
        }
    }
}
