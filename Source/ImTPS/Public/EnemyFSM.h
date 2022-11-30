// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyFSM.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle State"),
	Move,
	Attack,
	Damage,
	Die
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IMTPS_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FSM")
	EEnemyState mState = EEnemyState::Idle;

	UPROPERTY(EditAnywhere, Category="FSM")
	float IdleDelayTime = 2;
	float CurrentTime = 0;
	// 타겟
	UPROPERTY()
	class ATPSPlayer* Target;

	// 컴포넌트의 소유자
	UPROPERTY()
	class AEnemy* ME;

	// 공격범위
	UPROPERTY(EditAnywhere, Category=FSM)
	float AttackRange = 200;
	// 필요속성 : 공격대기시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float AttackDelayTime = 2;

	// 체력
	UPROPERTY(EditDefaultsOnly, Category = FSM)
	int32 Hp = 3;
	
	// 필요속성 : 피격대기시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float DamageDelayTime = 2;

public:
	// 일정시간 기다렸다가 상태를 Move 로 전환
	void IdleState();
	void MoveState();
	void AttackState();
	void DamageState();
	// 피격 받았을 때 호출되는 이벤트함수
	void OnDamageProcess();

	void DieState();
};
