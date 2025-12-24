#include "RealisticWalkMode.h"
#include "RealisticMovementSettings.h"
#include "RealisticDataModel.h"
#include "RealisticMovementDefines.h"
#include "MoverComponent.h"
#include "MoveLibrary/MovementUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "RealisticMoverComponent.h"
#include "Engine/Engine.h"

URealisticWalkMode::URealisticWalkMode()
{
}

float GetSurfaceFriction(const USceneComponent* UpdatedComponent, UWorld* World)
{
    if (!UpdatedComponent || !World) return 1.0f;
    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector End = Start - FVector(0, 0, 150.0f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(UpdatedComponent->GetOwner());
    Params.bReturnPhysicalMaterial = true;

    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (Hit.PhysMaterial.IsValid())
        {
            return FMath::Max(Hit.PhysMaterial->Friction, 0.1f);
        }
    }
    return 1.0f;
}

void URealisticWalkMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
    const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    const FRealisticMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FRealisticMoverInputCmd>();

    const URealisticMovementSettings* Settings = nullptr;
    if (const URealisticMoverComponent* MyMover = Cast<URealisticMoverComponent>(GetMoverComponent()))
    {
        Settings = MyMover->RealisticSettings;
    }
    if (!Settings) Settings = GetMoverComponent()->FindSharedSettings<URealisticMovementSettings>();

    if (!SyncState || !Settings || !Inputs) return;

    float DeltaSeconds = TimeStep.StepMs * 0.001f;
    FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();
    CurrentVelocity.Z = 0.f;

    FVector MoveInput = Inputs->GetMoveInput();

    // Мертвая зона
    if (MoveInput.Size() < 0.1f) MoveInput = FVector::ZeroVector;

    FVector MoveIntent = MoveInput.GetSafeNormal();
    float SurfaceFriction = GetSurfaceFriction(GetMoverComponent()->GetUpdatedComponent(), GetWorld());
    float Speed = CurrentVelocity.Size();
    bool bHasInput = !MoveIntent.IsNearlyZero();

    // Расчет направления относительно камеры (для замедления назад)
    // ControlRotation - куда смотрит камера/игрок
    FVector LookDir = Inputs->ControlRotation.Vector().GetSafeNormal2D();
    float ForwardDot = 0.0f;
    if (bHasInput)
    {
        ForwardDot = (MoveIntent | LookDir); // 1.0 = вперед, -1.0 = назад
    }

    // ОТЛАДКА СКОРОСТИ И РЕЖИМОВ
    if (GEngine)
    {
        FString MoveState = TEXT("Run");
        if (Inputs->bIsSprintPressed) MoveState = TEXT("SPRINT");
        if (Inputs->bIsCrouchPressed) MoveState = TEXT("CROUCH");
        if (ForwardDot < -0.5f) MoveState += TEXT(" (BACKWARDS)");

        FString DebugMsg = FString::Printf(TEXT("STATE: %s | Speed: %.0f / %.0f"),
            *MoveState,
            Speed,
            Settings->MaxWalkSpeed); // Показываем базовую макс скорость

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Cyan, DebugMsg);
    }

    bool bIsMoving = Speed > 1.0f;
    float Alignment = 0.0f;
    if (bIsMoving && bHasInput) Alignment = (CurrentVelocity.GetSafeNormal() | MoveIntent);
    bool bIsOpposing = bIsMoving && bHasInput && (Alignment < -0.1f);

    // --- 1. ТРЕНИЕ ---
    float FrictionForce = Settings->GroundFriction * SurfaceFriction;
    CurrentVelocity = CurrentVelocity / (1.0f + (FrictionForce * DeltaSeconds));

    // --- 2. ТОРМОЖЕНИЕ ---
    if (!bHasInput || bIsOpposing)
    {
        float BaseBrake = (Settings->BrakingDeceleration < 10.0f) ? 4000.0f : Settings->BrakingDeceleration;
        float BrakingMagnitude = BaseBrake;

        if (bIsOpposing) BrakingMagnitude *= 2.0f;
        else BrakingMagnitude *= 5.0f;

        float EffectiveFriction = FMath::Max(SurfaceFriction, 1.0f);
        BrakingMagnitude *= EffectiveFriction;

        float NewSpeed = FMath::Max(0.0f, Speed - (BrakingMagnitude * DeltaSeconds));

        if (Speed > 1.0f) CurrentVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
        else CurrentVelocity = FVector::ZeroVector;
    }

    // --- 3. РАЗГОН ---
    bool bCanAccelerate = bHasInput;
    if (bIsOpposing && CurrentVelocity.Size() > 50.0f) bCanAccelerate = false;

    if (bCanAccelerate)
    {
        float TargetMaxSpeed = Settings->MaxWalkSpeed;
        float PushForce = (Settings->MaxWalkForce < 100.0f) ? 192000.0f : Settings->MaxWalkForce;

        // --- ЛОГИКА МОДИФИКАТОРОВ ---

        // 1. ПРИСЕД (Приоритет №1)
        if (Inputs->bIsCrouchPressed)
        {
            TargetMaxSpeed *= Settings->CrouchSpeedMult; // 0.5 -> 300
        }
        // 2. СПРИНТ (Приоритет №2, если не сидим)
        else if (Inputs->bIsSprintPressed)
        {
            float SprintMult = (Settings->SprintSpeedMult > 0.0f) ? Settings->SprintSpeedMult : 1.5f;
            TargetMaxSpeed *= SprintMult;
        }

        // 3. БЕГ НАЗАД (Штраф 40%)
        // Если Dot Product < -0.5, значит мы идем назад
        if (ForwardDot < -0.5f)
        {
            TargetMaxSpeed *= 0.6f; // Замедляем до 60%
        }

        // --- АВТО-МАСШТАБИРОВАНИЕ СИЛЫ (FIX STUCK AT 320) ---
        // Если трение сильное, нам нужна сила больше, чем 192000.
        // F_needed = Mass * Friction * TargetSpeed
        float RequiredForce = Settings->Mass * Settings->GroundFriction * TargetMaxSpeed;

        // Берем либо настройку пользователя, либо вычисленную физически силу (что больше)
        // Чтобы точно достичь скорости TargetMaxSpeed
        if (PushForce < RequiredForce)
        {
            PushForce = RequiredForce * 1.2f; // +20% запаса для ускорения
        }
        // -------------------------------------------------------------

        PushForce *= SurfaceFriction;

        FVector Acceleration = (MoveIntent * PushForce) / Settings->Mass;
        FVector NewVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);

        // Soft Clamp
        if (NewVelocity.SizeSquared() > FMath::Square(TargetMaxSpeed))
        {
            if (NewVelocity.SizeSquared() > CurrentVelocity.SizeSquared())
            {
                NewVelocity = NewVelocity.GetSafeNormal() * TargetMaxSpeed;
            }
        }
        CurrentVelocity = NewVelocity;
    }

    if (CurrentVelocity.SizeSquared() < 1.0f && !bHasInput) CurrentVelocity = FVector::ZeroVector;

    OutProposedMove.LinearVelocity = CurrentVelocity;
    OutProposedMove.DirectionIntent = MoveIntent;

    if (Inputs->bIsJumpPressed)
    {
        OutProposedMove.LinearVelocity.Z = Settings->JumpImpulseForce / Settings->Mass;
    }
}

