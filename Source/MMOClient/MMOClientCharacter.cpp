// Copyright Epic Games, Inc. All Rights Reserved.

#include "MMOClientCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include<Components/SphereComponent.h>

#include<Kismet/GameplayStatics.h>
#include"MyGameInstance.h"
#include"MyPlayerController.h"
#include"MyMonster.h"

//////////////////////////////////////////////////////////////////////////
// AMMOClientCharacter

AMMOClientCharacter::AMMOClientCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
}

void AMMOClientCharacter::SetAttack(bool flag)
{
	_isAttack = flag;
}

bool AMMOClientCharacter::GetAttack()
{
	return _isAttack;
}

void AMMOClientCharacter::SetAttacked(bool flag)
{
	_isAttacked = flag;
}

bool AMMOClientCharacter::GetAttacked()
{
	return _isAttacked;
}

float AMMOClientCharacter::GetMaxHP()
{
	return  MAXHP;
}

float AMMOClientCharacter::GetHP()
{
	return _hp;
}

void AMMOClientCharacter::SetHP(float hp)
{
	_hp = hp;
}


//////////////////////////////////////////////////////////////////////////
// Input

void AMMOClientCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMMOClientCharacter::Attack);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMMOClientCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMMOClientCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMMOClientCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMMOClientCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMMOClientCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMMOClientCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMMOClientCharacter::OnResetVR);
}


void AMMOClientCharacter::DoAttack()
{
	PlayAttackAnim();
}

void AMMOClientCharacter::DoAttacked(int hp)
{
	PlayAttackedAnim();

	_hp = hp;
	UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	
	// ó���� �� �� ĳ���̸� hp hud ����
	if (_playerIndex == instance->_myPlayerIndex)
		instance->_controller->myHUDWidget->SetHPBar(_hp);

	if (_hp <= 0) {
		// TODO : Death
	}
}

void AMMOClientCharacter::Attack()
{
	this->PlayAttackAnim();

	// �浹�� ������ ���͵� ������
	TArray<UActorComponent*> actorComps;
	TArray<AActor*> attackedChars;

	this->GetComponents(actorComps);
	for (auto actorComp : actorComps) {
		USphereComponent* hitSphere = Cast<USphereComponent>(actorComp);
		if (hitSphere) {
			hitSphere->GetOverlappingActors(attackedChars);
			break;
		}
	}
	
	// ���ε� ���ԵǾ� �ֱ� ������, ���� �����ϰ� �ϳ��� ������, �� 2�� �̻��̸�
	UMyGameInstance* instance = Cast<UMyGameInstance>(GetGameInstance());
	TArray<ActorInfo> victims;

	if (attackedChars.Num() >= 2) {
		for (auto victim : attackedChars) {
			AMMOClientCharacter* victimChar = Cast<AMMOClientCharacter>(victim);
			
			// �÷��̾�
			if (victimChar) {
				// �÷��̾ ����� �н�
				if (victimChar->_playerIndex == instance->_myPlayerIndex)
					continue;

				PlayerInfo info;
				info.SetIndex(victimChar->_playerIndex);
				victims.Add(info);
			}
			
			// ����
			else {
				AMyMonster* victimMon = Cast<AMyMonster>(victim);

				MonsterInfo info;
				//info.SetIndex(victimMon->_monsterIndex);
				// victims.Add(info);
			}
		}
	}

	instance->_controller->MyPlayerAttack(victims);
}

void AMMOClientCharacter::OnResetVR()
{
	// If MMOClient is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in MMOClient.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMMOClientCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMMOClientCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMMOClientCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMMOClientCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMMOClientCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMMOClientCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
