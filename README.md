# Frostbite 3D - Unreal Engine 5.6 Project

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## 📖 Descrição

Frostbite 3D é uma reimaginação em 3D do clássico jogo Frostbite, desenvolvido em C++ para Unreal Engine 5.6. O projeto segue padrões profissionais e as melhores práticas da Unreal Engine para criar um jogo single-player com mecânicas de movimento baseado em grade de tiles, coleta de blocos de gelo e construção de iglu.

### Características Principais

- **Sistema de Grid de Tiles**: Grade dinâmica com estados (Intacto, Rachado, Quebrado)
- **Personagem com Movimento Preciso**: Saltos com snap automático ao centro dos tiles
- **Sistema de Coleta**: Blocos de gelo como recurso para construção
- **Construção de Iglu**: Mecânica de vitória baseada em completar estrutura
- **GameMode Completo**: Gerenciamento de vidas, pontuação, timer e estados de jogo
- **Preparado para Multiplayer**: Replicação implementada onde necessário

## 🎮 Mecânicas de Gameplay

Baseado no documento `Docs/Gameplay-Mechanics.md`, o jogo implementa:

1. **Movimento em Grid**: Jogador se move entre tiles com snap preciso
2. **Tiles Dinâmicos**: Quebram após tempo quando pisados
3. **Coleta de Blocos**: Pickups aparecem quando tiles são destruídos
4. **Construção**: Jogador usa blocos coletados para construir iglu
5. **Condições de Vitória/Derrota**: 
   - Vitória: Completar o iglu
   - Derrota: Vidas zeradas ou tempo esgotado

## 🏗️ Estrutura do Projeto

```
Frostbite3D/
├── Frostbite3D.uproject          # Arquivo de projeto UE 5.6
├── Source/
│   └── Frostbite3D/
│       ├── Frostbite3D.h         # Header do módulo principal
│       ├── Frostbite3D.cpp       # Implementação do módulo
│       ├── Frostbite3D.Build.cs  # Build configuration
│       ├── GameMode/
│       │   ├── FrostbiteGameMode.h     # GameMode principal
│       │   └── FrostbiteGameMode.cpp
│       ├── Characters/
│       │   ├── FrostbiteCharacter.h    # Character do jogador
│       │   └── FrostbiteCharacter.cpp
│       └── Components/
│           ├── TileManager.h           # Gerenciador de tiles
│           ├── TileManager.cpp
│           ├── IglooComponent.h        # Sistema de iglu
│           └── IglooComponent.cpp
├── Docs/
│   └── Gameplay-Mechanics.md     # Documentação completa das mecânicas
└── README.md                      # Este arquivo
```

## 📋 Pré-requisitos

### Software Necessário

