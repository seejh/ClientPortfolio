// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/*---------------------------------------------------------
	RunnableThread
---------------------------------------------------------*/
class RunnableThread : public FRunnable {
public:
	RunnableThread(TFunction<void()> job, const TCHAR* name, TFunction<void()> init);

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
private:
	TSharedPtr<FRunnableThread> _thread;
	TCHAR* _name;

	bool _isRun = true;

	TFunction<void()> _init;
	TFunction<void()> _job;
};

/*---------------------------------------------------------
	ThreadHandler
---------------------------------------------------------*/
class MMOCLIENT_API ThreadHandler
{
	ThreadHandler(){}
public:
	~ThreadHandler();

	static ThreadHandler* GetInstance() {
		static ThreadHandler instance;
		return &instance;
	}

	void LaunchThread(TFunction<void()> job, const TCHAR* name, TFunction<void()> init);
	void JoinThreads();
private:
	TArray<TSharedPtr<RunnableThread>> _threads;
};
