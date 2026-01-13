# Frostbite 3D — Documentação de Mecânicas / Gameplay

Versão: 0.1  
Data: 2026-01-13  
Autor: Copilot (para uso do usuário drigues90)  

Objetivo: reunir, organizar e especificar todas as mecânicas de gameplay necessárias para implementar o "Frostbite" recriado em 3D (Unreal Engine 5 + C++). Este documento serve como fonte de verdade consultável durante a implementação.

Sumário
- Visão geral
- Loop principal de jogo
- Sistemas e responsabilidades (lista de classes/Componentes)
- Regras e parâmetros de gameplay (valores configuráveis)
- Tiles (grade de gelo) — estados, comportamentos e eventos
- Jogador — movimentação, input, colisões, animações e interações com tiles
- Igloo (construção) — lógica de coleta e montagem
- Inimigos — tipos, IA, estados e interações
- HUD / Pontuação / Vidas / Timer
- Física, efeitos e feedbacks
- Replicação / Multiplayer (notas)
- Estruturas de dados e assinaturas C++ sugeridas
- Eventos / Mensagens (delegates) e fluxo de estado
- Casos de uso e cenários de aceitação
- Checklist de implementação / milestones
- TODOs e notas legais

---

## 1. Visão geral
Foco: recriar a jogabilidade do Frostbite clássico, mantendo a mecânica central (pular sobre blocos de gelo, coletar blocos, construir um iglu e evitar inimigos) mas em um mundo 3D realista.  
Plataforma alvo inicial: PC (UE5) single-player; arquitetura preparada para replicação se futuro multiplayer desejado.

Metas de gameplay:
- Sensação de salto preciso e "snap" a tiles.
- Tiles com comportamento determinístico (quebram/derretem, removem-se, ou mudam de estado).
- Coleta de blocos como recurso para construir iglu em posições específicas.
- Enemies que ameaçam o jogador e/ou tiles.
- UI clara: vidas, pontuação, blocos coletados, timer de fase.

---

## 2. Loop principal de jogo
1. Start level
2. Spawn player na posição de início
3. Geração / inicialização da grade de tiles (Tile Manager)
4. Jogador move-se e salta entre tiles
5. Ao executar ações (coletar bloco / completar iglu / morrer / tempo esgotar) atualizar estado do jogo
6. Win condition: iglu completo (ou pontuação alvo)
7. Lose condition: vidas acabaram, time esgotado, ou player preso sem tiles
8. Transição para próximo nível ou tela de fim

---

## 3. Sistemas e responsabilidades

Sugestão de classes/Componentes principais (nomes em estilo UE C++):

- AFrostbiteGameMode
  - Regras de jogo, estados de nível (Running, Paused, Won, Lost), transições.
- AFrostbitePlayerCharacter : ACharacter
  - Movimentação, animação, inputs, interações com tiles e itens.
- UTileManager : UObject / ActorComponent
  - Cria e gerencia a grid de tiles; expõe API para consultas/alterações.
- AIceTile : AActor
  - Representa um tile individual (mesh instanciado ou HISM/InstancedStaticMesh).
- UIglooComponent : UActorComponent
  - Gerencia a coleção de blocos do jogador e lógica de montagem.
- AEnemyBase : APawn / ACharacter
  - Base IA, estado (Patrol, Chasing, Attacking), dano ao jogador/tiles.
- UGameplayStats / UScoreManager : UObject
  - Pontuação, vidas, blocos coletados, timers.
- UHudWidget : UUserWidget
  - Exibir vidas, pontuação, blocos, timer, mensagens.
- UTileNavigationComponent (opcional)
  - Ajuda a calcular saltos válidos entre tiles (grafo).
- ANavigationAnchor / ASpawnPoint (opcional)
  - Pontos de spawn para inimigos, pickups, jogador.

Observação: para performance, tiles repetitivos devem usar HISM / Instanced Static Meshes com instâncias para cada tile, mantendo state separadamente em um array/struct.

---

## 4. Regras e parâmetros configuráveis (valores iniciais sugeridos)
Todos esses valores devem ser expostos como UPROPERTY EditAnywhere para fácil ajuste.

- Grid
  - GridSizeX = 8 (colunas)
  - GridSizeY = 6 (linhas)
  - TileSpacing = 120.0f (distância entre centros dos tiles)
  - TileHeightVariance = +/- 8.0f (pequenas variações para realismo)
