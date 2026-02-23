// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGame/SaveGameData.h"
#include "SaveGameSubsystem.generated.h"

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveCompleted, FString, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadCompleted, FString, SlotName,
                                              USaveGameData*, LoadedData);

// ---------------------------------------------------------------------------

/**
 * Game-instance subsystem that manages save slots.
 *
 * Automatically available via GetGameInstance()->GetSubsystem<USaveGameSubsystem>().
 * Supports async save/load, slot enumeration, and auto-save.
 *
 * @note Auto-save relies on a timer. Call SetAutoSaveEnabled(true) after BeginPlay.
 */
UCLASS()
class MODULARGAMEPLAYSYSTEMS_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// -----------------------------------------------------------------------
	// Events
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnLoadCompleted OnLoadCompleted;

	// -----------------------------------------------------------------------
	// Save / Load
	// -----------------------------------------------------------------------

	/**
	 * Asynchronously save the active save data to the named slot.
	 * Fires OnSaveCompleted when done.
	 *
	 * @param SlotName  Target slot name (e.g. "Slot_0").
	 * @param UserIndex Platform user index (0 for single-player).
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Asynchronously load save data from the named slot.
	 * Fires OnLoadCompleted when done. Returns nullptr if the slot does not exist.
	 *
	 * @param SlotName  Source slot name.
	 * @param UserIndex Platform user index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGame(const FString& SlotName, int32 UserIndex = 0);

	/**
	 * Delete a save slot.
	 * @return true if the slot existed and was deleted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSaveSlot(const FString& SlotName, int32 UserIndex = 0);

	/** Returns true if a save file exists for the given slot. */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveSlotExist(const FString& SlotName, int32 UserIndex = 0) const;

	// -----------------------------------------------------------------------
	// Active save data
	// -----------------------------------------------------------------------

	/**
	 * Returns the currently active USaveGameData (in-memory, not yet written to disk).
	 * Creates a fresh one if none exists.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	USaveGameData* GetCurrentSaveData();

	/** Replace the active save data with a new instance. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetCurrentSaveData(USaveGameData* NewData);

	/** Create a new blank save data object (does NOT replace the current active data). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	USaveGameData* CreateNewSaveData();

	// -----------------------------------------------------------------------
	// Auto-save
	// -----------------------------------------------------------------------

	/**
	 * Enable or disable periodic auto-save.
	 * Uses the current auto-save slot name and interval.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetAutoSaveEnabled(bool bEnabled);

	/** Set the auto-save interval in seconds. Default is 300 (5 min). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetAutoSaveInterval(float Seconds);

	/** Current auto-save slot name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString AutoSaveSlotName = TEXT("AutoSave");

	// -----------------------------------------------------------------------
	// UGameInstanceSubsystem interface
	// -----------------------------------------------------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** Callback bound to UGameplayStatics::AsyncSaveGameToSlot. */
	void HandleAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);

	/** Callback bound to UGameplayStatics::AsyncLoadGameFromSlot. */
	void HandleAsyncLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedSave);

	/** Perform one auto-save tick. */
	void PerformAutoSave();

	UPROPERTY()
	TObjectPtr<USaveGameData> CurrentSaveData = nullptr;

	FTimerHandle AutoSaveTimerHandle;
	float AutoSaveInterval = 300.0f;
	bool bAutoSaveEnabled = false;
};
