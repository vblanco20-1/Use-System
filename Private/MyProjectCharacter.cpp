// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "MyProject.h"
#include "MyProjectCharacter.h"
#include "MyProjectProjectile.h"
#include "Animation/AnimInstance.h"

#include "Usable.h"


//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter


AMyProjectCharacter::AMyProjectCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}
//////////////////////////////////////////////////////////////////////////
// Input

void AMyProjectCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	
	InputComponent->BindAction("Fire", IE_Pressed, this, &AMyProjectCharacter::OnFire);
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyProjectCharacter::TouchStarted);

	InputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);
}

void AMyProjectCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<AMyProjectProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void AMyProjectCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// only fire for first finger down
	if (FingerIndex == 0)
	{
		OnFire();
	}
}

void AMyProjectCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyProjectCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void AMyProjectCharacter::UseTarget()
{
	FHitResult HitTarget;
	TraceViewTarget(HitTarget);
	if (HitTarget.GetActor())
	{
		// Cast to IUsable for the C++ implementation of the interface
		IUsable* usable =Cast<IUsable>(HitTarget.GetActor());
		if (usable)
		{
			usable->OnUsed(GetController());
		}
		// pure blueprint interfaces wont be casted to IUsable, so we do it with the ImplementsInterface method and call the blueprint directly
		else if (HitTarget.GetActor()->GetClass()->ImplementsInterface(UUsable::StaticClass()))
		{
			IUsable::Execute_BTOnUsed(HitTarget.GetActor(), GetController());
		}
	}
}

void AMyProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// trace usable
	FHitResult HitTarget;
	TraceViewTarget(HitTarget);
	if (HitTarget.GetActor())
	{
		// Cast to IUsable for the C++ implementation of the interface
		IUsable* usable = Cast<IUsable>(HitTarget.GetActor());
		if (usable)
		{
			usable->OnFocus(GetController());
		}
		// pure blueprint interfaces wont be casted to IUsable, so we do it with the ImplementsInterface method and call the blueprint directly
		else if (HitTarget.GetActor()->GetClass()->ImplementsInterface(UUsable::StaticClass()))
		{
			IUsable::Execute_BTOnFocus(HitTarget.GetActor(), GetController());
		}
	}
}
void AMyProjectCharacter::TraceViewTarget(FHitResult &HitResult)
{

	// do a trace from view location, in the direction you are aiming, to get the object under the crosshair
	FCollisionQueryParams TraceParams("UseTrace", false, this);
	FVector TraceStart = GetPawnViewLocation();
	FVector TraceVector = GetBaseAimRotation().Vector();
	TraceVector.Normalize();
	FVector TraceEnd = TraceStart + (TraceVector * UseRange);
	HitResult = FHitResult(ForceInit);

	GetWorld()->LineTraceSingle(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
}
