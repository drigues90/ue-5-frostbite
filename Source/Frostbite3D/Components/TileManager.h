// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TileManager.generated.h"

// Forward declaration
struct FTileCoord;

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
 * Estrutura que representa informações de um tile individual
 */
USTRUCT(BlueprintType)
struct FTileInfo
{
	GENERATED_BODY()

	/** Coordenada do tile na grade */
	UPROPERTY(BlueprintReadOnly)
	FTileCoord Coord;

	/** Posição no mundo 3D */
	UPROPERTY(BlueprintReadOnly)
	FVector WorldPosition = FVector::ZeroVector;

	/** Estado atual do tile */
	UPROPERTY(BlueprintReadOnly)
	ETileState State = ETileState::Intact;

	/** Tempo desde que foi pisado (usado para quebra) */
	UPROPERTY(BlueprintReadOnly)
	float TimeSinceStepped = 0.0f;

	/** Indica se está ocupado por algum ator */
	UPROPERTY(BlueprintReadOnly)
	bool bOccupied = false;

	/** Referência ao ator ocupante (pode ser jogador ou inimigo) */
	UPROPERTY(BlueprintReadOnly)
	AActor* Occupant = nullptr;

	/** Índice no HISM (Hierarchical Instanced Static Mesh) se usando instancing */
	UPROPERTY(BlueprintReadOnly)
	int32 InstancedIndex = -1;

	/** Indica se este tile pode quebrar quando pisado */
	UPROPERTY(BlueprintReadOnly)
	bool bBreakable = true;

	FTileInfo()
		: Coord(FTileCoord(0, 0))
		, WorldPosition(FVector::ZeroVector)
		, State(ETileState::Intact)
		, TimeSinceStepped(0.0f)
		, bOccupied(false)
		, Occupant(nullptr)
		, InstancedIndex(-1)
		, bBreakable(true)
	{
	}
};

/**
 * TileManager - Gerenciador da grade de tiles do jogo
 * 
 * Responsabilidades:
 * - Criar e inicializar a grade de tiles no início do nível
 * - Gerenciar estados dos tiles (Intact, Cracked, Broken, etc)
 * - Processar timers de quebra de tiles de forma eficiente
 * - Fornecer API para consulta e manipulação de tiles
 * - Spawnar pickups quando tiles são destruídos
 * - Renderizar tiles usando HISM para performance
 * 
 * Design:
 * - Usa um array 2D de FTileInfo para armazenar dados dos tiles
 * - Usa HISM (Hierarchical Instanced Static Mesh) para renderização eficiente
 * - Processa timers de quebra centralizadamente no Tick para evitar overhead
 * 
 * Baseado nas especificações do documento Gameplay-Mechanics.md
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FROSTBITE3D_API UTileManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UTileManager();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== Configurações da Grade ==========
	
	/** Tamanho da grade em X (colunas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Grid")
	int32 GridSizeX = 8;

	/** Tamanho da grade em Y (linhas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Grid")
	int32 GridSizeY = 6;

	/** Espaçamento entre centros dos tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Grid")
	float TileSpacing = 120.0f;

	/** Variação de altura dos tiles para realismo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Grid")
	float TileHeightVariance = 8.0f;

	/** Origem da grade no mundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Grid")
	FVector GridOrigin = FVector::ZeroVector;

	// ========== Configurações de Quebra de Tiles ==========
	
	/** Tempo até começar a rachar após ser pisado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Tiles")
	float TimeToBreakAfterStepped = 1.2f;

	/** Tempo até remover completamente após rachar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Tiles")
	float TimeToRemoveAfterCrack = 0.8f;

	/** Chance de spawnar pickup quando tile é destruído (0.0 a 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Tiles")
	float CollectibleDropChance = 0.12f;

	// ========== Componente HISM (para renderização eficiente) ==========
	
	/** Componente para renderizar tiles usando instancing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frostbite|Rendering")
	class UHierarchicalInstancedStaticMeshComponent* TileHISM;

	// ========== Funções Públicas de Inicialização ==========
	
	/**
	 * Inicializa a grade de tiles
	 * @param SizeX - Número de colunas
	 * @param SizeY - Número de linhas
	 * @param Spacing - Distância entre tiles
	 * @param Origin - Posição de origem da grade
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Grid")
	void InitializeGrid(int32 SizeX, int32 SizeY, float Spacing, FVector Origin);

	// ========== Funções de Consulta ==========
	
	/**
	 * Verifica se uma coordenada é válida na grade
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Grid")
	bool IsValidCoord(const FTileCoord& Coord) const;

	/**
	 * Obtém informações de um tile específico
	 * @return Ponteiro para FTileInfo ou nullptr se inválido
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Grid")
	FTileInfo* GetTileInfo(const FTileCoord& Coord);

	/**
	 * Obtém tile na posição do mundo
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Grid")
	FTileInfo* GetTileAtWorldLocation(const FVector& Location);

	/**
	 * Converte posição do mundo para coordenada de tile
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Grid")
	FTileCoord WorldLocationToTileCoord(const FVector& WorldLocation) const;

	/**
	 * Converte coordenada de tile para posição do mundo
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Grid")
	FVector TileCoordToWorldLocation(const FTileCoord& Coord) const;

	// ========== Funções de Manipulação ==========
	
	/**
	 * Notifica que um ator pisou em um tile
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Tiles")
	void OnActorStepped(AActor* Actor, const FTileCoord& Coord);

	/**
	 * Quebra um tile específico
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Tiles")
	void BreakTile(const FTileCoord& Coord);

	/**
	 * Restaura um tile quebrado
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Tiles")
	void RestoreTile(const FTileCoord& Coord);

	/**
	 * Força quebra imediata de um tile (por dano externo)
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Tiles")
	void ForceBreakTile(const FTileCoord& Coord);

	// ========== Funções de Navegação ==========
	
	/**
	 * Obtém tiles vizinhos (adjacentes e diagonais)
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Navigation")
	TArray<FTileCoord> GetNeighbors(const FTileCoord& Coord, bool bIncludeDiagonals = true);

	/**
	 * Verifica se um tile é alcançável de outro
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Navigation")
	bool IsReachable(const FTileCoord& From, const FTileCoord& To);

protected:
	// ========== Dados Internos ==========
	
	/** Array 2D de tiles (armazenado como array 1D) */
	TArray<FTileInfo> TileGrid;

	/** Lista de tiles ativos que precisam de processamento de timer */
	TArray<FTileCoord> ActiveTimerTiles;

	// ========== Funções Internas ==========
	
	/**
	 * Processa timers de quebra de tiles
	 */
	void ProcessTileTimers(float DeltaTime);

	/**
	 * Atualiza estado visual de um tile
	 */
	void UpdateTileVisual(const FTileCoord& Coord);

	/**
	 * Spawna pickup em uma posição
	 */
	void SpawnPickupAtLocation(const FVector& Location);

	/**
	 * Calcula índice 1D a partir de coordenada 2D
	 */
	int32 CoordToIndex(const FTileCoord& Coord) const;

	/**
	 * Converte índice 1D para coordenada 2D
	 */
	FTileCoord IndexToCoord(int32 Index) const;

	// ========== Delegates (para expansão futura) ==========
	
	/** Delegate disparado quando estado de tile muda */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileStateChanged, FTileCoord, Coord, ETileState, NewState);
	// FOnTileStateChanged OnTileStateChanged;

	/** Delegate disparado quando tile é quebrado */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileBroken, FTileCoord, Coord);
	// FOnTileBroken OnTileBroken;
};
