#include "RealisticMoverComponent.h"
#include "RealisticWalkMode.h"
#include "RealisticAirMode.h"
#include "RealisticMovementDefines.h"

URealisticMoverComponent::URealisticMoverComponent()
{
    // Регистрируем режимы
    MovementModes.Add(RealisticModes::Walk, CreateDefaultSubobject<URealisticWalkMode>(TEXT("RealisticWalkMode")));
    MovementModes.Add(RealisticModes::Air, CreateDefaultSubobject<URealisticAirMode>(TEXT("RealisticAirMode")));

    // Стартовый режим
    StartingMovementMode = RealisticModes::Air;

    // Создаем настройки, чтобы они были видны в Blueprint
    RealisticSettings = CreateDefaultSubobject<URealisticMovementSettings>(TEXT("RealisticSettings"));
}
