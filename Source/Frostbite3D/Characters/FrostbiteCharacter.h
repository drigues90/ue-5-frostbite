// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FrostbiteCharacter.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Coordenada de um tile na grade
 */
USTRUCT(BlueprintType)
struct FTileCoord
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
};

/**
 * Character principal do jogador no Frostbite 3D
 * 
 * Responsabilidades:
 * - Movimentação baseada em grid de tiles com snap preciso
 * - Sistema de salto entre tiles adjacentes
 * - Coleta de blocos de gelo (pickups)
 * - Interação com tiles (OnSteppedOn)
 * - Gerenciamento de vidas e dano
 * - Animações: Idle, Walk, Jump, Land, Fall, Hurt
 * 
 * Mecânicas principais:
 * - Salto com snap ao centro do tile alvo
 * - Detecção de queda quando não há tile
 * - Invulnerabilidade temporária após respawn
 * - Integração com IglooComponent para construção
 * 
 * Baseado nas especificações do documento Gameplay-Mechanics.md
 */
UCLASS()
class FROSTBITE3D_API AFrostbiteCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFrostbiteCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

public:
	// ========== Componentes ==========
	
	/** Spring Arm para câmera de terceira pessoa */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frostbite|Camera")
	USpringArmComponent* CameraBoom;

	/** Câmera de terceira pessoa */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frostbite|Camera")
	UCameraComponent* FollowCamera;

	/** Componente de gerenciamento do iglu */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Frostbite|Igloo")
	class UIglooComponent* IglooComponent;

	// ========== Input Mapping (Enhanced Input System) ==========
	
	/** Contexto de mapeamento de input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frostbite|Input")
	UInputMappingContext* DefaultMappingContext;

	/** Ação de movimento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frostbite|Input")
	UInputAction* MoveAction;

	/** Ação de salto */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frostbite|Input")
	UInputAction* JumpAction;

	/** Ação de olhar (câmera) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frostbite|Input")
	UInputAction* LookAction;

	/** Ação de interação (colocar bloco no iglu) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Frostbite|Input")
	UInputAction* InteractAction;

	// ========== Configurações de Gameplay ==========
	
	/** Vidas do jogador */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Frostbite|Stats")
	int32 Lives = 3;

	/** Vidas máximas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Stats")
	int32 MaxLives = 3;

	/** Tolerância de snap horizontal ao centro do tile (em unidades UE) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Movement")
	float JumpHorizontalSnapTolerance = 40.0f;

	/** Duração da invulnerabilidade após respawn (em segundos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Combat")
	float RespawnInvulnerabilityDuration = 1.0f;

	/** Indica se o jogador está invulnerável */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Combat")
	bool bIsInvulnerable = false;

	// ========== Estado Atual ==========
	
	/** Coordenada do tile atual onde o jogador está */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Grid")
	FTileCoord CurrentTileCoord;

	/** Indica se o jogador está em um tile válido */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Grid")
	bool bIsOnValidTile = false;

	// ========== Funções Públicas ==========
	
	/**
	 * Tenta saltar para um tile específico
	 * @param TargetCoord - Coordenada do tile de destino
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Movement")
	void AttemptJumpToTile(const FTileCoord& TargetCoord);

	/**
	 * Chamado quando o jogador aterra em um tile
	 * @param TileCoord - Coordenada do tile onde aterrou
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Grid")
	void OnLandedOnTile(const FTileCoord& TileCoord);

	/**
	 * Aplicar dano ao jogador
	 * @param DamageAmount - Quantidade de vidas a remover
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Combat")
	void TakeDamage(int32 DamageAmount = 1);

	/**
	 * Respawn do jogador no checkpoint
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Respawn")
	void RespawnPlayer();

	/**
	 * Coleta um bloco de gelo
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Collectibles")
	void CollectIceBlock();

protected:
	// ========== Funções de Input ==========
	
	/** Callback para movimento */
	void Move(const FInputActionValue& Value);

	/** Callback para câmera */
	void Look(const FInputActionValue& Value);

	/** Callback para interação */
	void Interact(const FInputActionValue& Value);

	// ========== Funções Internas ==========
	
	/**
	 * Realiza snap à posição do tile
	 * @param TileWorldLocation - Posição do tile no mundo
	 */
	void SnapToTileCenter(const FVector& TileWorldLocation);

	/**
	 * Verifica se está caindo fora do mapa
	 */
	void CheckForFalling();

	/**
	 * Inicia invulnerabilidade temporária
	 */
	void StartInvulnerability();

	/**
	 * Termina invulnerabilidade
	 */
	void EndInvulnerability();

	/**
	 * Timer handle para invulnerabilidade
	 */
	FTimerHandle InvulnerabilityTimerHandle;

	// ========== Replicação ==========
	
	/** Server RPC para validar salto entre tiles */
	UFUNCTION(Server, Reliable)
	void Server_RequestJumpToTile(const FTileCoord& TargetCoord);

	/** Configuração de replicação */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== Delegates (para expansão futura) ==========
	
	/** Delegate disparado quando vidas mudam */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, NewLives);
	// FOnLivesChanged OnLivesChanged;

	/** Delegate disparado quando blocos coletados mudam */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlocksCollectedChanged, int32, NewCount);
	// FOnBlocksCollectedChanged OnBlocksCollectedChanged;
};
