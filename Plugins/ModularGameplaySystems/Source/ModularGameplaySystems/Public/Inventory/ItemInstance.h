// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemInstance.generated.h"

class UItemDefinition;

/**
 * Represents a runtime stack of items inside an inventory.
 *
 * Each UItemInstance holds a pointer to a shared UItemDefinition and
 * a current stack quantity. Quantities are always kept within
 * [0, ItemDefinition->MaxStackSize].
 */
UCLASS(BlueprintType)
class MODULARGAMEPLAYSYSTEMS_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initialise this instance. Called by UInventoryComponent when creating a new stack.
	 *
	 * @param InDefinition  The item definition (must not be null).
	 * @param InQuantity    Starting quantity (clamped to [1, MaxStackSize]).
	 */
	void Initialize(UItemDefinition* InDefinition, int32 InQuantity);

	/** Returns the shared item definition. */
	UFUNCTION(BlueprintPure, Category = "Item")
	UItemDefinition* GetDefinition() const { return ItemDefinition; }

	/** Returns the current stack size. */
	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetQuantity() const { return Quantity; }

	/**
	 * Directly set the quantity.
	 * Clamped to [0, ItemDefinition->MaxStackSize].
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(int32 NewQuantity);

	/**
	 * Add to the quantity up to MaxStackSize.
	 * @return The amount actually added.
	 */
	int32 AddQuantity(int32 Amount);

	/**
	 * Remove from the quantity down to 0.
	 * @return The amount actually removed.
	 */
	int32 RemoveQuantity(int32 Amount);

	/** Returns true when Quantity == MaxStackSize. */
	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsStackFull() const;

	/** Returns true when Quantity <= 0. */
	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsEmpty() const { return Quantity <= 0; }

	/** How many more items can be added to this stack. */
	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetRemainingCapacity() const;

protected:
	/** The static definition this instance references. */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<UItemDefinition> ItemDefinition = nullptr;

	/** Current stack quantity. */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 Quantity = 0;
};
