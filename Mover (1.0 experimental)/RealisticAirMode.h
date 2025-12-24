#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "RealisticAirMode.generated.h"

UCLASS()
class GAME_API URealisticAirMode : public UBaseMovementMode
{
    GENERATED_BODY()
public:
    URealisticAirMode();

    virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
    virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
};