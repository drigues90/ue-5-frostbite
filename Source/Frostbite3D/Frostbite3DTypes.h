// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Frostbite3DTypes.generated.h"

/**
 * Coordenada de um tile na grade
 * 
 * Estrutura simples para representar posições discretas na grade 2D de tiles.
 * Usada tanto pelo TileManager quanto pelo Character para comunicação sobre posições.
 */
USTRUCT(BlueprintType)
struct FROSTBITE3D_API FTileCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;

	FTileCoord() : X(0), Y(0) {}
	FTileCoord(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FTileCoord& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	bool operator!=(const FTileCoord& Other) const
	{
		return !(*this == Other);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("(%d, %d)"), X, Y);
	}
};

/**
 * Estados possíveis de um tile
 */
UENUM(BlueprintType)
enum class ETileState : uint8
{
	Intact UMETA(DisplayName = "Intacto"),
	Cracked UMETA(DisplayName = "Rachado"),
	Broken UMETA(DisplayName = "Quebrado"),
	Frozen UMETA(DisplayName = "Congelado"),
	Occupied UMETA(DisplayName = "Ocupado")
};

/**
 * Estados possíveis do jogo
 */
UENUM(BlueprintType)
enum class EFrostbiteGameState : uint8
{
	Initializing UMETA(DisplayName = "Inicializando"),
	Running UMETA(DisplayName = "Em Execução"),
	Paused UMETA(DisplayName = "Pausado"),
	Won UMETA(DisplayName = "Vitória"),
	Lost UMETA(DisplayName = "Derrota")
};
