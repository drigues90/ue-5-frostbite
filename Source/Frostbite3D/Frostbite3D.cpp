// Copyright (c) 2026 Frostbite 3D Team. Todos os direitos reservados.

#include "Frostbite3D.h"
#include "Modules/ModuleManager.h"

void FFrostbite3DModule::StartupModule()
{
	// Este código será executado após o carregamento do módulo
	// Utilize para inicialização de sistemas que precisam ser configurados antes do jogo iniciar
}

void FFrostbite3DModule::ShutdownModule()
{
	// Este código será executado antes do módulo ser descarregado
	// Utilize para limpeza de recursos e sistemas criados no StartupModule
}

IMPLEMENT_PRIMARY_GAME_MODULE(FFrostbite3DModule, Frostbite3D, "Frostbite3D");
