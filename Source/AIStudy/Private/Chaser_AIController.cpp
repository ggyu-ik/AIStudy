#include "Chaser_AIController.h"
#include "GameFramework/Character.h"

AChaser_AIController::AChaser_AIController()
{
    // 매 프레임 틱 활성화
    PrimaryActorTick.bCanEverTick = true;

    // 시야 감지 설정 생성
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRadius;
    SightConfig->LoseSightRadius = LoseInterestRadius;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    
    // 부모 클래스의 PerceptionComponent에 시야 설정 추가


}

void AChaser_AIController::BeginPlay()
{
    Super::BeginPlay();
    
    // 인지 컴포넌트 초기화 후 컴포넌트 세팅
    if (SightConfig && GetPerceptionComponent())
    {

        GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
		    GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
    // 인지 이벤트에 델리게이트 바인딩
        GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AChaser_AIController::OnPerceptionUpdated);
    }
    
    
    // 기본 타겟으로 플레이어 설정 (선택적)
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        TargetActor = PlayerCharacter;  // ACharacter*는 AActor*로 암시적으로 변환 가능
    }
}


// Tick 이벤트는 아래 코드로 변경해주세요. 기존 코드와 유사하지만 디버그 시각화를 추가했습니다.
void AChaser_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 상태 업데이트 추가
    UpdateAIState();
    

    if (bIsChasing && TargetActor)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
            
            if (Distance <= ChaseRadius)
            {
                MoveToActor(TargetActor, 100.0f);
                
                // 마지막 위치 갱신 추가
                LastKnownLocation = TargetActor->GetActorLocation();
                
                // 디버그 시각화 추가
                #if WITH_EDITOR
                DrawDebugLine(
                    GetWorld(),
                    ControlledPawn->GetActorLocation(),
                    TargetActor->GetActorLocation(),
                    FColor::Red,
                    false,
                    -1.0f,
                    0,
                    2.0f
                );
                #endif
            }
            else if (Distance > LoseInterestRadius)  // 확장된 조건
            {
                StopChasing();
            }
        }
    }
}

// StartChasing과 StopChasing도 업데이트 해주세요
void AChaser_AIController::StartChasing(AActor* Target)
{
    TargetActor = Target;
    bIsChasing = true;
    
    if (Target)
    {
        // 마지막 위치 업데이트 추가
        LastKnownLocation = Target->GetActorLocation();
    }
    
    // 상태 변경 추가
    CurrentState = EAIState::Chasing;
}

void AChaser_AIController::StopChasing()
{
    bIsChasing = false;
    StopMovement();
    
    // 상태 변경 추가
    CurrentState = EAIState::Idle;
}


// Status별 상태 전환 함수를 추가해줍니다.
void AChaser_AIController::UpdateAIState()
{
    if (!TargetActor) return;
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;
    
    float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
    
    switch (CurrentState)
    {
        case EAIState::Idle:
            if (DistanceToTarget <= DetectionRadius)
            {
                CurrentState = EAIState::Suspicious;
            }
            break;
            
        case EAIState::Suspicious:
            if (DistanceToTarget <= ChaseRadius)
            {
                StartChasing(TargetActor);
            }
            else if (DistanceToTarget > DetectionRadius)
            {
                CurrentState = EAIState::Idle;
            }
            break;
            
        case EAIState::Chasing:
            if (DistanceToTarget > LoseInterestRadius)
            {
                StopChasing();
            }
            break;
    }
}

// 인지 시스템의 이벤트 발생시 처리하는 함수 추가.
void AChaser_AIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // 플레이어 캐릭터인지 확인
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    
    if (Actor && PlayerCharacter && Actor == PlayerCharacter)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // 플레이어 감지 성공
            TargetActor = Actor;
            
            // 거리에 따라 상태 변경
            APawn* ControlledPawn = GetPawn();
            if (ControlledPawn)
            {
                float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Actor->GetActorLocation());
                
                if (Distance <= ChaseRadius)
                {
                    StartChasing(Actor);
                }
                else if (Distance <= DetectionRadius)
                {
                    CurrentState = EAIState::Suspicious;
                }
            }
        }
        else
        {
            // 플레이어 감지 실패 (시야에서 사라짐)
            if (CurrentState == EAIState::Chasing)
            {
                // 마지막으로 본 위치로 이동
                MoveToLocation(LastKnownLocation, 50.0f);
                
                // 의심 상태로 전환
                CurrentState = EAIState::Suspicious;
            }
        }
    }
}