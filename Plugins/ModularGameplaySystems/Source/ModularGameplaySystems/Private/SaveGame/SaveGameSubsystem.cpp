// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "SaveGame/SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSaveGame, Log, TEXT("SaveGameSubsystem initialized."));
}

void USaveGameSubsystem::Deinitialize()
{
	SetAutoSaveEnabled(false);
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------------------

void USaveGameSubsystem::SaveGame(const FString& SlotName, int32 UserIndex)
{
	USaveGameData* DataToSave = GetCurrentSaveData();
	DataToSave->StampCurrentTime();

	UE_LOG(LogSaveGame, Log, TEXT("Saving game to slot '%s' (version=%d)..."),
		   *SlotName, DataToSave->SaveVersion);

	FAsyncSaveGameToSlotDelegate Delegate;
	Delegate.BindUObject(this, &USaveGameSubsystem::HandleAsyncSaveComplete);
	UGameplayStatics::AsyncSaveGameToSlot(DataToSave, SlotName, UserIndex, Delegate);
}

void USaveGameSubsystem::LoadGame(const FString& SlotName, int32 UserIndex)
{
	if (!DoesSaveSlotExist(SlotName, UserIndex))
	{
		UE_LOG(LogSaveGame, Warning, TEXT("LoadGame: slot '%s' does not exist."), *SlotName);
		OnLoadCompleted.Broadcast(SlotName, nullptr);
		return;
	}

	UE_LOG(LogSaveGame, Log, TEXT("Loading game from slot '%s'..."), *SlotName);

	FAsyncLoadGameFromSlotDelegate Delegate;
	Delegate.BindUObject(this, &USaveGameSubsystem::HandleAsyncLoadComplete);
	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, Delegate);
}

bool USaveGameSubsystem::DeleteSaveSlot(const FString& SlotName, int32 UserIndex)
{
	if (!DoesSaveSlotExist(SlotName, UserIndex))
	{
		return false;
	}
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	UE_LOG(LogSaveGame, Log, TEXT("Deleted save slot '%s'."), *SlotName);
	return true;
}

bool USaveGameSubsystem::DoesSaveSlotExist(const FString& SlotName, int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

// ---------------------------------------------------------------------------
// Active save data
// ---------------------------------------------------------------------------

USaveGameData* USaveGameSubsystem::GetCurrentSaveData()
{
	if (!CurrentSaveData)
	{
		CurrentSaveData = CreateNewSaveData();
	}
	return CurrentSaveData;
}

void USaveGameSubsystem::SetCurrentSaveData(USaveGameData* NewData)
{
	CurrentSaveData = NewData;
}

USaveGameData* USaveGameSubsystem::CreateNewSaveData()
{
	return Cast<USaveGameData>(UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass()));
}

// ---------------------------------------------------------------------------
// Auto-save
// ---------------------------------------------------------------------------

void USaveGameSubsystem::SetAutoSaveEnabled(bool bEnabled)
{
	bAutoSaveEnabled = bEnabled;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	if (bEnabled)
	{
		World->GetTimerManager().SetTimer(AutoSaveTimerHandle,
		                                  FTimerDelegate::CreateUObject(this, &USaveGameSubsystem::PerformAutoSave),
		                                  AutoSaveInterval, /*bLoop=*/true);
		UE_LOG(LogSaveGame, Log, TEXT("Auto-save enabled (interval=%.0fs, slot='%s')."),
			   AutoSaveInterval, *AutoSaveSlotName);
	}
	else
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		UE_LOG(LogSaveGame, Log, TEXT("Auto-save disabled."));
	}
}

void USaveGameSubsystem::SetAutoSaveInterval(float Seconds)
{
	AutoSaveInterval = FMath::Max(1.0f, Seconds);
	if (bAutoSaveEnabled)
	{
		// Re-register with the new interval
		SetAutoSaveEnabled(true);
	}
}

void USaveGameSubsystem::PerformAutoSave()
{
	SaveGame(AutoSaveSlotName);
	UE_LOG(LogSaveGame, Log, TEXT("Auto-save triggered (slot='%s')."), *AutoSaveSlotName);
}

// ---------------------------------------------------------------------------
// Private callbacks
// ---------------------------------------------------------------------------

void USaveGameSubsystem::HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex,
                                                  bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogSaveGame, Log, TEXT("Save to slot '%s' succeeded."), *SlotName);
	}
	else
	{
		UE_LOG(LogSaveGame, Error, TEXT("Save to slot '%s' FAILED."), *SlotName);
	}
	OnSaveCompleted.Broadcast(SlotName, bSuccess);
}

void USaveGameSubsystem::HandleAsyncLoadComplete(const FString& SlotName, int32 UserIndex,
                                                  USaveGame* LoadedSave)
{
	USaveGameData* LoadedData = Cast<USaveGameData>(LoadedSave);
	if (LoadedData)
	{
		CurrentSaveData = LoadedData;
		UE_LOG(LogSaveGame, Log, TEXT("Loaded slot '%s' (version=%d)."),
			   *SlotName, LoadedData->SaveVersion);
	}
	else
	{
		UE_LOG(LogSaveGame, Warning, TEXT("Load from slot '%s': no valid USaveGameData found."),
			   *SlotName);
	}
	OnLoadCompleted.Broadcast(SlotName, LoadedData);
}
