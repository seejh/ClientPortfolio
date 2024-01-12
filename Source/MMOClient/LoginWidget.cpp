// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"

#include<Kismet/KismetSystemLibrary.h>
#include<Kismet/GameplayStatics.h>
#include"MyGameInstance.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	instance->_loginWidget = this;
	LoginBtn->OnReleased.AddDynamic(this, &ULoginWidget::OnClickedLogin);
}

void ULoginWidget::OnClickedLogin()
{
	FString id(IdEditableTextBox->GetText().ToString());
	FString pw(PwEditableTextBox->GetText().ToString());
	
	// login
	UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	instance->TryLogin(id, pw);
}

void ULoginWidget::GoNextLevel()
{
	FString levelName = L"/Game/NewMap";
	UGameplayStatics::OpenLevel(GetWorld(), *levelName, false, "");
}

void ULoginWidget::SetEnableWidget(bool flag)
{
	LoginBtn->SetIsEnabled(flag);
	IdEditableTextBox->SetIsEnabled(flag);
	PwEditableTextBox->SetIsEnabled(flag);
}
