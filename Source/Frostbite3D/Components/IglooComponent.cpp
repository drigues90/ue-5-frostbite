// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#include "IglooComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "../GameMode/FrostbiteGameMode.h"

UIglooComponent::UIglooComponent()
{
	// Não precisa de Tick por padrão
	PrimaryComponentTick.bCanEverTick = false;

	// Habilitar replicação
	SetIsReplicatedByDefault(true);
}

void UIglooComponent::BeginPlay()
{
	Super::BeginPlay();

	// Gerar BuildSites se configurado para auto-gerar
	if (bAutoGenerateBuildSites)
	{
		GenerateBuildSites();
	}

	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Inicializado. Requer %d blocos para completar"), RequiredBlocks);
}

bool UIglooComponent::AddBlock()
{
	// Validação no servidor em ambiente multiplayer
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_AddBlock();
		return true;
	}

	BlocksCollected++;
	
	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Bloco coletado! Total: %d"), BlocksCollected);

	// TODO: Disparar delegate OnBlocksCollectedChanged

	// TODO: Adicionar pontuação ao jogador
	if (AFrostbiteGameMode* GameMode = Cast<AFrostbiteGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->AddScore(BlockCollectionScore);
	}

	// TODO: Reproduzir som/efeito de coleta

	return true;
}

bool UIglooComponent::TryPlaceBlockAtNearestSite(const FVector& PlayerLocation)
{
	// Verificar se tem blocos disponíveis
	if (BlocksCollected <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("IglooComponent: Sem blocos para colocar!"));
		return false;
	}

	// Verificar se iglu já está completo
	if (bIglooComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("IglooComponent: Iglu já está completo!"));
		return false;
	}

	// Encontrar BuildSite mais próximo
	int32 NearestSiteIndex = FindNearestUnfilledBuildSite(PlayerLocation, BuildSiteInteractionDistance);
	
	if (NearestSiteIndex == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("IglooComponent: Nenhum BuildSite disponível próximo"));
		return false;
	}

	// Validação no servidor em ambiente multiplayer
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_PlaceBlockAtSite(NearestSiteIndex);
		return true;
	}

	// Colocar bloco no site encontrado
	return PlaceBlockAtSite(NearestSiteIndex);
}

