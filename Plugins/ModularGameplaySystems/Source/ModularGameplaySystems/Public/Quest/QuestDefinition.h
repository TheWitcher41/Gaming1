// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/ObjectiveBase.h"
#include "QuestDefinition.generated.h"

/**
 * Data asset that describes a quest's static configuration.
 *
 * Create Blueprint subclasses (e.g. DA_MainQuest_FindSword) to author quests
 * in the editor without writing any C++.
 */
UCLASS(BlueprintType, Blueprintable)
class MODULARGAMEPLAYSYSTEMS_API UQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable quest title shown in the journal. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FText Title;

	/** Longer description / lore text. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FText Description;

	/** Unique identifier used for save/load and scripting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	/**
	 * Ordered list of objective class defaults.
	 * QuestComponent instantiates these at runtime when the quest starts.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest",
			  Instanced)
	TArray<TObjectPtr<UObjectiveBase>> Objectives;

	/** Whether all objectives must be completed to finish the quest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	bool bRequireAllObjectives = true;

	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
