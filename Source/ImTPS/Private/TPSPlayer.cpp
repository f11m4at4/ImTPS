// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "ImTPS.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <Blueprint/UserWidget.h>
#include <Kismet/GameplayStatics.h>
#include "EnemyFSM.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 스캘레탈메시 데이터 할당
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}

	// 카메라 관련 컴포넌트 붙이기
	// 1. SpringArm
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->SetRelativeLocation(FVector(0, 70, 90));
	SpringArmComp->bUsePawnControlRotation = true;
	// 2. camera
	TPSCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamComp"));
	TPSCamComp->SetupAttachment(SpringArmComp);
	TPSCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	JumpMaxCount = 2;
	// Crosshair 를 불러와서 할당
	ConstructorHelpers::FClassFinder<UUserWidget> TempClass(TEXT("WidgetBlueprint'/Game/Blueprints/WBP_Crosshair.WBP_Crosshair_C'"));
	if (TempClass.Succeeded())
	{
		CrosshairFactory = TempClass.Class;
	}
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	_CrosshairUI = CreateWidget(GetWorld(), CrosshairFactory);
	_CrosshairUI->AddToViewport();
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	Direction = FTransform(GetControlRotation()).TransformVector(Direction);
	AddMovementInput(Direction);
	Direction = FVector::ZeroVector;
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ATPSPlayer::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ATPSPlayer::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ATPSPlayer::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ATPSPlayer::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ATPSPlayer::Jump);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATPSPlayer::FireInput);
}

void ATPSPlayer::MoveRight(float Value)
{
	Direction.Y = Value;
	/*FVector Direction = GetActorRightVector();
	AddMovementInput(Direction, Value);*/
}

void ATPSPlayer::MoveForward(float Value)
{
	Direction.X = Value;
	/*FVector Direction = GetActorForwardVector();
	AddMovementInput(Direction, Value);*/
}

void ATPSPlayer::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATPSPlayer::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATPSPlayer::FireInput()
{
	// LineTrace 시작위치
	FVector StartPos = TPSCamComp->GetComponentLocation();
	FVector EndPos = StartPos + TPSCamComp->GetForwardVector() * 5000;

	FHitResult HitInfo;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPos, EndPos, ECC_Visibility, Params);
	if (bHit)
	{
		// 부딪힌 지점에 효과 발생시키기
		FTransform BulletTrans;
		BulletTrans.SetLocation(HitInfo.ImpactPoint);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletEffectFactory, BulletTrans);

		// 부딪힌 녀석을 날려보내고 싶다.
		// 1. 만약 부딪힌 녀석의 컴포넌트가 물리가 켜져있다면
		auto Comp = HitInfo.GetComponent();
		if (Comp && Comp->IsSimulatingPhysics())
		{
			FVector Force = TPSCamComp->GetForwardVector() * Comp->GetMass() * 5000;

			Comp->AddImpulseAtLocation(Force, HitInfo.ImpactPoint);
		}

		// 만약 부딪힌 녀석이 Enemy 이면
		//HitInfo.GetActor()->GetName().Contains("Enemy")
		auto Enemy = HitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
		if (Enemy)
		{
			// -> OnDamageProcess 호출해주기
			auto FSM = Cast<UEnemyFSM>(Enemy);
			FSM->OnDamageProcess();
		}
	}
}

