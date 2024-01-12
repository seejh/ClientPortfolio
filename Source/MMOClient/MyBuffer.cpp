// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBuffer.h"


/*----------------------------------------
    RecvBuffer
----------------------------------------*/
MyRecvBuffer::MyRecvBuffer(int32 size = 1000) : _size(size + 200), _readPos(0), _writePos(0)
{
    _buffer = new uint8[_size];
}

MyRecvBuffer::~MyRecvBuffer()
{
    delete _buffer;
}

bool MyRecvBuffer::OnRead(int32 len)
{
    if (DataSize() < len) return false;

    _readPos += len;

    return true;
}

bool MyRecvBuffer::OnWrite(int32 len)
{
    if (FreeSize() < len) return false;

    _writePos += len;

    return true;
}

void MyRecvBuffer::Reset()
{
    if (_readPos == 0) return;

    if (_readPos == _writePos) {
        _readPos = _writePos = 0;
        return;
    }

    ::memcpy(_buffer, &_buffer[_readPos], DataSize());

    _writePos -= _readPos;
    _readPos = 0;
}

uint8* MyRecvBuffer::BufferReadPos()
{
    return &_buffer[_readPos];
}

uint8* MyRecvBuffer::BufferWritePos()
{
    return &_buffer[_writePos];
}

int32 MyRecvBuffer::DataSize()
{
    return _writePos - _readPos;
}

int32 MyRecvBuffer::FreeSize()
{
    return _size - _writePos;
}

/*-------------------------------
    SendBuffer
--------------------------------*/
MySendBuffer::MySendBuffer(int size = 1000) : _size(size)
{
    _buffer = new uint8[_size + 200];
}

MySendBuffer::~MySendBuffer()
{
    delete _buffer;
}

uint8* MySendBuffer::Buffer()
{
    return _buffer;
}

int MySendBuffer::Size()
{
    return _size;
}
