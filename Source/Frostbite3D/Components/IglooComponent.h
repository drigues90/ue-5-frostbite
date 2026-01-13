// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IglooComponent.generated.h"

/**
 * Informações sobre um local de construção do iglu
 */
USTRUCT(BlueprintType)
struct FROSTBITE3D_API FIglooBuildSite
{
	GENERATED_BODY()

	/** Posição do local de construção no mundo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	/** Índice/ordem deste site (usado para construção sequencial) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BuildOrder = 0;

	/** Indica se este site já foi preenchido com um bloco */
	UPROPERTY(BlueprintReadOnly)
	bool bFilled = false;

	/** Referência ao bloco visual spawned neste site */
	UPROPERTY(BlueprintReadOnly)
	AActor* BlockActor = nullptr;

	FIglooBuildSite()
		: WorldLocation(FVector::ZeroVector)
		, BuildOrder(0)
		, bFilled(false)
		, BlockActor(nullptr)
	{
	}
};

/**
 * IglooComponent - Gerencia a coleta e construção do iglu
 * 
 * Responsabilidades:
 * - Rastrear blocos de gelo coletados pelo jogador
 * - Gerenciar locais de construção (BuildSites) do iglu
 * - Processar colocação de blocos nos BuildSites
 * - Detectar quando o iglu está completo
 * - Notificar GameMode sobre conclusão
 * 
 * Design:
 * - Blocos coletados são recursos consumíveis
 * - BuildSites podem ser configurados no editor ou criados proceduralmente
 * - Cada bloco colocado spawna uma representação visual no BuildSite
 * - Quando todos os RequiredBlocks são colocados, o iglu está completo
 * 
 * Baseado nas especificações do documento Gameplay-Mechanics.md
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FROSTBITE3D_API UIglooComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UIglooComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== Configurações ==========
	
	/** Número de blocos necessários para completar o iglu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo")
	int32 RequiredBlocks = 10;

	/** Distância máxima do jogador para um BuildSite para poder colocar bloco */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo")
	float BuildSiteInteractionDistance = 200.0f;

	/** Pontos ganhos ao coletar um bloco */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Scoring")
	int32 BlockCollectionScore = 100;

	/** Pontos ganhos ao colocar um bloco no iglu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Scoring")
	int32 BlockPlacementScore = 250;

	// ========== Estado Atual ==========
	
	/** Quantidade de blocos coletados (não colocados ainda) */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Frostbite|Igloo")
	int32 BlocksCollected = 0;

	/** Quantidade de blocos já colocados no iglu */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Frostbite|Igloo")
	int32 BlocksPlaced = 0;

	/** Indica se o iglu foi completado */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Igloo")
	bool bIglooComplete = false;

	// ========== Build Sites ==========
	
	/** Lista de locais onde blocos podem ser colocados */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo")
	TArray<FIglooBuildSite> BuildSites;

	/** Se true, cria BuildSites automaticamente em forma circular */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo")
	bool bAutoGenerateBuildSites = true;

	/** Raio do círculo de BuildSites (se auto-gerado) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo", meta=(EditCondition="bAutoGenerateBuildSites"))
	float BuildSiteRadius = 300.0f;

	/** Posição central do iglu (se auto-gerado) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Igloo", meta=(EditCondition="bAutoGenerateBuildSites"))
	FVector IglooCenterLocation = FVector::ZeroVector;

	// ========== Classes de Assets ==========
	
	/** Classe do bloco visual a spawnar nos BuildSites */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Assets")
	TSubclassOf<AActor> BlockVisualClass;

	// ========== Funções Públicas ==========
	
	/**
	 * Adiciona um bloco à coleção do jogador
	 * @return true se foi adicionado com sucesso
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Igloo")
	bool AddBlock();

	/**
	 * Tenta colocar um bloco no BuildSite mais próximo
	 * @param PlayerLocation - Posição atual do jogador
	 * @return true se conseguiu colocar o bloco
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Igloo")
	bool TryPlaceBlockAtNearestSite(const FVector& PlayerLocation);

	/**
	 * Verifica se o iglu está completo
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Igloo")
	bool IsComplete() const { return bIglooComplete; }

	/**
	 * Obtém quantidade de blocos coletados
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Igloo")
	int32 GetBlocksCollected() const { return BlocksCollected; }

	/**
	 * Obtém quantidade de blocos colocados
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Igloo")
	int32 GetBlocksPlaced() const { return BlocksPlaced; }

	/**
	 * Obtém progresso do iglu (0.0 a 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Igloo")
	float GetIglooProgress() const;

	/**
	 * Reseta o componente para começar novo nível
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Igloo")
	void ResetIgloo();

protected:
	// ========== Funções Internas ==========
	
	/**
	 * Gera BuildSites automaticamente em formação circular
	 */
	void GenerateBuildSites();

	/**
	 * Encontra o BuildSite não preenchido mais próximo de uma posição
	 * @return Índice do BuildSite ou -1 se nenhum encontrado
	 */
	int32 FindNearestUnfilledBuildSite(const FVector& Location, float MaxDistance);

	/**
	 * Coloca um bloco em um BuildSite específico
	 */
	bool PlaceBlockAtSite(int32 SiteIndex);

	/**
	 * Spawna visual do bloco em um BuildSite
	 */
	void SpawnBlockVisualAtSite(int32 SiteIndex);

	/**
	 * Verifica e processa conclusão do iglu
	 */
	void CheckIglooCompletion();

	/**
	 * Notifica GameMode sobre conclusão do iglu
	 */
	void NotifyIglooComplete();

	// ========== Replicação ==========
	
	/** Configuração de replicação */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server RPC para adicionar bloco */
	UFUNCTION(Server, Reliable)
	void Server_AddBlock();

	/** Server RPC para colocar bloco */
	UFUNCTION(Server, Reliable)
	void Server_PlaceBlockAtSite(int32 SiteIndex);

	/** Multicast RPC para efeitos visuais */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnBlockPlaced(int32 SiteIndex);

	// ========== Delegates (para expansão futura) ==========
	
	/** Delegate disparado quando blocos coletados mudam */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlocksCollectedChanged, int32, NewCount);
	// FOnBlocksCollectedChanged OnBlocksCollectedChanged;

	/** Delegate disparado quando bloco é colocado */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlockPlaced, int32, BlocksPlaced, int32, RequiredBlocks);
	// FOnBlockPlaced OnBlockPlaced;

	/** Delegate disparado quando iglu é completado */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIglooComplete);
	// FOnIglooComplete OnIglooComplete;
};