- Player
  - JumpHeight = 420.0f
  - JumpHorizontalSnapTolerance = 40.0f (tolerância de "snap" ao centro do tile ao aterrissar)
  - MoveSpeed = 420.0f
  - MaxLives = 3
  - RespawnInvulnerability = 1.0s
- Tile
  - DefaultState = Intact
  - BreakableState -> "Cracked" -> "Broken" / Removed
  - TimeToBreakAfterStepped = 1.2s (tempo para começar a rachar)
  - TimeToRemoveAfterCrack = 0.8s
  - CollectibleDropChance = 0.12 (12% de chance de soltar bloco quando destruído)
- Igloo
  - RequiredBlocks = e.g. 10 (ou configurável por nível)
- Enemies
  - PatrolSpeed = 150.0f
  - ChaseSpeed = 350.0f
  - DamageToPlayer = 1 Life
  - TileDamageOnPass = boolean/config (alguns inimigos quebram tiles ao passar)
- Timer
  - LevelTime = 120s (por nível inicial)

Esses são valores iniciais — ajustar por playtests.

---

## 5. Tiles (grade de gelo)

Conceito: Grid 2D de tiles posicionados no mundo 3D em coordenadas X,Y; cada tile possui estado e propriedades.

Estados do tile (enum ETileState):
- Intact
- Cracked
- Broken (Removed/Disabled)
- Frozen (temporariamente imune)
- Moving (se houver tiles dinâmicos)
- Occupied (por jogador/enemy/item)

Propriedades:
- GridCoord (int x, int y)
- WorldLocation (FVector)
- State (ETileState)
- Occupant (AActor* — jogador ou inimigo)
- TimeSinceStepped (float)
- InstancedIndex (int32) se usa HISM
- Mesh/Material: material muda conforme state (ex.: crack decal)

Eventos importantes:
- OnSteppedOn(AActor* who)
  - Ação: iniciar timer para rachar (se breakable)
  - Se tile já Cracked e stepped again -> pode acelerar quebra
- OnLeave(AActor* who)
  - Limpa occupant
- OnBreak()
  - Play particle (Niagara), som, despawn ou desativar colisão
  - Possível spawn de collectible
- OnRestore()
  - Usado para respawn de tile após X segundos (opcional)
- OnForceBreak() / ExternalDamage()

Comportamento temporal:
- Ao receber OnSteppedOn:
  - Se State == Intact e Breakable -> set State = Cracked after TimeToCrack
  - Depois TimeToRemoveAfterCrack -> set State = Broken
- Tile Manager controlaTimers globalmente para eficiência (evitar Tick por tile)

Implementação eficiente:
- Use uma única Actor (TileManager) com HISM para render; store tile states em array/struct.
- Colisão: use simples box collision por tile (thin) ou um único collision mesh dividido; outra opção: spawn cheap collision actors só para tiles ocupados (tradeoff).

Interação física:
- Se tile quebra com jogador em cima -> aplicar knockback e queda (usar LaunchCharacter / ragdoll configurações leves).

---

## 6. Jogador

Principais responsabilidades:
- Movimentação (andar, correr opcional, pular)
- Saltos precisos entre tiles (snap)
- Coletar blocos que aparecem (pickup)
- Interagir com tiles (OnSteppedOn)
- Pegar dano, perder vidas e respawn
- Animações: Idle, Walk, Run, JumpStart, MidAir, Land, Fall, Hurt

Detalhes de implementação:
- Herda de ACharacter para aproveitar CharacterMovementComponent.
- Override Jump mechanics para suportar "grid jump":
  - Quando o jogador pressiona Jump e um tile alvo válido está na direção do input, realizar movimento que garante aterrissagem central no tile (ler TileManager para validação).
  - Durante o salto, desativar "root motion" lateral do CharacterMovement; use LaunchCharacter com vetor calculado para posição do tile; ou aplicar timeline que move o Character para o destino com paralela animação.
- Snap ao tile:
  - Ao detectar aterrissagem próxima do tile center (dist < JumpHorizontalSnapTolerance), set actor location X,Y ao center para evitar deslize visual.
- Ground detection:
  - Quando colidir com um tile, informar TileManager / Tile actor via OnSteppedOn.
