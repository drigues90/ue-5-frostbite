// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Frostbite3DTypes.h"
#include "FrostbiteGameMode.generated.h"

/**
 * GameMode principal do Frostbite 3D
 * 
 * Responsabilidades:
 * - Gerenciar estados de jogo (Running, Paused, Won, Lost)
 * - Controlar timer de fase/nível
 * - Processar condições de vitória (iglu completo) e derrota (vidas zeradas, tempo esgotado)
 * - Gerenciar transições entre níveis
 * - Spawnar jogador e inimigos
 * - Manter pontuação e estatísticas do jogador
 * 
 * Baseado nas especificações do documento Gameplay-Mechanics.md
 */
UCLASS()
class FROSTBITE3D_API AFrostbiteGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFrostbiteGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ========== Configurações de Nível ==========
	
	/** Tempo total do nível em segundos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Level")
	float LevelTime = 120.0f;

	/** Tempo restante no nível atual */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Level")
	float RemainingTime;

	/** Número de blocos necessários para completar o iglu */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frostbite|Level")
	int32 RequiredBlocksForVictory = 10;

	// ========== Estado do Jogo ==========
	
	/** Estado atual do jogo */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|State")
	EFrostbiteGameState CurrentGameState = EFrostbiteGameState::Initializing;

	/** Pontuação total do jogador */
	UPROPERTY(BlueprintReadOnly, Category = "Frostbite|Score")
	int32 PlayerScore = 0;

	// ========== Funções Públicas ==========
	
	/**
	 * Inicia o nível - inicializa timer e muda estado para Running
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Game")
	void StartLevel();

	/**
	 * Pausa ou despausa o jogo
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Game")
	void TogglePause();

	/**
	 * Chamado quando o jogador morre
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Game")
	void OnPlayerDeath();

	/**
	 * Chamado quando o iglu é completado
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Game")
	void OnIglooComplete();

	/**
	 * Adiciona pontos à pontuação do jogador
	 * @param Points - Quantidade de pontos a adicionar
	 */
	UFUNCTION(BlueprintCallable, Category = "Frostbite|Score")
	void AddScore(int32 Points);

	/**
	 * Verifica se o tempo do nível esgotou
	 */
	UFUNCTION(BlueprintPure, Category = "Frostbite|Game")
	bool IsTimeUp() const { return RemainingTime <= 0.0f; }

protected:
	// ========== Funções Internas ==========
	
	/**
	 * Atualiza o timer do nível
	 */
	void UpdateLevelTimer(float DeltaTime);

	/**
	 * Processa vitória do nível
	 */
	void ProcessLevelVictory();

	/**
	 * Processa derrota do nível
	 */
	void ProcessLevelDefeat();

	/**
	 * Calcula bônus de tempo para pontuação final
	 */
	int32 CalculateTimeBonusScore() const;

	// ========== Delegates (para expansão futura) ==========
	
	/** Delegate disparado quando o estado do jogo muda */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EFrostbiteGameState, NewState);
	// FOnGameStateChanged OnGameStateChanged;

	/** Delegate disparado quando a pontuação muda */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);
	// FOnScoreChanged OnScoreChanged;
};
