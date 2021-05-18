// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "WheeledVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class SINGLEVEHICLEMOTION_API AWheeledVehiclePawn : public AWheeledVehicle
{
public:
	AWheeledVehiclePawn();

	// step function
	virtual void Tick(float DeltaTime) override;

	// Vehilce Movemnt Handle via Input
	virtual void SetUpPlayerInputComponent(UInputComponent* PlayerInputComponent); //override;

	// Throttle
	void applyThrottle(float val);

	// Steering
	void applySteering(float val);

	// Look up & Turn
	void lookUp(float val);
	void turn(float val);

	// Handbrake
	void onHandBrakePressed();
	void onHandBrakeReleased();

	// Move the vehicle around the air
	void updateAirControl(float DeltaTime);

private:
	GENERATED_BODY()

protected:
	// Spring arm - offset the camera
	UPROPERTY(Category = camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* springArm;

	// Camera componenet - viewpoint
	UPROPERTY(Category = camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* camera;

};
