// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#include "TileManager.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

UTileManager::UTileManager()
{
	// Ativa Tick para processar timers de quebra de tiles
	PrimaryComponentTick.bCanEverTick = true;

	// TODO: Criar HISM component quando houver um Owner Actor
	// O HISM precisa ser criado no BeginPlay quando tivermos acesso ao Owner
}

void UTileManager::BeginPlay()
{
	Super::BeginPlay();

	// Inicializar a grade automaticamente no BeginPlay
	if (GridSizeX > 0 && GridSizeY > 0)
	{
		InitializeGrid(GridSizeX, GridSizeY, TileSpacing, GridOrigin);
	}

	UE_LOG(LogTemp, Log, TEXT("TileManager: Inicializado com grade %dx%d"), GridSizeX, GridSizeY);
}

void UTileManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Processar timers de quebra de tiles
	ProcessTileTimers(DeltaTime);
}

void UTileManager::InitializeGrid(int32 SizeX, int32 SizeY, float Spacing, FVector Origin)
{
	GridSizeX = SizeX;
	GridSizeY = SizeY;
	TileSpacing = Spacing;
	GridOrigin = Origin;

	// Limpar grade existente
	TileGrid.Empty();
	ActiveTimerTiles.Empty();

	// Reservar espaço para performance
	TileGrid.Reserve(GridSizeX * GridSizeY);

	// Criar todos os tiles
	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			FTileInfo NewTile;
			NewTile.Coord = FTileCoord(X, Y);
			
			// Calcular posição no mundo
			FVector Position = Origin;
			Position.X += X * Spacing;
			Position.Y += Y * Spacing;
			
			// Adicionar pequena variação de altura para realismo
			Position.Z += FMath::RandRange(-TileHeightVariance, TileHeightVariance);
			
			NewTile.WorldPosition = Position;
			NewTile.State = ETileState::Intact;
			NewTile.bBreakable = true; // Pode ser configurado por tile específico depois

			TileGrid.Add(NewTile);

			// TODO: Adicionar instância ao HISM aqui
			// if (TileHISM)
			// {
			//     FTransform InstanceTransform(Position);
			//     int32 InstanceIndex = TileHISM->AddInstance(InstanceTransform);
			//     TileGrid.Last().InstancedIndex = InstanceIndex;
			// }
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TileManager: Grade inicializada com %d tiles"), TileGrid.Num());
}

bool UTileManager::IsValidCoord(const FTileCoord& Coord) const
{
	return Coord.X >= 0 && Coord.X < GridSizeX && Coord.Y >= 0 && Coord.Y < GridSizeY;
}

FTileInfo* UTileManager::GetTileInfo(const FTileCoord& Coord)
{
	if (!IsValidCoord(Coord))
	{
		return nullptr;
	}

	int32 Index = CoordToIndex(Coord);
	if (TileGrid.IsValidIndex(Index))
	{
		return &TileGrid[Index];
	}

	return nullptr;
}

FTileInfo* UTileManager::GetTileAtWorldLocation(const FVector& Location)
{
	FTileCoord Coord = WorldLocationToTileCoord(Location);
	return GetTileInfo(Coord);
}

FTileCoord UTileManager::WorldLocationToTileCoord(const FVector& WorldLocation) const
{
	FVector RelativePos = WorldLocation - GridOrigin;
	
	int32 X = FMath::RoundToInt(RelativePos.X / TileSpacing);
	int32 Y = FMath::RoundToInt(RelativePos.Y / TileSpacing);
	
	return FTileCoord(X, Y);
}

FVector UTileManager::TileCoordToWorldLocation(const FTileCoord& Coord) const
{
	if (!IsValidCoord(Coord))
	{
		return FVector::ZeroVector;
	}

	// Obter posição armazenada (que inclui variação de altura)
	int32 Index = CoordToIndex(Coord);
	if (TileGrid.IsValidIndex(Index))
	{
		return TileGrid[Index].WorldPosition;
	}

	// Fallback: calcular posição base
	FVector Position = GridOrigin;
	Position.X += Coord.X * TileSpacing;
	Position.Y += Coord.Y * TileSpacing;
	return Position;
}

