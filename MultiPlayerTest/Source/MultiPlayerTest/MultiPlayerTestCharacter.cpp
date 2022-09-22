// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiPlayerTestCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


//////////////////////////////////////////////////////////////////////////
// AMultiPlayerTestCharacter

AMultiPlayerTestCharacter::AMultiPlayerTestCharacter():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,&ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,&ThisClass::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
	if(OnlineSubSystem)
	{
		OnlineSessionInterface = OnlineSubSystem->GetSessionInterface();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue,FString::Printf(TEXT("Found SubSystem %s"), *OnlineSubSystem->GetSubsystemName().ToString()));
		}
	}
}

void AMultiPlayerTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMultiPlayerTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMultiPlayerTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMultiPlayerTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMultiPlayerTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMultiPlayerTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMultiPlayerTestCharacter::TouchStopped);
}

void AMultiPlayerTestCharacter::CreatGameSession()
{
	//called when pressing "1" key
	if (!OnlineSessionInterface.IsValid()) { return; }
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		//if existing session, destroy it 
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	//add our delegate to delegate list
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//create a new session
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;//最大连接数量
	SessionSettings->bAllowJoinInProgress = true;//允许在会话已被创建时加入
	SessionSettings->bAllowJoinViaPresence = true;//可以通过steam加入
	SessionSettings->bShouldAdvertise = true;//通过steam公开
	SessionSettings->bUsesPresence = true;//使用steam账户所在地区
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);

}

void AMultiPlayerTestCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid()){ return; }
	// find game session

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000;//可以发现的最大结果数量
	SessionSearch->bIsLanQuery = false;//不适用lan
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true,EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

void AMultiPlayerTestCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Success to create session %s"),*SessionName.ToString()));
		}
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString("Game/Map/Lobby?listen"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Fail to create session")));
		}
	}
}

void AMultiPlayerTestCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid()) { return; }
	for (auto Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString UserName = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"),MatchType);

		if (GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, FString::Printf(TEXT("ID: %s  UserName: %s"), *Id,*UserName));}
		if (MatchType == FString("FreeForAll"))
		{
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, FString::Printf(TEXT("Joining Match Type: %s"), *MatchType)); }
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,Result);
	    }
	}
}

void AMultiPlayerTestCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid()) { return; }

	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Connect string: %s"), *Address)); }
	}
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (PlayerController)
	{
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}

}

void AMultiPlayerTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMultiPlayerTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMultiPlayerTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMultiPlayerTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMultiPlayerTestCharacter::MoveForward(float Value)
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

void AMultiPlayerTestCharacter::MoveRight(float Value)
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
