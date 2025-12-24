#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "RealisticWalkMode.generated.h"

UCLASS()
class GAME_API URealisticWalkMode : public UBaseMovementMode
{
    GENERATED_BODY()
public:
    URealisticWalkMode();

    // ВАЖНО для UE 5.7:
    // 1. Имя функции без "On"
    // 2. Суффикс _Implementation
    // 3. GenerateMove ОБЯЗАН быть const
    virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

    // 4. SimulationTick НЕ const
    virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
};