void UTileManager::OnActorStepped(AActor* Actor, const FTileCoord& Coord)
{
	FTileInfo* Tile = GetTileInfo(Coord);
	if (!Tile)
	{
		UE_LOG(LogTemp, Warning, TEXT("TileManager: Tentou pisar em tile inválido (%d, %d)"), Coord.X, Coord.Y);
		return;
	}

	// Marcar tile como ocupado
	Tile->bOccupied = true;
	Tile->Occupant = Actor;

	UE_LOG(LogTemp, Log, TEXT("TileManager: Ator %s pisou no tile (%d, %d)"), *Actor->GetName(), Coord.X, Coord.Y);

	// Se tile está intacto e é breakable, iniciar processo de quebra
	if (Tile->State == ETileState::Intact && Tile->bBreakable)
	{
		Tile->TimeSinceStepped = 0.0f;
		
		// Adicionar à lista de tiles ativos se ainda não estiver
		ActiveTimerTiles.AddUnique(Coord);
	}

	// TODO: Disparar evento OnTileStepped
}

void UTileManager::BreakTile(const FTileCoord& Coord)
{
	FTileInfo* Tile = GetTileInfo(Coord);
	if (!Tile)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("TileManager: Tile (%d, %d) quebrado!"), Coord.X, Coord.Y);

	Tile->State = ETileState::Broken;
	
	// Remover da lista de timers ativos
	ActiveTimerTiles.Remove(Coord);

	// TODO: Atualizar visual (esconder instância do HISM ou trocar material)
	UpdateTileVisual(Coord);

	// TODO: Reproduzir efeitos (partículas, som)

	// Chance de spawnar pickup
	if (FMath::FRand() < CollectibleDropChance)
	{
		SpawnPickupAtLocation(Tile->WorldPosition);
	}

	// Se havia um ocupante, aplicar efeito (queda, knockback, etc)
	if (Tile->Occupant)
	{
		// TODO: Aplicar knockback ou iniciar queda no ocupante
		UE_LOG(LogTemp, Warning, TEXT("TileManager: Tile quebrou com ocupante %s"), *Tile->Occupant->GetName());
	}

	// TODO: Disparar evento OnTileBroken
}

void UTileManager::RestoreTile(const FTileCoord& Coord)
{
	FTileInfo* Tile = GetTileInfo(Coord);
	if (!Tile)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TileManager: Tile (%d, %d) restaurado"), Coord.X, Coord.Y);

	Tile->State = ETileState::Intact;
	Tile->TimeSinceStepped = 0.0f;
	Tile->bOccupied = false;
	Tile->Occupant = nullptr;

	// Remover da lista de timers ativos
	ActiveTimerTiles.Remove(Coord);

	// TODO: Atualizar visual
	UpdateTileVisual(Coord);

	// TODO: Reproduzir efeito de restauração
}

void UTileManager::ForceBreakTile(const FTileCoord& Coord)
{
	FTileInfo* Tile = GetTileInfo(Coord);
	if (!Tile)
	{
		return;
	}

	// Quebra imediata sem esperar timer
	BreakTile(Coord);
}

TArray<FTileCoord> UTileManager::GetNeighbors(const FTileCoord& Coord, bool bIncludeDiagonals)
{
	TArray<FTileCoord> Neighbors;

	// Direções adjacentes (cima, baixo, esquerda, direita)
	const TArray<FTileCoord> AdjacentDirs = {
		FTileCoord(0, 1),   // Norte
		FTileCoord(0, -1),  // Sul
		FTileCoord(-1, 0),  // Oeste
		FTileCoord(1, 0)    // Leste
	};

	// Direções diagonais
	const TArray<FTileCoord> DiagonalDirs = {
		FTileCoord(1, 1),   // Nordeste
		FTileCoord(1, -1),  // Sudeste
		FTileCoord(-1, 1),  // Noroeste
		FTileCoord(-1, -1)  // Sudoeste
	};

	// Adicionar vizinhos adjacentes
	for (const FTileCoord& Dir : AdjacentDirs)
	{
		FTileCoord Neighbor(Coord.X + Dir.X, Coord.Y + Dir.Y);
		if (IsValidCoord(Neighbor))
		{
			Neighbors.Add(Neighbor);
		}
	}

	// Adicionar vizinhos diagonais se solicitado
	if (bIncludeDiagonals)
	{
		for (const FTileCoord& Dir : DiagonalDirs)
		{
			FTileCoord Neighbor(Coord.X + Dir.X, Coord.Y + Dir.Y);
			if (IsValidCoord(Neighbor))
			{
				Neighbors.Add(Neighbor);
			}
		}
	}

	return Neighbors;
}

