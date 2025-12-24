#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
// Важный инклюд, чтобы видеть настройки
#include "RealisticMovementSettings.h" 
#include "RealisticMoverComponent.generated.h"

UCLASS(ClassGroup = (Mover), meta = (BlueprintSpawnableComponent))
class GAME_API URealisticMoverComponent : public UMoverComponent
{
    GENERATED_BODY()

public:
    URealisticMoverComponent();

    UPROPERTY(EditAnywhere, Instanced, Category = "Realistic Movement")
    TObjectPtr<URealisticMovementSettings> RealisticSettings;

};