// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#include "FrostbiteGameMode.h"
#include "Kismet/GameplayStatics.h"

AFrostbiteGameMode::AFrostbiteGameMode()
{
	// Ativa Tick para este GameMode (necessário para atualizar timer)
	PrimaryActorTick.bCanEverTick = true;

	// TODO: Configurar classes padrão quando criadas
	// DefaultPawnClass = AFrostbiteCharacter::StaticClass();
	// HUDClass = AFrostbiteHUD::StaticClass();
}

void AFrostbiteGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Inicializa o tempo restante com o tempo total do nível
	RemainingTime = LevelTime;

	// TODO: Inicializar TileManager e outros sistemas
	// TODO: Spawnar inimigos iniciais

	// Inicia o nível após um pequeno delay para permitir inicializações
	FTimerHandle StartLevelTimerHandle;
	GetWorldTimerManager().SetTimer(StartLevelTimerHandle, this, &AFrostbiteGameMode::StartLevel, 1.0f, false);
}

void AFrostbiteGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Atualiza timer apenas se o jogo estiver em execução
	if (CurrentGameState == EFrostbiteGameState::Running)
	{
		UpdateLevelTimer(DeltaTime);
	}
}

void AFrostbiteGameMode::StartLevel()
{
	CurrentGameState = EFrostbiteGameState::Running;
	
	UE_LOG(LogTemp, Log, TEXT("Frostbite: Nível iniciado! Tempo: %.1f segundos"), LevelTime);
	
	// TODO: Disparar evento OnGameStateChanged
	// TODO: Notificar HUD para exibir "Level Start"
}

void AFrostbiteGameMode::TogglePause()
{
	if (CurrentGameState == EFrostbiteGameState::Running)
	{
		CurrentGameState = EFrostbiteGameState::Paused;
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		UE_LOG(LogTemp, Log, TEXT("Frostbite: Jogo pausado"));
	}
	else if (CurrentGameState == EFrostbiteGameState::Paused)
	{
		CurrentGameState = EFrostbiteGameState::Running;
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		UE_LOG(LogTemp, Log, TEXT("Frostbite: Jogo retomado"));
	}
}

void AFrostbiteGameMode::OnPlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Frostbite: Jogador morreu!"));

	// TODO: Verificar se o jogador ainda tem vidas
	// TODO: Se Lives > 0: respawn jogador
	// TODO: Se Lives == 0: ProcessLevelDefeat()
}

void AFrostbiteGameMode::OnIglooComplete()
{
	UE_LOG(LogTemp, Log, TEXT("Frostbite: Iglu completado!"));
	ProcessLevelVictory();
}

void AFrostbiteGameMode::AddScore(int32 Points)
{
	if (Points > 0)
	{
		PlayerScore += Points;
		UE_LOG(LogTemp, Log, TEXT("Frostbite: +%d pontos! Total: %d"), Points, PlayerScore);
		
		// TODO: Disparar delegate OnScoreChanged
		// TODO: Atualizar HUD
	}
}

void AFrostbiteGameMode::UpdateLevelTimer(float DeltaTime)
{
	RemainingTime -= DeltaTime;

	// Verifica se o tempo esgotou
	if (RemainingTime <= 0.0f)
	{
		RemainingTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("Frostbite: Tempo esgotado!"));
		ProcessLevelDefeat();
	}
	
	// TODO: Se tempo baixo (ex: < 20s), disparar alerta visual/sonoro
}

void AFrostbiteGameMode::ProcessLevelVictory()
{
	CurrentGameState = EFrostbiteGameState::Won;

	// Calcula pontuação de bônus por tempo restante
	int32 TimeBonus = CalculateTimeBonusScore();
	AddScore(TimeBonus);
	AddScore(1000); // Bônus por completar o nível

	UE_LOG(LogTemp, Log, TEXT("Frostbite: VITÓRIA! Pontuação final: %d (Bônus de tempo: %d)"), PlayerScore, TimeBonus);

	// TODO: Exibir tela de vitória
	// TODO: Salvar pontuação
	// TODO: Preparar transição para próximo nível
}

void AFrostbiteGameMode::ProcessLevelDefeat()
{
	CurrentGameState = EFrostbiteGameState::Lost;

	UE_LOG(LogTemp, Warning, TEXT("Frostbite: DERROTA! Pontuação final: %d"), PlayerScore);

	// TODO: Exibir tela de Game Over
	// TODO: Opções de retry ou menu principal
}

int32 AFrostbiteGameMode::CalculateTimeBonusScore() const
{
	// 10 pontos por segundo restante
	return FMath::FloorToInt(RemainingTime) * 10;
}
