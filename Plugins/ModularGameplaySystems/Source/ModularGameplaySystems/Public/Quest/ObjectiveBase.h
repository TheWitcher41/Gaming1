// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveBase.generated.h"

/** Logging category for the Quest system. */
DECLARE_LOG_CATEGORY_EXTERN(LogQuest, Log, All);

/** Tracks the current completion state of a quest objective. */
UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	Inactive   UMETA(DisplayName = "Inactive"),
	Active     UMETA(DisplayName = "Active"),
	Completed  UMETA(DisplayName = "Completed"),
	Failed     UMETA(DisplayName = "Failed"),
};

/**
 * Base class for a single quest objective.
 *
 * Derive in Blueprint or C++ to implement concrete objective logic
 * (e.g. kill X enemies, collect Y items, reach location Z).
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class MODULARGAMEPLAYSYSTEMS_API UObjectiveBase : public UObject
{
	GENERATED_BODY()

public:
	/** Short description shown in the quest tracker UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
	FText Description;

	/** Stable identifier referenced by QuestComponent to update progress. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective")
	FName ObjectiveID;

	/** Target progress value to consider this objective completed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objective",
			  meta = (ClampMin = 1))
	int32 RequiredProgress = 1;

	// -----------------------------------------------------------------------

	/** Current progress toward RequiredProgress. */
	UFUNCTION(BlueprintPure, Category = "Objective")
	int32 GetCurrentProgress() const { return CurrentProgress; }

	/** Returns a [0,1] fraction of completion. */
	UFUNCTION(BlueprintPure, Category = "Objective")
	float GetProgressFraction() const;

	/** Current lifecycle state. */
	UFUNCTION(BlueprintPure, Category = "Objective")
	EObjectiveState GetState() const { return State; }

	/** Returns true when State == Completed. */
	UFUNCTION(BlueprintPure, Category = "Objective")
	bool IsCompleted() const { return State == EObjectiveState::Completed; }

	/**
	 * Add Delta to progress. Auto-completes when progress reaches RequiredProgress.
	 * @return true if the objective just completed as a result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	bool AddProgress(int32 Delta = 1);

	/** Activate this objective (called by QuestComponent when the quest starts). */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Activate();

	/** Mark failed (called by QuestComponent on quest failure/cancellation). */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Fail();

	/** Blueprint hook: called whenever progress changes. */
	UFUNCTION(BlueprintNativeEvent, Category = "Objective")
	void OnProgressUpdated(int32 OldProgress, int32 NewProgress);
	virtual void OnProgressUpdated_Implementation(int32 OldProgress, int32 NewProgress);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	int32 CurrentProgress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	EObjectiveState State = EObjectiveState::Inactive;
};
