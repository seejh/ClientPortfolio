// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include<Components/EditableTextBox.h>
#include<Components/TextBlock.h>
#include<Components/Button.h>

#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class MMOCLIENT_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UEditableTextBox* IdEditableTextBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
		class UEditableTextBox* PwEditableTextBox;
	UPROPERTY(meta = (BindWidget))
		class UButton* LoginBtn;

	UFUNCTION(BlueprintCallable)
		void OnClickedLogin();
	
	void GoNextLevel();
	void SetEnableWidget(bool flag);
public:

};
