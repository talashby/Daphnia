// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DaphniaPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "MyPlayerController.h"
#include "MyHudWidget.h"

static ADaphniaPawn* s_InstancePtr;

ADaphniaPawn::ADaphniaPawn()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	RootComponent = PlaneMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->TargetOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 150.f;
	MaxSpeed = 2000.f;
	MinSpeed = 0.f;
	CurrentForwardSpeed = 0.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	Camera->Activate();

	// Create camera eye component
	CameraEye = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraEye"));
	auto EyeCoord = ConstructorStatics.PlaneMesh.Get()->GetBoundingBox().Max.X;
	CameraEye->SetRelativeLocation(FVector(EyeCoord, 0, 0));
	CameraEye->SetupAttachment(RootComponent);	// Attach the camera
	CameraEye->bUsePawnControlRotation = false; // Don't rotate camera with controller
	CameraEye->Deactivate();

	EyeSceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("EyeSceneCaptureComponent2D0"));
	EyeSceneCaptureComponent2D->SetupAttachment(RootComponent);
	FMinimalViewInfo MinimalViewInfo;
	CameraEye->GetCameraView(1, MinimalViewInfo);
	EyeSceneCaptureComponent2D->SetCameraView(MinimalViewInfo);
	EyeSceneCaptureComponent2D->SetRelativeLocation(FVector(EyeCoord, 0, 0));
	EyeSceneCaptureComponent2D->bCaptureEveryFrame = true;
}

ADaphniaPawn* ADaphniaPawn::Instance()
{
	checkSlow(s_InstancePtr);
	return s_InstancePtr;
}

void ADaphniaPawn::BeginPlay()
{
	s_InstancePtr = this;
	Super::BeginPlay();

	EyeRenderTarget2D = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), 32, 32, RTF_RGBA8);
	EyeSceneCaptureComponent2D->TextureTarget = EyeRenderTarget2D;
}

void ADaphniaPawn::Tick(float DeltaSeconds)
{
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0,0,0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ADaphniaPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
}


void ADaphniaPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAxis("Thrust", this, &ADaphniaPawn::ThrustInput);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADaphniaPawn::MoveUpInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADaphniaPawn::MoveRightInput);
}

void ADaphniaPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);
	// If input is not held down, reduce speed
	float CurrentAcc = bHasInput ? (Val * Acceleration) : (-0.5f * Acceleration);
	// Calculate new speed
	float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
	// Clamp between MinSpeed and MaxSpeed
	CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void ADaphniaPawn::MoveUpInput(float Val)
{
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ADaphniaPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetYawSpeed = (Val * TurnSpeed);

	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If turning, yaw value is used to influence roll
	// If not turning, roll to reverse current roll value.
	float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ADaphniaPawn::SwitchView()
{
	auto InputController = AMyPlayerController::Instance();
	verify(InputController);
	if (InputController)
	{
		++CameraViewMode;
		if (CameraViewMode >= static_cast<int>(ECameraViewMode::Max))
		{
			CameraViewMode = 0;
		}

		if (CameraViewMode == static_cast<int>(ECameraViewMode::TargetOffset))
		{
			SpringArm->TargetOffset = FVector(0.f, 0.f, 60.f);
			SpringArm->SocketOffset = FVector();
			Camera->Activate();
			CameraEye->Deactivate();
		}
		else if (CameraViewMode == static_cast<int>(ECameraViewMode::SocketOffset))
		{
			SpringArm->TargetOffset = FVector();
			SpringArm->SocketOffset = FVector(-200.f, 0.f, 0.f);
			Camera->Activate();
			CameraEye->Deactivate();
		}
		else if (CameraViewMode == static_cast<int>(ECameraViewMode::Eye))
		{
			Camera->Deactivate();
			CameraEye->Activate();
		}
		else
		{
			checkNoEntry();
		}
		InputController->SetViewTargetWithBlend(this);
	}
}

class UTextureRenderTarget2D* ADaphniaPawn::GetEyeRenderTarget2D()
{
	return EyeRenderTarget2D;
}
