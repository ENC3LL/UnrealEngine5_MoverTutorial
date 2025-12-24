#pragma once

#include "CoreMinimal.h"
#include "MoverDataModelTypes.h"
#include "RealisticDataModel.generated.h"

USTRUCT(BlueprintType)
struct GAME_API FRealisticMoverInputCmd : public FCharacterDefaultInputs
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = Mover)
    bool bIsCrouchPressed = false;

    UPROPERTY(BlueprintReadWrite, Category = Mover)
    bool bIsSprintPressed = false;

    virtual FMoverDataStructBase* Clone() const override
    {
        return new FRealisticMoverInputCmd(*this);
    }

    virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
    {
        Super::NetSerialize(Ar, Map, bOutSuccess);
        // Сериализуем флаги (битовая маска для экономии трафика)
        uint8 Flags = (bIsCrouchPressed << 0) | (bIsSprintPressed << 1);
        Ar.SerializeBits(&Flags, 2);

        if (Ar.IsLoading())
        {
            bIsCrouchPressed = (Flags & (1 << 0)) != 0;
            bIsSprintPressed = (Flags & (1 << 1)) != 0;
        }

        return true;
    }

    virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
};