// Tick оставляем
void URealisticWalkMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
    FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
    const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    check(StartingSyncState);

    float DeltaTime = Params.TimeStep.StepMs * 0.001f;
    FVector ProposedVelocity = Params.ProposedMove.LinearVelocity;
    FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

    if (ProposedVelocity.Z > 10.0f)
    {
        OutputState.MovementEndState.NextModeName = RealisticModes::Air;
        FHitResult Hit;
        Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, CurrentRotation, true, &Hit);
        if (Hit.IsValidBlockingHit())
        {
            FVector Slide = FVector::VectorPlaneProject(ProposedVelocity, Hit.Normal);
            ProposedVelocity = Slide;
        }
        OutputSyncState.SetTransforms_WorldSpace(Params.MovingComps.UpdatedComponent->GetComponentLocation(), Params.MovingComps.UpdatedComponent->GetComponentRotation(), ProposedVelocity, FVector::ZeroVector);
        return;
    }

    FHitResult Hit;
    Params.MovingComps.UpdatedComponent->MoveComponent(ProposedVelocity * DeltaTime, CurrentRotation, true, &Hit);

    if (Hit.IsValidBlockingHit())
    {
        FMovementRecord MoveRecord;
        MoveRecord.SetDeltaSeconds(DeltaTime);
        UMovementUtils::TryMoveToSlideAlongSurface(Params.MovingComps, ProposedVelocity * DeltaTime, 1.0f - Hit.Time, CurrentRotation, Hit.Normal, Hit, true, MoveRecord);
        ProposedVelocity = MoveRecord.GetRelevantVelocity();
    }

    FFloorCheckResult FloorResult;
    UFloorQueryUtils::FindFloor(Params.MovingComps, 50.0f, 0.71f, true, Params.MovingComps.UpdatedComponent->GetComponentLocation(), FloorResult);

    if (FloorResult.bWalkableFloor)
    {
        if (FloorResult.FloorDist > 0.1f) Params.MovingComps.UpdatedComponent->MoveComponent(FVector(0, 0, -FloorResult.FloorDist), CurrentRotation, true, nullptr);
    }
    else
    {
        OutputState.MovementEndState.NextModeName = RealisticModes::Air;
    }

    OutputSyncState.SetTransforms_WorldSpace(Params.MovingComps.UpdatedComponent->GetComponentLocation(), Params.MovingComps.UpdatedComponent->GetComponentRotation(), ProposedVelocity, FVector::ZeroVector);
}