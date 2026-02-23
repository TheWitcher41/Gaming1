// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "ModularGameplaySystems.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FModularGameplaySystemsModule, ModularGameplaySystems)

void FModularGameplaySystemsModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("ModularGameplaySystems: Module started."));
}

void FModularGameplaySystemsModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("ModularGameplaySystems: Module shutdown."));
}
