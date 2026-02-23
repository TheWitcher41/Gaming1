// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestDefinition.h"
#include "QuestComponent.generated.h"

class UQuestDefinition;
class UObjectiveBase;

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted,    UQuestDefinition*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted,  UQuestDefinition*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed,     UQuestDefinition*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgress, UQuestDefinition*, Quest,
                                              UObjectiveBase*, Objective);

// ---------------------------------------------------------------------------

/** Tracks a quest that is currently in-flight. */
USTRUCT(BlueprintType)
struct MODULARGAMEPLAYSYSTEMS_API FActiveQuestEntry
{
	GENERATED_BODY()

	/** The definition being tracked. */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UQuestDefinition> Definition = nullptr;

	/** Runtime objective instances (created from Definition->Objectives). */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UObjectiveBase>> ActiveObjectives;
};

// ---------------------------------------------------------------------------

/**
 * Actor component that tracks active, completed, and failed quests.
 *
 * Attach to a player controller or character to give them a quest log.
 */
UCLASS(ClassGroup = "ModularGameplaySystems", BlueprintType, Blueprintable,
	   meta = (BlueprintSpawnableComponent))
class MODULARGAMEPLAYSYSTEMS_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

	// -----------------------------------------------------------------------
	// Events
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnObjectiveProgress OnObjectiveProgress;

	// -----------------------------------------------------------------------
	// Mutations
	// -----------------------------------------------------------------------

	/**
	 * Begin tracking QuestDef. Does nothing if the quest is already active or
	 * has already been completed.
	 * @return true if the quest was started successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuest(UQuestDefinition* QuestDef);

	/**
	 * Manually mark a quest as completed.
	 * Fires OnQuestCompleted and moves it to CompletedQuestIDs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(UQuestDefinition* QuestDef);

	/**
	 * Fail/abandon an active quest.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest(UQuestDefinition* QuestDef);

	/**
	 * Advance the progress of an objective identified by ObjectiveID inside the
	 * given quest by Delta units.
	 *
	 * @param QuestDef    Parent quest definition.
	 * @param ObjectiveID Matches UObjectiveBase::ObjectiveID.
	 * @param Delta       Amount to add (default 1).
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateObjectiveProgress(UQuestDefinition* QuestDef, FName ObjectiveID,
	                              int32 Delta = 1);

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/** Returns true if QuestDef is currently in-progress. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestActive(UQuestDefinition* QuestDef) const;

	/** Returns true if QuestDef has been completed. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted(UQuestDefinition* QuestDef) const;

	/**
	 * Returns a [0,1] progress fraction for the overall quest
	 * (average of all objective progress fractions).
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	float GetQuestProgress(UQuestDefinition* QuestDef) const;

	/** Returns all currently active quest entries. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	const TArray<FActiveQuestEntry>& GetActiveQuests() const { return ActiveQuests; }

	// -----------------------------------------------------------------------
	// Blueprint extensibility
	// -----------------------------------------------------------------------

	/**
	 * Called when a quest objective is completed.
	 * Override in Blueprint to drive UI, play sounds, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void OnQuestObjectiveCompleted(UQuestDefinition* Quest, int32 ObjectiveIndex);
	virtual void OnQuestObjectiveCompleted_Implementation(UQuestDefinition* Quest,
	                                                      int32 ObjectiveIndex);

protected:
	virtual void BeginPlay() override;

private:
	/** Helper: find the active entry for a quest definition (nullptr if not active). */
	FActiveQuestEntry* FindActiveEntry(UQuestDefinition* QuestDef);
	const FActiveQuestEntry* FindActiveEntry(UQuestDefinition* QuestDef) const;

	/** Check if all required objectives are complete and auto-complete the quest. */
	void CheckQuestCompletion(FActiveQuestEntry& Entry);

	UPROPERTY()
	TArray<FActiveQuestEntry> ActiveQuests;

	UPROPERTY()
	TSet<FName> CompletedQuestIDs;

	UPROPERTY()
	TSet<FName> FailedQuestIDs;
};
