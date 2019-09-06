// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "FPSGameMode.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnPawnHeard);

	GuardState = EAIState::Idle;
}


// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();

	OriginalRotation = GetActorRotation();
}


/* Method for when Guard sees Player */
void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if (SeenPawn == nullptr) { return; }

	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Red, false, 10.0f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);
}


/* Edit Guard direction based on noise heard */
void AFPSAIGuard::OnPawnHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	//If Guard state is alerted we dont need noises
	if (GuardState == EAIState::Alerted) { return; }
	SetGuardState(EAIState::Suspicious);
	
	//Debug
	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	//get direction to look at
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	//Make Guard look at new direction without changing Z axis
	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;
	SetActorRotation(NewLookAt);

	//Timer for how long to look before looking back at original dir
		//Reset timer if one was already made
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
		//Make new timer
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);
}


/* Method for reseting Guard orientation */
void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted) { return; }
	SetGuardState(EAIState::Idle);

	SetActorRotation(OriginalRotation);
}


/* Sets the Guard State */
void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	//If already set to this state then don't set
	if (GuardState == NewState) { return; }

	//Set state
	GuardState = NewState;

	OnStateChanged(GuardState);
}


// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