1. **Unreal Engine 5.6**
   - Download: [Epic Games Launcher](https://www.unrealengine.com/download)
   - Instalação via Epic Games Launcher

2. **Visual Studio 2022** (Windows)
   - Download: [Visual Studio](https://visualstudio.microsoft.com/downloads/)
   - Workloads necessários:
     - "Game development with C++"
     - ".NET desktop development"
     - "Desktop development with C++"

3. **Git** (para controle de versão)
   - Download: [Git](https://git-scm.com/downloads)

### Requisitos de Sistema (Recomendado)

- **OS**: Windows 10/11 64-bit
- **Processor**: Quad-core Intel or AMD, 2.5 GHz or faster
- **Memory**: 16 GB RAM
- **Graphics**: DirectX 11 or DirectX 12 compatible graphics card
- **Storage**: 10 GB available space

## 🚀 Como Começar

### 1. Clonar o Repositório

```bash
git clone https://github.com/drigues90/ue-5-frostbite.git
cd ue-5-frostbite
```

### 2. Gerar Arquivos de Projeto

**Opção A: Via Clique Direito (Windows)**
1. Localize o arquivo `Frostbite3D.uproject`
2. Clique com botão direito
3. Selecione "Generate Visual Studio project files"

**Opção B: Via Linha de Comando**
```bash
# Navegue até a pasta de instalação do Unreal Engine
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="[CAMINHO_COMPLETO]\Frostbite3D.uproject" -game -rocket -progress
```

### 3. Abrir o Projeto

**Método 1: Via Editor**
1. Abra o Epic Games Launcher
2. Vá para "Unreal Engine" → "Library"
3. Clique em "Add" e selecione `Frostbite3D.uproject`
4. Clique no projeto para abrir

**Método 2: Via Visual Studio**
1. Abra `Frostbite3D.sln` gerado
2. Defina "Frostbite3D" como projeto inicial
3. Selecione configuração "Development Editor"
4. Pressione F5 para compilar e iniciar

### 4. Compilar o Projeto

**No Visual Studio:**
```
Build → Build Solution (Ctrl+Shift+B)
```

**Na Unreal Engine:**
- O editor compilará automaticamente ao abrir
- Para recompilar manualmente: Tools → Compile

## 🔧 Configuração Inicial

### Configurar Input Mapping

Após abrir o projeto no editor:

1. **Criar Input Mapping Context**:
   - Content Browser → Right Click → Input → Input Mapping Context
   - Nome: `IMC_Default`

2. **Criar Input Actions**:
   - `IA_Move` (Value Type: Axis2D) - Movimento do personagem
   - `IA_Look` (Value Type: Axis2D) - Rotação da câmera
   - `IA_Jump` (Value Type: Digital) - Pulo
   - `IA_Interact` (Value Type: Digital) - Interação com iglu

3. **Configurar Character Blueprint**:
   - Crie um Blueprint baseado em `AFrostbiteCharacter`
   - Atribua as Input Actions criadas às propriedades correspondentes
   - Configure Mesh e Animações

### Configurar GameMode

1. **Project Settings → Maps & Modes**:
   - Default GameMode: `AFrostbiteGameMode`
   - Default Pawn Class: [Seu Character Blueprint]

2. **Configurar Valores de Gameplay**:
   - Edite os valores no GameMode para ajustar dificuldade:
     - `LevelTime`: Tempo de cada nível
     - `RequiredBlocksForVictory`: Blocos necessários

## 📚 Classes Principais

### FrostbiteGameMode
**Localização**: `Source/Frostbite3D/GameMode/FrostbiteGameMode.h`

Responsável por:
- Gerenciar estados de jogo (Running, Paused, Won, Lost)
- Controlar timer de nível
- Processar condições de vitória e derrota
- Gerenciar pontuação

**Propriedades Principais**:
- `LevelTime`: Duração do nível (padrão: 120s)
- `RequiredBlocksForVictory`: Blocos necessários (padrão: 10)
- `PlayerScore`: Pontuação atual

### FrostbiteCharacter
**Localização**: `Source/Frostbite3D/Characters/FrostbiteCharacter.h`

Responsável por:
- Movimento do jogador com snap a tiles
- Sistema de vidas e dano
- Coleta de blocos
- Integração com Enhanced Input System

**Propriedades Principais**:
- `Lives`: Vidas do jogador (padrão: 3)
- `JumpHorizontalSnapTolerance`: Tolerância de snap (padrão: 40.0f)
- `IglooComponent`: Referência ao componente de iglu

### TileManager
**Localização**: `Source/Frostbite3D/Components/TileManager.h`

Responsável por:
- Gerenciar grade de tiles (criação, estados, quebra)
- Processar timers de quebra eficientemente
- Fornecer API para consultas de tiles
- Spawnar pickups quando tiles quebram

**Propriedades Principais**:
- `GridSizeX` / `GridSizeY`: Tamanho da grade (padrão: 8x6)
- `TileSpacing`: Espaçamento entre tiles (padrão: 120.0f)
- `TimeToBreakAfterStepped`: Tempo até rachar (padrão: 1.2s)
- `CollectibleDropChance`: Chance de drop (padrão: 12%)

### IglooComponent
**Localização**: `Source/Frostbite3D/Components/IglooComponent.h`

Responsável por:
- Rastrear blocos coletados
- Gerenciar BuildSites do iglu
- Processar colocação de blocos
- Detectar conclusão do iglu

**Propriedades Principais**:
- `RequiredBlocks`: Blocos necessários (padrão: 10)
- `BlocksCollected`: Blocos coletados pelo jogador
- `BlocksPlaced`: Blocos já colocados no iglu
- `BuildSites`: Locais de construção

## 🎨 Próximos Passos de Desenvolvimento

1. **Assets Visuais**:
   - [ ] Criar ou importar mesh de tile de gelo
   - [ ] Criar materiais (ice intact, cracked, broken)
   - [ ] Criar mesh de bloco de iglu
   - [ ] Criar mesh de personagem ou usar Mannequin

2. **Animações**:
   - [ ] Idle, Walk, Run
   - [ ] Jump Start, Mid Air, Land
   - [ ] Hurt, Death

3. **VFX e Audio**:
   - [ ] Partículas Niagara (crack, break, collect)
   - [ ] Sons SFX (jump, land, break, collect)
   - [ ] Música de fundo

4. **UI/HUD**:
   - [ ] Widget de HUD (vidas, pontuação, blocos, timer)
   - [ ] Telas de vitória/derrota
   - [ ] Menu principal

5. **Inimigos e IA**:
   - [ ] Implementar AEnemyBase
   - [ ] IA de patrulha e perseguição
   - [ ] Sistema de dano a tiles

6. **Polish**:
   - [ ] Ajustar valores via playtesting
   - [ ] Otimizações (HISM para tiles)
   - [ ] Camera polish

## 🐛 Troubleshooting

### Problema: "Could not find definition for module 'Frostbite3D'"
**Solução**: Regenere os arquivos de projeto (clique direito no .uproject)

### Problema: Erros de compilação sobre Enhanced Input
**Solução**: Verifique que `EnhancedInput` está em `PublicDependencyModuleNames` no .Build.cs

### Problema: Character não responde a input
**Solução**: 
1. Verifique que Input Mapping Context foi criado
2. Verifique que Input Actions foram atribuídas no Blueprint do Character
3. Verifique que o Character é possuído por um PlayerController

### Problema: Editor não abre após compilação
**Solução**:
1. Delete as pastas `Binaries`, `Intermediate`, `Saved`
2. Regenere arquivos de projeto
3. Recompile no Visual Studio

## 📝 Convenções de Código

- **Prefixos de Classes**:
  - `A`: Actor classes (AFrostbiteGameMode, AFrostbiteCharacter)
  - `U`: UObject classes (UTileManager, UIglooComponent)
  - `F`: Structs (FTileCoord, FTileInfo)
  - `E`: Enums (ETileState, EFrostbiteGameState)

- **Comentários**: Todos em português conforme requisito
- **Macros Unreal**: UCLASS, UPROPERTY, UFUNCTION usadas apropriadamente
- **Replicação**: Preparada onde necessário (Lives, BlocksCollected, etc)

## 🤝 Contribuindo

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Add MinhaFeature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está sob a licença especificada no arquivo [LICENSE](LICENSE).

## 📧 Contato

- **Desenvolvedor**: drigues90
- **Repositório**: https://github.com/drigues90/ue-5-frostbite

## 🙏 Agradecimentos

- Baseado no clássico Frostbite da Atari
- Documentação de mecânicas em `Docs/Gameplay-Mechanics.md`
- Comunidade Unreal Engine

---

**Nota**: Este é um projeto base/skeleton. Assets visuais, animações e conteúdo adicional precisam ser criados ou importados separadamente conforme os próximos passos de desenvolvimento.
