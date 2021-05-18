// Fill out your copyright notice in the Description page of Project Settings.


#include "WheeledVehiclePawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"

static const FName NAME_SteerInput("Steer");
static const FName NAME_ThrottleInput("Throttle");

AWheeledVehiclePawn::AWheeledVehiclePawn()
{
	UWheeledVehicleMovementComponent4W* vehicle4w = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());
	
	// Tire load
	vehicle4w->MinNormalizedTireLoad = 0.0f;
	vehicle4w->MinNormalizedTireLoadFiltered = 0.2f;
	vehicle4w->MaxNormalizedTireLoad = 2.0f;
	vehicle4w->MaxNormalizedTireLoadFiltered = 2.0f;

	// Torque
	vehicle4w->SteeringCurve.GetRichCurve()->Reset();
	vehicle4w->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	vehicle4w->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	vehicle4w->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	vehicle4w->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	vehicle4w->DifferentialSetup.FrontRearSplit = 0.65;

	// Gearbox - automatic
	vehicle4w->TransmissionSetup.bUseGearAutoBox = true;
	vehicle4w->TransmissionSetup.GearSwitchTime = 0.15f;
	vehicle4w->TransmissionSetup.GearAutoBoxLatency = 1.0f;

	// Spring arm component creation for chase camera
	springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArm"));
	springArm->SetupAttachment(RootComponent);
	springArm->TargetArmLength = 250.0f;
	springArm->bUsePawnControlRotation = true;

	// chase camera component creation
	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("chaseCamera"));
	camera->SetupAttachment(springArm, USpringArmComponent::SocketName);
	camera->FieldOfView = 90.0f;
}

void AWheeledVehiclePawn::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
	updateAirControl(DeltaTime);
}

void AWheeledVehiclePawn::applyThrottle(float val)
{
	GetVehicleMovementComponent()->SetThrottleInput(val);
}

void AWheeledVehiclePawn::applySteering(float val)
{
	GetVehicleMovementComponent()->SetSteeringInput(val);
}

void AWheeledVehiclePawn::lookUp(float val)
{
	if (val != 0.0f)
	{
		AddControllerPitchInput(val);
	}
}

void AWheeledVehiclePawn::turn(float val)
{
	if (val != 0.0f)
	{
		AddControllerYawInput(val);
	}
}

void AWheeledVehiclePawn::onHandBrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AWheeledVehiclePawn::onHandBrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AWheeledVehiclePawn::updateAirControl(float DeltaTime)
{
	if (UWheeledVehicleMovementComponent4W* vehicle4w = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement()))
	{
		FCollisionQueryParams QueryParms;
		QueryParms.AddIgnoredActor(this);

		const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, 50.f);
		const FVector TraceEnd = GetActorLocation() - FVector(0.0f, 0.0f, 200.f);
	
		FHitResult Hit;

		// Check if the vehicle is flipped - in the air
		const bool inAir = !GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParms);
		const bool notGrounded = FVector::DotProduct(GetActorUpVector(), FVector::UpVector) < 0.1f;

		// if only in air or not grounded
		if (inAir || notGrounded)
		{
			const float ForwardInput = InputComponent->GetAxisValue(NAME_ThrottleInput);
			const float RightInput = InputComponent->GetAxisValue(NAME_SteerInput);

			// Allow player to roll the vehicle over
			const float airMovementForcePitch = 3.0f;
			const float airMovementForceRoll = !inAir && notGrounded ? 20.f : 3.f;

			if (UPrimitiveComponent* vehicleMesh = vehicle4w->UpdatedPrimitive)
			{
				const FVector movementVecotr = FVector(RightInput * -airMovementForceRoll, ForwardInput * airMovementForcePitch, 0.f) * DeltaTime * 200.0f;
				const FVector newAngularMovement = GetActorRotation().RotateVector(movementVecotr);

				vehicleMesh->SetPhysicsAngularVelocity(newAngularMovement, true);
			}
		}
	}
}

void AWheeledVehiclePawn::SetUpPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(NAME_ThrottleInput, this, &AWheeledVehiclePawn::applyThrottle);
	PlayerInputComponent->BindAxis(NAME_SteerInput, this, &AWheeledVehiclePawn::applySteering);
	PlayerInputComponent->BindAxis("lookUp", this, &AWheeledVehiclePawn::lookUp);
	PlayerInputComponent->BindAxis("turn", this, &AWheeledVehiclePawn::turn);

	PlayerInputComponent->BindAction("handBrake", IE_Pressed, this, &AWheeledVehiclePawn::onHandBrakePressed);
	PlayerInputComponent->BindAction("handBrake", IE_Released, this, &AWheeledVehiclePawn::onHandBrakeReleased);
}
