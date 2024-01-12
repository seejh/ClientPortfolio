// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDWidget.h"
#include"MyGameInstance.h"

void UMyHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChatBtn->OnReleased.AddDynamic(this, &UMyHUDWidget::OnClickedChatBtn);
}

void UMyHUDWidget::SetHPBar(float hp)
{
	HPBar->SetPercent(hp / 100.f);
}

void UMyHUDWidget::AddChatMessage(FString& message)
{
	UTextBlock* newTextBlock = NewObject<UTextBlock>(ChatScrollBox);
	newTextBlock->SetText(FText::FromString(message));
	
	ChatScrollBox->AddChild(newTextBlock);
	ChatScrollBox->ScrollToEnd();
}

void UMyHUDWidget::OnClickedChatBtn()
{
	// TEST
	UE_LOG(LogTemp, Error, TEXT("ChatBtn Clicked"));

	FString chatMessage(ChatEditableTextBox->GetText().ToString());
	if (chatMessage.IsEmpty())
		return;

	UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	instance->_controller->MyPlayerChat(chatMessage);
	
	ChatEditableTextBox->SetText(FText::GetEmpty());
}
