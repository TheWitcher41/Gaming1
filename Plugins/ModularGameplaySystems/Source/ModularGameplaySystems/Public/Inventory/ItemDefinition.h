// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

/** Logging category for the Inventory system. */
DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

/**
 * Data asset that defines an item's static properties.
 *
 * Create Blueprint subclasses (e.g. BP_HealthPotion) to define specific items.
 * Assign these to your UInventoryComponent via AddItem().
 */
UCLASS(BlueprintType, Blueprintable)
class MODULARGAMEPLAYSYSTEMS_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** Short flavour text / tooltip shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	/**
	 * Maximum number of items that can share a single inventory slot.
	 * Set to 1 to make the item non-stackable.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 1;

	/** Whether the item can be activated / used by the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bCanBeUsed = false;

	/** Weight per single unit. Used for encumbrance calculations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 0.0f))
	float Weight = 0.1f;

	/**
	 * Unique identifier for this item type used when saving/loading.
	 * Should be set to a stable value and never changed after shipping.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
