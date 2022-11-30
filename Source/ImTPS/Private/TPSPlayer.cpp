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

	// ��Ķ��Ż�޽� ������ �Ҵ�
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}

	// ī�޶� ���� ������Ʈ ���̱�
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
	// Crosshair �� �ҷ��ͼ� �Ҵ�
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
	// LineTrace ������ġ
	FVector StartPos = TPSCamComp->GetComponentLocation();
	FVector EndPos = StartPos + TPSCamComp->GetForwardVector() * 5000;

	FHitResult HitInfo;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPos, EndPos, ECC_Visibility, Params);
	if (bHit)
	{
		// �ε��� ������ ȿ�� �߻���Ű��
		FTransform BulletTrans;
		BulletTrans.SetLocation(HitInfo.ImpactPoint);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletEffectFactory, BulletTrans);

		// �ε��� �༮�� ���������� �ʹ�.
		// 1. ���� �ε��� �༮�� ������Ʈ�� ������ �����ִٸ�
		auto Comp = HitInfo.GetComponent();
		if (Comp && Comp->IsSimulatingPhysics())
		{
			FVector Force = TPSCamComp->GetForwardVector() * Comp->GetMass() * 5000;

			Comp->AddImpulseAtLocation(Force, HitInfo.ImpactPoint);
		}

		// ���� �ε��� �༮�� Enemy �̸�
		//HitInfo.GetActor()->GetName().Contains("Enemy")
		auto Enemy = HitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
		if (Enemy)
		{
			// -> OnDamageProcess ȣ�����ֱ�
			auto FSM = Cast<UEnemyFSM>(Enemy);
			FSM->OnDamageProcess();
		}
	}
}

