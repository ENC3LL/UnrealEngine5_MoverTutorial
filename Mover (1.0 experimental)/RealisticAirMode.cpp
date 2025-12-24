#include "RealisticAirMode.h"
#include "RealisticMovementSettings.h"
#include "RealisticDataModel.h"
#include "RealisticMovementDefines.h"
#include "MoverComponent.h"
#include "RealisticMoverComponent.h"

URealisticAirMode::URealisticAirMode()
{
    SharedSettingsClasses.Add(URealisticMovementSettings::StaticClass());
}

void URealisticAirMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
    const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    const FRealisticMoverInputCmd* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FRealisticMoverInputCmd>();

    const URealisticMovementSettings* Settings = nullptr;
    if (const URealisticMoverComponent* MyMover = Cast<URealisticMoverComponent>(GetMoverComponent()))
    {
        Settings = MyMover->RealisticSettings;
    }
    if (!Settings)
    {
        Settings = GetMoverComponent()->FindSharedSettings<URealisticMovementSettings>();
    }

    if (!SyncState || !Settings) return;

    float DeltaSeconds = TimeStep.StepMs * 0.001f;
    FVector CurrentVelocity = SyncState->GetVelocity_WorldSpace();

    // 1. ГРАВИТАЦИЯ (УСИЛЕННАЯ)
    // Берем мировую гравитацию (-980) и умножаем на GravityScale (например 2.0)
    // Получится -1960. Это заставит персонажа падать быстро и тяжело.
    float WorldGravityZ = GetWorld()->GetGravityZ();
    FVector GravityAccel = FVector(0, 0, WorldGravityZ * Settings->GravityScale);

    FVector GravityForce = GravityAccel * Settings->Mass;

    // 2. Сопротивление воздуха
    float Speed = CurrentVelocity.Size();
    FVector DragForce = FVector::ZeroVector;

    if (Speed > 1.0f)
    {
        float DragMagnitude = Settings->AirDragCoef * Speed * Speed;
        DragForce = -CurrentVelocity.GetSafeNormal() * DragMagnitude;
    }

    // 3. Управление в воздухе
    FVector InputForce = FVector::ZeroVector;
    if (Inputs)
    {
        FVector MoveInput = Inputs->GetMoveInput();
        MoveInput.Z = 0.f;
        InputForce = MoveInput * Settings->AirControlForce;
    }

    FVector NetForce = GravityForce + DragForce + InputForce;
    FVector Acceleration = NetForce / Settings->Mass;

    OutProposedMove.LinearVelocity = CurrentVelocity + (Acceleration * DeltaSeconds);

    // Terminal Velocity (Максимальная скорость падения)
    // Если падаем быстрее 5300 см/с (190 км/ч), ограничиваем
    if (OutProposedMove.LinearVelocity.Z < -5300.0f)
    {
        OutProposedMove.LinearVelocity.Z = -5300.0f;
    }

    if (Inputs) OutProposedMove.DirectionIntent = Inputs->GetMoveInput();
}

void URealisticAirMode::SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState)
{
    FMoverDefaultSyncState& OutputSyncState = OutputState.SyncState.SyncStateCollection.FindOrAddMutableDataByType<FMoverDefaultSyncState>();
    const FMoverDefaultSyncState* StartingSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
    check(StartingSyncState);

    float DeltaTime = Params.TimeStep.StepMs * 0.001f;
    FQuat CurrentRotation = StartingSyncState->GetOrientation_WorldSpace().Quaternion();

    FHitResult Hit;
    Params.MovingComps.UpdatedComponent->MoveComponent(Params.ProposedMove.LinearVelocity * DeltaTime, CurrentRotation, true, &Hit);

    if (Hit.IsValidBlockingHit())
    {
        // Если ударились об пол (нормаль вверх) -> переходим в ходьбу
        if (Hit.Normal.Z >= 0.71f)
        {
            OutputState.MovementEndState.NextModeName = RealisticModes::Walk;
        }
        else
        {
            // Скользим по стене
            FVector SlideDelta = FVector::VectorPlaneProject(Params.ProposedMove.LinearVelocity * DeltaTime, Hit.Normal) * (1.0f - Hit.Time);
            Params.MovingComps.UpdatedComponent->MoveComponent(SlideDelta, CurrentRotation, true, nullptr);
        }
    }

    OutputSyncState.SetTransforms_WorldSpace(
        Params.MovingComps.UpdatedComponent->GetComponentLocation(),
        Params.MovingComps.UpdatedComponent->GetComponentRotation(),
        Params.ProposedMove.LinearVelocity,
        FVector::ZeroVector);
}