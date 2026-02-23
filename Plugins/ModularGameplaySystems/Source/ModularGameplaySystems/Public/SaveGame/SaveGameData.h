// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameData.generated.h"

/** Logging category for the Save system. */
DECLARE_LOG_CATEGORY_EXTERN(LogSaveGame, Log, All);

/**
 * Versioned container for all persistent game data.
 *
 * Extend this class or create a Blueprint child to add your own save fields.
 * The SaveVersion field lets you migrate old saves when the data layout changes.
 */
UCLASS(BlueprintType, Blueprintable)
class MODULARGAMEPLAYSYSTEMS_API USaveGameData : public USaveGame
{
	GENERATED_BODY()

public:
	USaveGameData();

	/**
	 * Incremented whenever the save data format changes.
	 * Read this at load time to perform migration if needed.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 1;

	/** Wall-clock timestamp when this save was last written. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FDateTime SaveTimestamp;

	/** Friendly label for the save slot shown in UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SlotDisplayName;

	/** Total in-game play time in seconds at the point of saving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float TotalPlayTime = 0.0f;

	/** Name of the map / level that was active when the game was saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName CurrentLevelName;

	/**
	 * Generic key-value store for lightweight boolean flags
	 * (quest completed, tutorial shown, etc.).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, bool> BoolFlags;

	/**
	 * Generic key-value store for lightweight integer values
	 * (counters, currency, levels, etc.).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FName, int32> IntValues;

	/** Update SaveTimestamp to the current UTC time. Called before writing to disk. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void StampCurrentTime();
};