float UIglooComponent::GetIglooProgress() const
{
	if (RequiredBlocks <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(BlocksPlaced) / static_cast<float>(RequiredBlocks);
}

void UIglooComponent::ResetIgloo()
{
	BlocksCollected = 0;
	BlocksPlaced = 0;
	bIglooComplete = false;

	// Limpar BuildSites
	for (FIglooBuildSite& Site : BuildSites)
	{
		Site.bFilled = false;
		
		// Destruir bloco visual se existir
		if (Site.BlockActor)
		{
			Site.BlockActor->Destroy();
			Site.BlockActor = nullptr;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Resetado para novo nível"));
}

void UIglooComponent::GenerateBuildSites()
{
	BuildSites.Empty();

	// Criar BuildSites em formação circular ao redor do centro do iglu
	const float AngleStep = 360.0f / RequiredBlocks;
	
	for (int32 i = 0; i < RequiredBlocks; ++i)
	{
		FIglooBuildSite NewSite;
		
		// Calcular posição em círculo
		// AngleStep está em graus, convertemos para radianos para usar com Sin/Cos
		float Angle = FMath::DegreesToRadians(AngleStep * i);
		FVector Offset;
		Offset.X = FMath::Cos(Angle) * BuildSiteRadius;
		Offset.Y = FMath::Sin(Angle) * BuildSiteRadius;
		Offset.Z = 0.0f;
		
		NewSite.WorldLocation = IglooCenterLocation + Offset;
		NewSite.BuildOrder = i;
		NewSite.bFilled = false;
		
		BuildSites.Add(NewSite);
	}

	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Gerados %d BuildSites em formação circular"), BuildSites.Num());
}

int32 UIglooComponent::FindNearestUnfilledBuildSite(const FVector& Location, float MaxDistance)
{
	int32 NearestIndex = -1;
	float NearestDistanceSq = MaxDistance * MaxDistance;

	for (int32 i = 0; i < BuildSites.Num(); ++i)
	{
		const FIglooBuildSite& Site = BuildSites[i];
		
		// Pular sites já preenchidos
		if (Site.bFilled)
		{
			continue;
		}

		float DistanceSq = FVector::DistSquared(Location, Site.WorldLocation);
		
		if (DistanceSq < NearestDistanceSq)
		{
			NearestDistanceSq = DistanceSq;
			NearestIndex = i;
		}
	}

	return NearestIndex;
}

bool UIglooComponent::PlaceBlockAtSite(int32 SiteIndex)
{
	if (!BuildSites.IsValidIndex(SiteIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("IglooComponent: Índice de BuildSite inválido: %d"), SiteIndex);
		return false;
	}

	FIglooBuildSite& Site = BuildSites[SiteIndex];

	// Verificar se site já está preenchido
	if (Site.bFilled)
	{
		UE_LOG(LogTemp, Warning, TEXT("IglooComponent: BuildSite %d já está preenchido"), SiteIndex);
		return false;
	}

	// Consumir um bloco
	BlocksCollected = FMath::Max(0, BlocksCollected - 1);
	BlocksPlaced++;

	// Marcar site como preenchido
	Site.bFilled = true;

	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Bloco colocado no site %d! Progresso: %d/%d"), 
	       SiteIndex, BlocksPlaced, RequiredBlocks);

	// Spawnar visual do bloco
	SpawnBlockVisualAtSite(SiteIndex);

	// Adicionar pontuação
	if (AFrostbiteGameMode* GameMode = Cast<AFrostbiteGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->AddScore(BlockPlacementScore);
	}

	// Notificar clientes em multiplayer
	Multicast_OnBlockPlaced(SiteIndex);

	// TODO: Disparar delegate OnBlockPlaced

	// Verificar se completou o iglu
	CheckIglooCompletion();

	return true;
}

void UIglooComponent::SpawnBlockVisualAtSite(int32 SiteIndex)
{
	if (!BuildSites.IsValidIndex(SiteIndex))
	{
		return;
	}

	FIglooBuildSite& Site = BuildSites[SiteIndex];

	// TODO: Spawnar actor visual do bloco
	// if (BlockVisualClass && GetWorld())
	// {
	//     FActorSpawnParameters SpawnParams;
	//     SpawnParams.Owner = GetOwner();
	//     SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//     
	//     AActor* BlockActor = GetWorld()->SpawnActor<AActor>(
	//         BlockVisualClass, 
	//         Site.WorldLocation, 
	//         FRotator::ZeroRotator, 
	//         SpawnParams
	//     );
	//     
	//     Site.BlockActor = BlockActor;
	//     
	//     // TODO: Reproduzir animação de colocação (slide in, scale up, etc)
	// }

	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Visual do bloco spawned no site %d"), SiteIndex);
}

void UIglooComponent::CheckIglooCompletion()
{
	// Verificar se alcançou o número necessário de blocos
	if (BlocksPlaced >= RequiredBlocks && !bIglooComplete)
	{
		bIglooComplete = true;
		NotifyIglooComplete();
	}
}

void UIglooComponent::NotifyIglooComplete()
{
	UE_LOG(LogTemp, Log, TEXT("IglooComponent: IGLU COMPLETO!"));

	// TODO: Disparar delegate OnIglooComplete

	// Notificar GameMode
	if (AFrostbiteGameMode* GameMode = Cast<AFrostbiteGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->OnIglooComplete();
	}

	// TODO: Reproduzir efeitos de vitória (partículas, sons, animação)
}

void UIglooComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicar propriedades importantes para clientes
	DOREPLIFETIME(UIglooComponent, BlocksCollected);
	DOREPLIFETIME(UIglooComponent, BlocksPlaced);
}

void UIglooComponent::Server_AddBlock_Implementation()
{
	AddBlock();
}

void UIglooComponent::Server_PlaceBlockAtSite_Implementation(int32 SiteIndex)
{
	PlaceBlockAtSite(SiteIndex);
}

void UIglooComponent::Multicast_OnBlockPlaced_Implementation(int32 SiteIndex)
{
	// TODO: Reproduzir efeitos visuais/sonoros em todos os clientes
	UE_LOG(LogTemp, Log, TEXT("IglooComponent: Efeito de bloco colocado no site %d (multicast)"), SiteIndex);
}
