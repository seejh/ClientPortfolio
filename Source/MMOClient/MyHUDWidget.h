// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include<Components/ProgressBar.h>
#include<Components/ScrollBox.h>
#include<Components/EditableTextBox.h>
#include<Components/Button.h>

#include "MyHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class MMOCLIENT_API UMyHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UProgressBar* HPBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UScrollBox* ChatScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UEditableTextBox* ChatEditableTextBox;

	UPROPERTY(meta = (BindWidget))
		class UButton* ChatBtn;

	UFUNCTION(BlueprintCallable)
		void OnClickedChatBtn();

	void SetHPBar(float hp);
	void AddChatMessage(FString& message);
	
};
