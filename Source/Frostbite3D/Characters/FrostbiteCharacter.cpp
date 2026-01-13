// Copyright (c) 2025 Frostbite 3D Team. Todos os direitos reservados.

#include "FrostbiteCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "../Components/IglooComponent.h"

AFrostbiteCharacter::AFrostbiteCharacter()
{
	// Configuração básica do Character
	PrimaryActorTick.bCanEverTick = true;

	// Configurar colisão da cápsula
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Não rotacionar com o controller, apenas a câmera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configurar movement component
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 420.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// Criar Spring Arm para câmera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Criar câmera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Criar componente de iglu
	IglooComponent = CreateDefaultSubobject<UIglooComponent>(TEXT("IglooComponent"));
}

void AFrostbiteCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Configurar Enhanced Input
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Iniciar com invulnerabilidade
	StartInvulnerability();

	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Inicializado com %d vidas"), Lives);
}

void AFrostbiteCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Configurar Enhanced Input Actions
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFrostbiteCharacter::Move);
		}

		// Jump
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Look
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFrostbiteCharacter::Look);
		}

		// Interact
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AFrostbiteCharacter::Interact);
		}
	}
}

void AFrostbiteCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Verificar se está caindo fora do mapa
	CheckForFalling();
}

void AFrostbiteCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Aterrissou em %s"), *Hit.GetActor()->GetName());

	// TODO: Determinar coordenada do tile baseado na posição
	// TODO: Chamar TileManager para notificar que pisou no tile
	// TODO: Fazer snap ao centro do tile se dentro da tolerância
}

void AFrostbiteCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Descobrir qual direção é "forward"
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Obter vetor forward
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		// Obter vetor right
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Adicionar movimento
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFrostbiteCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Adicionar yaw e pitch input ao controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFrostbiteCharacter::Interact(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Interact pressionado"));

	// TODO: Tentar colocar bloco no iglu se próximo de um build site
	if (IglooComponent)
	{
		// IglooComponent->TryPlaceBlockAtNearestSite(GetActorLocation());
	}
}

void AFrostbiteCharacter::AttemptJumpToTile(const FTileCoord& TargetCoord)
{
	// TODO: Implementar lógica de salto para tile específico
	// 1. Validar se o tile alvo é alcançável (adjacente ou diagonal)
	// 2. Calcular trajetória de salto
	// 3. Aplicar LaunchCharacter com vetor calculado
	
	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Tentando saltar para tile (%d, %d)"), TargetCoord.X, TargetCoord.Y);

	// Em multiplayer, validar no servidor
	if (!HasAuthority())
	{
		Server_RequestJumpToTile(TargetCoord);
	}
}

void AFrostbiteCharacter::OnLandedOnTile(const FTileCoord& TileCoord)
{
	CurrentTileCoord = TileCoord;
	bIsOnValidTile = true;

	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Agora no tile (%d, %d)"), TileCoord.X, TileCoord.Y);

	// TODO: Notificar TileManager que pisou neste tile
	// TODO: Fazer snap ao centro do tile
}

void AFrostbiteCharacter::TakeDamage(int32 DamageAmount)
{
	// Ignorar dano se invulnerável
	if (bIsInvulnerable)
	{
		UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Dano ignorado (invulnerável)"));
		return;
	}

	Lives = FMath::Max(0, Lives - DamageAmount);
	
	UE_LOG(LogTemp, Warning, TEXT("FrostbiteCharacter: Recebeu %d de dano! Vidas restantes: %d"), DamageAmount, Lives);

	// TODO: Disparar delegate OnLivesChanged
	// TODO: Aplicar knockback
	// TODO: Reproduzir animação/som de dano

	if (Lives <= 0)
	{
		// Jogador morreu
		UE_LOG(LogTemp, Error, TEXT("FrostbiteCharacter: Morreu!"));
		
		// TODO: Notificar GameMode
		// TODO: Reproduzir animação de morte
		// TODO: Desabilitar input temporariamente
	}
	else
	{
		// Ainda tem vidas, ativar invulnerabilidade temporária
		StartInvulnerability();
	}
}

