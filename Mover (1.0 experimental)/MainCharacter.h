#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MoverSimulationTypes.h"
#include "MoverTypes.h"
#include "MainCharacter.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class USpringArmComponent;
class UMoverComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class GAME_API AMainCharacter : public APawn, public IMoverInputProducerInterface
{
    GENERATED_BODY()

public:
    AMainCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Генерация ввода для Mover
    virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void CheckFloorPhysics(float& OutFrictionMult);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAME|Components")
    TObjectPtr<UCapsuleComponent> CapsuleComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAME|Components")
    TObjectPtr<USpringArmComponent> SpringArmComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAME|Components")
    TObjectPtr<UCameraComponent> CameraComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAME|Movement")
    TObjectPtr<UMoverComponent> MoverComp;

    // --- INPUT ---
    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, Category = "GAME|Input")
    TObjectPtr<UInputAction> CrouchAction;

    UPROPERTY(EditAnywhere, Category = "GAME|Camera")
    float LookSensitivity = 1.0f;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    void OnJumpStarted(const FInputActionValue& Value);
    void OnSprintStarted(const FInputActionValue& Value);
    void OnSprintEnded(const FInputActionValue& Value);
    void OnCrouchStarted(const FInputActionValue& Value);
    void OnCrouchEnded(const FInputActionValue& Value);

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GAME|Input")
    FVector LastMoveInput = FVector::ZeroVector;

    bool bRequestJump = false;
    bool bRequestSprint = false;
    bool bRequestCrouch = false;
};