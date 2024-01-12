// Fill out your copyright notice in the Description page of Project Settings.


#include "ThreadHandler.h"

/*---------------------------------------------------------
    RunnableThread
---------------------------------------------------------*/
RunnableThread::RunnableThread(TFunction<void()> job, const TCHAR* name, TFunction<void()> init)
{
    _job = job;
    _init = init;
    _thread = MakeShareable(FRunnableThread::Create(this, name));
}

bool RunnableThread::Init()
{
    _init();
    return true;
}

uint32 RunnableThread::Run()
{
    while (_isRun) {
        _job();
    }

    return 0;
}

void RunnableThread::Stop()
{
    _isRun = false;
}

void RunnableThread::Exit()
{
}


/*---------------------------------------------------------
    ThreadHandler
---------------------------------------------------------*/
ThreadHandler::~ThreadHandler()
{
    JoinThreads();
}

void ThreadHandler::LaunchThread(TFunction<void()> job, const TCHAR* name, TFunction<void()> init)
{
    _threads.Push(MakeShared<RunnableThread>(job, name, init));
}

void ThreadHandler::JoinThreads()
{
    for (auto& thread : _threads)
        thread->Stop();
}