void AFrostbiteCharacter::RespawnPlayer()
{
	// Restaurar uma vida (ou manter se já chegou a 0)
	if (Lives <= 0)
	{
		Lives = 1;
	}

	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Respawn! Vidas: %d"), Lives);

	// TODO: Teleportar para posição de respawn/checkpoint
	// TODO: Resetar estado do personagem
	// TODO: Limpar debuffs/status

	// Iniciar invulnerabilidade de respawn
	StartInvulnerability();

	// TODO: Reproduzir efeito visual de respawn
}

void AFrostbiteCharacter::CollectIceBlock()
{
	if (IglooComponent)
	{
		IglooComponent->AddBlock();
		
		UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Bloco coletado! Total: %d"), IglooComponent->GetBlocksCollected());
		
		// TODO: Reproduzir som/efeito de coleta
		// TODO: Adicionar pontuação (+100)
	}
}

void AFrostbiteCharacter::SnapToTileCenter(const FVector& TileWorldLocation)
{
	FVector CurrentLocation = GetActorLocation();
	
	// Calcular distância horizontal ao centro do tile
	FVector2D CurrentPos2D(CurrentLocation.X, CurrentLocation.Y);
	FVector2D TilePos2D(TileWorldLocation.X, TileWorldLocation.Y);
	float Distance = FVector2D::Distance(CurrentPos2D, TilePos2D);

	// Se dentro da tolerância, fazer snap
	if (Distance <= JumpHorizontalSnapTolerance)
	{
		FVector SnappedLocation = FVector(TileWorldLocation.X, TileWorldLocation.Y, CurrentLocation.Z);
		SetActorLocation(SnappedLocation);
		
		UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Snap ao centro do tile (distância: %.1f)"), Distance);
	}
}

void AFrostbiteCharacter::CheckForFalling()
{
	// Verificar se caiu abaixo de um limite Z
	const float FallDeathZ = -1000.0f; // Ajustar conforme necessário
	
	if (GetActorLocation().Z < FallDeathZ)
	{
		UE_LOG(LogTemp, Warning, TEXT("FrostbiteCharacter: Caiu fora do mapa!"));
		
		// Aplicar dano por queda
		TakeDamage(1);
		
		// Se ainda tem vidas, respawn
		if (Lives > 0)
		{
			RespawnPlayer();
		}
	}
}

void AFrostbiteCharacter::StartInvulnerability()
{
	bIsInvulnerable = true;
	
	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Invulnerabilidade ativada por %.1f segundos"), RespawnInvulnerabilityDuration);

	// TODO: Ativar efeito visual de invulnerabilidade (material piscante, etc)

	// Configurar timer para terminar invulnerabilidade
	GetWorldTimerManager().SetTimer(
		InvulnerabilityTimerHandle,
		this,
		&AFrostbiteCharacter::EndInvulnerability,
		RespawnInvulnerabilityDuration,
		false
	);
}

void AFrostbiteCharacter::EndInvulnerability()
{
	bIsInvulnerable = false;
	
	UE_LOG(LogTemp, Log, TEXT("FrostbiteCharacter: Invulnerabilidade desativada"));

	// TODO: Desativar efeito visual de invulnerabilidade
}

void AFrostbiteCharacter::Server_RequestJumpToTile_Implementation(const FTileCoord& TargetCoord)
{
	// Servidor valida se o salto é permitido
	// TODO: Verificar se tile alvo é alcançável e válido
	// TODO: Executar salto se válido
	
	AttemptJumpToTile(TargetCoord);
}

void AFrostbiteCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicar propriedades importantes para clientes
	DOREPLIFETIME(AFrostbiteCharacter, Lives);
}
