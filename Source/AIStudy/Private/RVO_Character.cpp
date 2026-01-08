#include "RVO_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

// Sets default values
ARVO_Character::ARVO_Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// RVO 회피 시스템 활성화
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = AvoidanceRadius;
		MovementComponent->AvoidanceWeight = 0.5f;
	}
}

// Called when the game starts or when spawned
void ARVO_Character::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 참조 얻기
	AIController = Cast<AAIController>(GetController());

	// AI 컨트롤러가 없으면 로그 출력
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is not controlled by an AIController. Movement functions will not work."), *GetName());
	}
	else if (TargetActor)
	{
		// 타겟 액터가 설정되어 있으면 자동으로 이동 시작
		MoveToTarget();
	}
}

// Called every frame
void ARVO_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARVO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARVO_Character::MoveToTarget()
{
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget failed: No AI Controller for %s"), *GetName());
		return;
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget failed: No Target Actor set for %s"), *GetName());
		return;
	}

	// 타겟 액터를 향해 이동
	AIController->MoveToActor(
		TargetActor,    // 목표 액터
		50.0f,          // 도착 판정 반경
		true,           // 충돌 영역이 겹치면 도착으로 간주
		true,           // 경로 탐색 사용
		false           // 목적지를 네비게이션 메시에 투영(Projection)하지 않음
	);

	UE_LOG(LogTemp, Display, TEXT("%s moving to target: %s"),
		*GetName(), *TargetActor->GetName());
}

void ARVO_Character::SetRVOAvoidanceEnabled(bool bEnable)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bUseRVOAvoidance = bEnable;
	}
}