- Pickup handling:
  - OnOverlap with collectible actor -> increment UIglooComponent.BlocksCollected, play SFX, destroy pickup actor.
- Health/Lives:
  - UPROPERTY Lives — quando 0 => trigger AFrostbiteGameMode::OnPlayerDeath.

Edge cases:
- Queda entre tiles -> perder 1 vida e respawn no checkpoint.
- If tile breaks under player midair -> check next landing: if no tile => fall.

Eventos públicos (UFUNCTION / delegates):
- OnBlocksCollectedChanged(int NewCount)
- OnLivesChanged(int NewLives)
- OnInvulnerabilityStarted(float Duration)
- OnPlayerDeath()

Replicação:
- Replicate character movement; authority server should validate tile stepping and igloo block counts in multiplayer.

---

## 7. Igloo (Construção)

Conceito:
- Jogador coleciona "ice blocks" (pickups) que são recursos. Ao alcançar posições específicas (igloo build site) ou pressionar ação perto do igloo, consome blocos para preencher seções.
- Quando RequiredBlocks alcançados -> igloo complete -> level victory.

Detalhes:
- UIglooComponent:
  - Properties:
    - BlocksCollected (int)
    - RequiredBlocks (int)
    - BuildSites: array de locations (ou child actors) onde blocos serão visualmente posicionados.
  - API:
    - AddBlock() -> incrementa, event OnBlocksChanged()
    - TryPlaceBlockAtNearestSite(FVector PlayerLocation) -> if has blocks, place block, decrement blocks collected, mark site filled
    - IsComplete() -> BlocksPlaced >= RequiredBlocks
  - Visual:
    - Ao colocar cada bloco, spawn pequeno mesh/instância no BuildSite com animação (slide/attach).
- Pickups:
  - Small actors that float above broken tile for a moment; OnOverlap com player -> call AddBlock.

Design de níveis:
- Igloo pode estar num canto ou centro do mapa. BuildSites podem ter ordem (sequencial ou livre).

---

## 8. Inimigos

Tipos iniciais (sugestões):
- Bird (pássaro) — voa sobre tiles, perssegue jogador em linha reta por certo tempo.
- PolarBear (urso) — caminha em tiles, pode quebrar tiles ao passar.
- Seal / Predator secundário — telegraphed dash attacks.

IA básica:
- States: Idle/Patrol -> Investigate -> Chase -> Attack -> Retreat
- Perception:
  - Uso simples de SphereTrace ou PawnSensing para detectar jogador.
- Pathfinding:
  - Preferir usar grade de tiles (TileManager) para determinar próximos destinos — facilita decidir se inimigo pode andar por tiles.
- Interações com tiles:
  - Alguns inimigos não quebram tiles; outros dão "damage" ao tile ao passar.
- Damage:
  - Ao tocar jogador -> deal DamageToPlayer (1 life) and bounce player (apply small knockback).
  - Collisions com broken tile -> fall.

Implementação:
- AEnemyBase com AIController. Comportamentos simples podem ser realizados em C++ (state machine) sem BehaviorTree inicialmente.

---

## 9. HUD / Pontuação / Vidas / Timer

Elementos:
- Vidas (icons)
- Pontuação (score)
- Blocos coletados / Required
- Timer (mm:ss)
- Mensagens de estado (Level Start, Level Complete, Game Over)
- Painel de feedback (hit flash, low time warning)

Data flow:
- UScoreManager notifica UHudWidget via delegates ou pelo PlayerController.
- AFrostbiteGameMode expõe LevelTime e atualiza timer (replicated if multiplayer).

Regras de pontuação (sugestão inicial):
- Coletar bloco: +100
- Construir bloco no igloo: +250
- Passar nível: +1000 + tempo restante * 10
- Enemy defeated (se houver mecânica de derrotar): +200

---

## 10. Física, efeitos e feedbacks

- Sistema de partículas: Niagara para gelo rachando, respingos de neve, "fall dust".
- Sons: MetaSounds para SFX (jump, land, break, collect); música ambiental dinâmica.
- Camera:
  - Terceira pessoa com follow suave. Ao saltar, câmera levemente amplia e ajusta pitch.
  - Occlusion handling (colisão com parede) e lerp to avoid clipping.
