// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "ImTPS.h"
#include <Kismet/GameplayStatics.h>
#include "TPSPlayer.h"
#include "Enemy.h"

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Target = Cast<ATPSPlayer>(UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass()));

	ME = Cast<AEnemy>(GetOwner());
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UEnum* enumValue = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	FString str = enumValue->GetNameStringByIndex((int32)mState);
	PRINTLOG(TEXT("State : %s"), *str);

	switch (mState)
	{
	case EEnemyState::Idle:
		IdleState();
		break;
	case EEnemyState::Move:
		MoveState();
		break;
	case EEnemyState::Attack:
		AttackState();
		break;
	case EEnemyState::Damage:
		DamageState();
		break;
	case EEnemyState::Die:
		DieState();
		break;
	}
}

void UEnemyFSM::IdleState()
{
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	if (CurrentTime > IdleDelayTime)
	{
		CurrentTime = 0;
		mState = EEnemyState::Move;
	}
}

// Ÿ�������� �̵��ϰ� �ʹ�.
void UEnemyFSM::MoveState()
{
	// ����
	// Direction = Target - me
	FVector Direction = Target->GetActorLocation() - ME->GetActorLocation();

	ME->AddMovementInput(Direction.GetSafeNormal());

	// ���ݹ����ȿ� ������ ���¸� �������� ��ȯ�ϰ� �ʹ�.
	// 1. ���ݹ����ȿ� �������ϱ�
	if (Direction.Size() < AttackRange)
	{
		// 2. ���¸� �������� ��ȯ
		mState = EEnemyState::Attack;
	}
}

// �����ð��� �ѹ��� �����ϰ� �ʹ�.
void UEnemyFSM::AttackState()
{
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	if (CurrentTime > AttackDelayTime)
	{
		CurrentTime = 0;
		PRINTLOG(TEXT("Attack!!"));
	}

	// Ÿ���� ���ݹ����� ����� ���¸� �̵����� ��ȯ��Ű�� �ʹ�.
	float Distance = FVector::Dist(Target->GetActorLocation(), ME->GetActorLocation());
	if (Distance > AttackRange)
	{
		mState = EEnemyState::Move;
	}
}

// �����ð� ������ ���¸� ���� ��ȯ�ϰ� �ʹ�.
void UEnemyFSM::DamageState()
{
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	if (CurrentTime > DamageDelayTime)
	{
		CurrentTime = 0;
		mState = EEnemyState::Idle;
	}
}

void UEnemyFSM::OnDamageProcess()
{
	Hp--;
	if (Hp > 0)
	{
		CurrentTime = 0;
		mState = EEnemyState::Damage;
	}
	else
	{
		ME->Destroy();
	}
}

void UEnemyFSM::DieState()
{

}

