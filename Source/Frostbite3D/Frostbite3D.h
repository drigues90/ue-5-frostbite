// Copyright (c) 2025 Frostbite 3D Team. Todos os direitos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Módulo principal do jogo Frostbite 3D
 * 
 * Este módulo contém a lógica central do jogo, incluindo:
 * - GameMode e regras de jogo
 * - Character do jogador com mecânicas de movimento em grid
 * - Sistema de gerenciamento de tiles (TileManager)
 * - Sistema de construção de iglu (IglooComponent)
 * 
 * O jogo é baseado no clássico Frostbite, reimaginado em 3D para Unreal Engine 5.6
 */
class FFrostbite3DModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