- Haptics (gamepad) — opcional.
- VFX materiais:
  - Tile material muda roughness/normal ao rachar.
- Feedback de tela:
  - Quando urgente (low time) -> HUD e áudio, tile pulsate.

---

## 11. Replicação / Multiplayer (notas rápidas)
- Planejar desde início:
  - TileManager deve ser server-authoritative (server decide state changes).
  - Player movement replicated using CharacterMovementComponent.
  - Events críticos (Tile break, pickup spawn, igloo build) replicate Multicast from server.
- Avoid replicating per-tile tick; server runs timers and replicates state changes only when state changes.
- Use RPCs:
  - Client -> Server: RequestJumpToTile(GridCoord) (server validates reachable)
  - Server -> Clients: MulticastTileStateChanged(GridCoord, NewState)
- Synchronize score and lives via replicated variables.

---

## 12. Estruturas de dados e assinaturas C++ sugeridas

Exemplos de structs e enums:

```cpp
// Enums
UENUM(BlueprintType)
enum class ETileState : uint8
{
    Intact,
    Cracked,
    Broken,
    Frozen,
    Occupied
};

// Tile coordinate
USTRUCT(BlueprintType)
struct FTileCoord
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) int32 X;
    UPROPERTY(EditAnywhere) int32 Y;
};

// Tile info (lightweight)
USTRUCT()
struct FTileInfo
{
    GENERATED_BODY()
    FTileCoord Coord;
    FVector WorldPosition;
    ETileState State;
    float TimeSinceStepped;
    bool bOccupied;
    int32 InstancedIndex; // -1 if not using instancing
};
```

APIs/Assinaturas (propostas):

TileManager:
```cpp
UCLASS()
class UTileManager : public UActorComponent
{
    GENERATED_BODY()
public:
    // Inicializa / gera grid
    void InitializeGrid(int32 SizeX, int32 SizeY, float Spacing, FVector Origin);

    // Query
    bool IsValidCoord(const FTileCoord& Coord) const;
    FTileInfo* GetTileInfo(const FTileCoord& Coord);
    FTileInfo* GetTileAtWorldLocation(const FVector& Location);

    // Manipulação
    void OnActorStepped(AActor* Actor, const FTileCoord& Coord);
    void BreakTile(const FTileCoord& Coord);
    void RestoreTile(const FTileCoord& Coord);

    // Path helpers
    TArray<FTileCoord> GetNeighbors(const FTileCoord& Coord);
    bool IsReachable(const FTileCoord& From, const FTileCoord& To);
};
```

PlayerCharacter:
```cpp
UCLASS()
class AFrostbiteCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated) int32 Lives;
    UPROPERTY(Replicated) int32 BlocksCollected;

    UFUNCTION(Server, Reliable)
    void Server_RequestJumpToTile(const FTileCoord& Target);

    void AttemptJumpToTile(const FTileCoord& Target);
    void OnLandedOnTile(const FTileCoord& TileCoord);
};
```

