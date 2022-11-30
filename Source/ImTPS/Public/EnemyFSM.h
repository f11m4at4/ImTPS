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
	// Ÿ��
	UPROPERTY()
	class ATPSPlayer* Target;

	// ������Ʈ�� ������
	UPROPERTY()
	class AEnemy* ME;

	// ���ݹ���
	UPROPERTY(EditAnywhere, Category=FSM)
	float AttackRange = 200;
	// �ʿ�Ӽ� : ���ݴ��ð�
	UPROPERTY(EditAnywhere, Category = FSM)
	float AttackDelayTime = 2;

	// ü��
	UPROPERTY(EditDefaultsOnly, Category = FSM)
	int32 Hp = 3;
	
	// �ʿ�Ӽ� : �ǰݴ��ð�
	UPROPERTY(EditAnywhere, Category = FSM)
	float DamageDelayTime = 2;

public:
	// �����ð� ��ٷȴٰ� ���¸� Move �� ��ȯ
	void IdleState();
	void MoveState();
	void AttackState();
	void DamageState();
	// �ǰ� �޾��� �� ȣ��Ǵ� �̺�Ʈ�Լ�
	void OnDamageProcess();

	void DieState();
};