bool UTileManager::IsReachable(const FTileCoord& From, const FTileCoord& To)
{
	// Verificar se ambos são válidos
	if (!IsValidCoord(From) || !IsValidCoord(To))
	{
		return false;
	}

	// Verificar se To está nos vizinhos de From
	TArray<FTileCoord> Neighbors = GetNeighbors(From, true);
	for (const FTileCoord& Neighbor : Neighbors)
	{
		if (Neighbor == To)
		{
			// Verificar se o tile de destino não está quebrado
			FTileInfo* ToTile = GetTileInfo(To);
			return ToTile && ToTile->State != ETileState::Broken;
		}
	}

	return false;
}

void UTileManager::ProcessTileTimers(float DeltaTime)
{
	// Iterar sobre tiles ativos (que estão sendo processados)
	for (int32 i = ActiveTimerTiles.Num() - 1; i >= 0; --i)
	{
		const FTileCoord& Coord = ActiveTimerTiles[i];
		FTileInfo* Tile = GetTileInfo(Coord);

		if (!Tile)
		{
			ActiveTimerTiles.RemoveAt(i);
			continue;
		}

		// Incrementar tempo
		Tile->TimeSinceStepped += DeltaTime;

		// Processar transição de estado baseado no tempo
		if (Tile->State == ETileState::Intact && Tile->TimeSinceStepped >= TimeToBreakAfterStepped)
		{
			// Transição para Cracked
			Tile->State = ETileState::Cracked;
			UpdateTileVisual(Coord);
			
			UE_LOG(LogTemp, Log, TEXT("TileManager: Tile (%d, %d) rachou"), Coord.X, Coord.Y);
			
			// TODO: Reproduzir som/efeito de rachadura
		}
		else if (Tile->State == ETileState::Cracked && 
		         Tile->TimeSinceStepped >= (TimeToBreakAfterStepped + TimeToRemoveAfterCrack))
		{
			// Transição para Broken (remoção)
			BreakTile(Coord);
		}
	}
}

void UTileManager::UpdateTileVisual(const FTileCoord& Coord)
{
	FTileInfo* Tile = GetTileInfo(Coord);
	if (!Tile)
	{
		return;
	}

	// TODO: Atualizar visual do tile baseado no estado
	// Por exemplo:
	// - Intact: material normal
	// - Cracked: material com rachadura
	// - Broken: esconder instância ou usar material transparente
	
	// if (TileHISM && Tile->InstancedIndex >= 0)
	// {
	//     switch (Tile->State)
	//     {
	//         case ETileState::Broken:
	//             TileHISM->UpdateInstanceTransform(Tile->InstancedIndex, FTransform(FVector(0,0,-10000)), false, true);
	//             break;
	//         case ETileState::Cracked:
	//             // Trocar material ou aplicar decal
	//             break;
	//         default:
	//             break;
	//     }
	// }
}

void UTileManager::SpawnPickupAtLocation(const FVector& Location)
{
	UE_LOG(LogTemp, Log, TEXT("TileManager: Spawning pickup at %s"), *Location.ToString());

	// TODO: Spawnar actor de pickup de bloco de gelo
	// if (PickupClass)
	// {
	//     FActorSpawnParameters SpawnParams;
	//     SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//     
	//     FVector SpawnLocation = Location + FVector(0, 0, 50); // Acima do tile
	//     GetWorld()->SpawnActor<AActor>(PickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	// }
}

int32 UTileManager::CoordToIndex(const FTileCoord& Coord) const
{
	// Validação básica para evitar acesso fora dos limites
	if (!IsValidCoord(Coord))
	{
		UE_LOG(LogTemp, Error, TEXT("TileManager: CoordToIndex chamado com coordenada inválida (%d, %d)"), Coord.X, Coord.Y);
		return 0; // Retorna índice seguro
	}
	
	return Coord.Y * GridSizeX + Coord.X;
}

FTileCoord UTileManager::IndexToCoord(int32 Index) const
{
	int32 X = Index % GridSizeX;
	int32 Y = Index / GridSizeX;
	return FTileCoord(X, Y);
}
