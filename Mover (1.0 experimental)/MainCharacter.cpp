#include "MainCharacter.h" 
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MoverComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Исправленные пути, чтобы компилятор видел файлы в соседней папке Movement
#include "../Movement/RealisticWalkMode.h"
#include "../Movement/RealisticAirMode.h"
#include "../Movement/RealisticDataModel.h"
#include "../Movement/RealisticMovementDefines.h"

AMainCharacter::AMainCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->InitCapsuleSize(34.0f, 88.0f);
    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(CapsuleComp);
    SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));
    SpringArmComp->TargetArmLength = 0.0f;
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->bEnableCameraLag = true;
    SpringArmComp->bEnableCameraRotationLag = false;
    SpringArmComp->CameraLagSpeed = 30.0f;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp);
    CameraComp->bUsePawnControlRotation = false;

    // Лучше использовать URealisticMoverComponent, если вы создали его в C++, 
    // но стандартный UMoverComponent тоже подойдет, если мы правильно настроим его в BeginPlay
    MoverComp = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComp"));

    bReplicates = true;
    SetReplicateMovement(false);
}

void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();

    // --- ДОБАВЛЕНИЕ INPUT MAPPING CONTEXT ---
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // --- РЕГИСТРАЦИЯ РЕЖИМОВ (ВАЖНО!) ---
    if (UMoverComponent* Mover = MoverComp)
    {
        // Очищаем старые, если были
        Mover->MovementModes.Empty();

        // Создаем новые режимы. Используем имена из RealisticMovementDefines
        Mover->MovementModes.Add(RealisticModes::Walk, NewObject<URealisticWalkMode>(this));
        Mover->MovementModes.Add(RealisticModes::Air, NewObject<URealisticAirMode>(this));

        // Удаляем явные переходы (Transitions), они теперь встроенны в логику режимов
        Mover->Transitions.Empty();

        // Устанавливаем стартовый режим
        Mover->QueueNextMode(RealisticModes::Air);
    }
}

void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            // Вызывается каждый кадр, пока держишь кнопку
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);

            // Вызывается ОДИН раз, когда ты отпускаешь кнопку. Value будет (0,0), и персонаж остановится.
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMainCharacter::Move);
        }

        if (LookAction)
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainCharacter::Look);

        if (JumpAction)
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMainCharacter::OnJumpStarted);

        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AMainCharacter::OnSprintStarted);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMainCharacter::OnSprintEnded);
        }

        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMainCharacter::OnCrouchStarted);
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AMainCharacter::OnCrouchEnded);
        }
    }
}

void AMainCharacter::Move(const FInputActionValue& Value)
{
    FVector2D InputVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        LastMoveInput = (ForwardDirection * InputVector.X) + (RightDirection * InputVector.Y);
        LastMoveInput.Normalize();
    }
}

void AMainCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X * LookSensitivity);
        AddControllerPitchInput(LookAxisVector.Y * LookSensitivity);
    }
}

void AMainCharacter::OnJumpStarted(const FInputActionValue& Value) { bRequestJump = true; }
void AMainCharacter::OnSprintStarted(const FInputActionValue& Value) { bRequestSprint = true; }
void AMainCharacter::OnSprintEnded(const FInputActionValue& Value) { bRequestSprint = false; }
void AMainCharacter::OnCrouchStarted(const FInputActionValue& Value) { bRequestCrouch = true; }
void AMainCharacter::OnCrouchEnded(const FInputActionValue& Value) { bRequestCrouch = false; }


// --- ГЛАВНАЯ ФУНКЦИЯ ВВОДА ---
void AMainCharacter::ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
    // Получаем или создаем структуру ввода правильного типа
    FRealisticMoverInputCmd& CharacterInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FRealisticMoverInputCmd>();

    // Обработка физики поверхности (опционально, ваш старый код)
    float FrictionMult = 1.0f;
    CheckFloorPhysics(FrictionMult);

    FVector EffectiveInput = LastMoveInput;
    if (FrictionMult < 0.5f)
    {
        EffectiveInput *= 0.2f; // Скользкий лед
    }

    // Заполняем данные
    CharacterInputs.ControlRotation = GetControlRotation();

    // В Mover 2.0 OrientationIntent и MoveInput разделены
    if (!EffectiveInput.IsNearlyZero())
    {
        CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, EffectiveInput);
        CharacterInputs.OrientationIntent = EffectiveInput; // Смотрим туда, куда идем
    }
    else
    {
        CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, FVector::ZeroVector);
        // Если стоим, не меняем ориентацию (или можно использовать ControlRotation.Vector())
        CharacterInputs.OrientationIntent = GetActorForwardVector();
    }

    CharacterInputs.bIsJumpPressed = bRequestJump;
    CharacterInputs.bIsCrouchPressed = bRequestCrouch;
    CharacterInputs.bIsSprintPressed = bRequestSprint;

    // Сбрасываем разовые триггеры
    bRequestJump = false;
}

void AMainCharacter::CheckFloorPhysics(float& OutFrictionMult)
{
    OutFrictionMult = 1.0f;
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0, 0, 100.0f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bReturnPhysicalMaterial = true;

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (Hit.PhysMaterial.IsValid())
        {
            OutFrictionMult = Hit.PhysMaterial->Friction;
        }
    }
}