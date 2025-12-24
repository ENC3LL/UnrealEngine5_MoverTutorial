#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MoverComponent.h" 
#include "RealisticMovementSettings.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class GAME_API URealisticMovementSettings : public UObject, public IMovementSettingsInterface
{
    GENERATED_BODY()

public:
    URealisticMovementSettings();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 80.0f;

    // --- ГРАВИТАЦИЯ ---
    // 1.0 = Земля (медленно для игр). 
    // 2.0 - 3.0 = Шутер/Экшен (резкое падение).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
    float GravityScale = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
    float MaxWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
    float SprintSpeedMult = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
    float BrakingDeceleration = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
    float MaxWalkForce = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk")
    float GroundFriction = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
    float JumpImpulseForce = 45000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
    float CrouchSpeedMult = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
    float AirDragCoef = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air")
    float AirControlForce = 500.0f;

    virtual FString GetDisplayName() const override { return TEXT("Realistic Physics"); }
};