IglooComponent:
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UIglooComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    int32 BlocksCollected;
    int32 BlocksPlaced;
    int32 RequiredBlocks;

    bool AddBlock();
    bool TryPlaceBlockAtNearestSite(const FVector& PlayerLocation);
    bool IsComplete() const;
};
```

Events/delegates (exemplos):
- DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlocksChanged, int32, NewCount);
- FOnTileStateChanged(GridCoord, ETileState)

---

## 13. Eventos / Mensagens e fluxo de estado

Eventos importantes:
- Game:
  - OnLevelStart()
  - OnLevelComplete()
  - OnLevelFailed()
- Tile:
  - OnTileStepped(GridCoord, AActor*)
  - OnTileStateChanged(GridCoord, ETileState)
  - OnTileBroken(GridCoord)
- Player:
  - OnPlayerLandedTile(GridCoord)
  - OnPlayerFell()
  - OnPlayerDamaged(int newLives)
  - OnBlocksCollectedChanged(int newCount)
- Igloo:
  - OnBlockPlaced(int placed, int required)
  - OnIglooComplete()

Fluxo de exemplo (colisão tile):
1. Player lands -> PlayerCharacter::OnLanded -> call TileManager->OnActorStepped(this, landedCoord)
2. TileManager updates FTileInfo.TimeSinceStepped and sets tile to Cracked after threshold -> broadcasts OnTileStateChanged
3. If tile reaches Broken and Player still on tile -> GameMode handles fall/damage or tile triggers apply to occupant (launch).

---

## 14. Casos de uso / Cenários de aceitação

1. Saltar entre tiles
   - Dado um jogador e dois tiles adjacentes
   - Quando o jogador pulsa jump em direção ao tile alvo válido
   - Então o jogador deve ser lançado e aterrissar centralizado no tile alvo sem atravessar ou deslizar.

2. Tile racha e quebra
   - Dado um tile breakable
   - Quando o jogador pisa e permanece por TimeToBreakAfterStepped
   - Então o tile passa para Cracked, play VFX/SFX; depois de TimeToRemoveAfterCrack passa a Broken e se desativa.

3. Coleta de bloco
   - Dado um pickup spawnado
   - Quando o jogador fizer overlap com o pickup
   - Então BlocksCollected++ e o pickup é destruído; HUD atualiza.

4. Construção de iglu
   - Dado RequiredBlocks = 5 e jogador com 5 blocks
   - Quando o jogador usa TryPlaceBlock nas 5 BuildSites
   - Então igloo completa e OnIglooComplete() é disparado -> GameMode marca level complete.

5. Inimigo causa dano
   - Quando inimigo colide com jogador em estado attack
   - Então jogador perde 1 vida e entra em invulnerabilidade temporária.

---

## 15. Checklist de implementação / milestones (tarefa prática)

Fase 1 — Protótipo baseline
- [ ] Criar projeto UE5 C++ e configurar input
- [ ] Implementar AFrostbitePlayerCharacter com jump/snapping básico
- [ ] Implementar UTileManager + geração simples de grid e tiles visuais (instanced)
- [ ] Conectar OnSteppedOn pipeline

Fase 2 — Gameplay core
- [ ] Tile cracks e quebra com timers centralizados
- [ ] Pickups de bloco e UIglooComponent com contagem
- [ ] Implementar HUD básico com vidas/blocos/timer
- [ ] Implementar respawn e penalidade por queda

Fase 3 — Enemies & Polishing
- [ ] Basic Enemy (Patrol + Chase)
- [ ] Tile-damage interaction (some enemies break tiles)
- [ ] Visuals (materials, particles), câmera polida e áudio

Fase 4 — Optimization & polish
- [ ] Trocar meshes por Nanite/Instancing
- [ ] Otimizações de tick (TileManager timer batching)
- [ ] Playtests, ajustar valores de gameplay

---

## 16. TODOs / Observações finais

- Refinar valores de gameplay via playtests (jump arc, snap tolerances, timers de tile).
- Implementar ferramenta de debug visual para mostrar grid/coords e states (útil para QA).
- Definir assets: modelos de tile, partículas de crack, sounds SFX/Music.
- Preparar sistema de save/checkpoints se levels forem longos.
- Revisar IP/trademarks caso haja intenção de publicação comercial — Frostbite é propriedade da Atari (ou detentor atual).

---

Apêndice: Exemplo de pseudocódigo (TileManager loop que processa timers):

```cpp
// Pseudocode simplificado (executado no TileManager Tick ou via Timer)
for each tile in ActiveTiles:
    if tile.TimeSinceStepped > 0:
        tile.TimeSinceStepped += DeltaTime
        if tile.State == Intact and tile.TimeSinceStepped >= TimeToCrack:
            tile.State = Cracked
            BroadcastTileStateChanged(tile.Coord, Cracked)
        else if tile.State == Cracked and tile.TimeSinceStepped >= TimeToCrack + TimeToRemoveAfterCrack:
            tile.State = Broken
            BroadcastTileBroken(tile.Coord)
            PossiblySpawnPickup(tile.WorldPosition)
```

---

Seções seguintes que posso gerar automaticamente (me diga qual prefere a seguir):
- GDD compacto em formato Markdown com telas, UX e regras completas.
- Scaffold C++ (arquivos header/cpp) para TileManager, AFrostbiteCharacter e structs mostrados aqui.
- Blueprint/BlueprintNativeImplementable protótipos para testes rápidos.

Se quiser, eu salvo este arquivo diretamente em um repositório (criar PR) — se for este o caso, me informe o repositório (owner/repo) e peça explicitamente para "abrir um pull request" contendo este arquivo.
