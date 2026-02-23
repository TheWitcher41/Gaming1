// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/ItemInstance.h"
#include "InventoryComponent.generated.h"

class UItemDefinition;
class UItemInstance;

// ---------------------------------------------------------------------------
// Delegate declarations
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded,   UItemDefinition*, ItemDef, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, UItemDefinition*, ItemDef, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

// ---------------------------------------------------------------------------

/**
 * Actor component that maintains an item inventory.
 *
 * Attach to any Actor (e.g. ACharacter) to give it an inventory with
 * add/remove/query support, optional slot limits, and automatic item stacking.
 *
 * All public functions are Blueprint-callable and the component is
 * structured for server-authority replication (call mutations on the server).
 *
 * Quick start (C++):
 * @code
 *   Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
 *   Inventory->MaxSlots = 20;
 * @endcode
 */
UCLASS(ClassGroup = "ModularGameplaySystems", BlueprintType, Blueprintable,
	   meta = (BlueprintSpawnableComponent))
class MODULARGAMEPLAYSYSTEMS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// -----------------------------------------------------------------------
	// Configuration
	// -----------------------------------------------------------------------

	/**
	 * Maximum distinct item slots available.
	 * Set to 0 for an unlimited inventory.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory",
			  meta = (ClampMin = 0))
	int32 MaxSlots = 20;

	// -----------------------------------------------------------------------
	// Events  (bind in Blueprint or C++ with AddDynamic)
	// -----------------------------------------------------------------------

	/** Fired after items are successfully added. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemAdded OnItemAdded;

	/** Fired after items are successfully removed. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	/** Fired any time the inventory contents change. */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/**
	 * Returns true if the given quantity of ItemDef can be added right now
	 * (respects MaxSlots and per-item MaxStackSize).
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanAddItem(UItemDefinition* ItemDef, int32 Quantity = 1) const;

	/** Returns the total number of ItemDef units currently held. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(UItemDefinition* ItemDef) const;

	/** Returns true if the inventory holds at least Quantity units of ItemDef. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(UItemDefinition* ItemDef, int32 Quantity = 1) const;

	/** Returns all active item stacks. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<UItemInstance*> GetAllItems() const;

	/** Number of distinct item stacks (slots) currently occupied. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetUsedSlotCount() const { return Items.Num(); }

	// -----------------------------------------------------------------------
	// Mutations  (call on server in networked games)
	// -----------------------------------------------------------------------

	/**
	 * Attempts to add Quantity units of ItemDef. Stacks into existing slots
	 * first, then opens new slots if needed.
	 *
	 * @return true if ALL requested units were added; false if inventory full.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemDefinition* ItemDef, int32 Quantity = 1);

	/**
	 * Removes Quantity units of ItemDef. Drains stacks oldest-first.
	 *
	 * @return true if ALL requested units were removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItemDefinition* ItemDef, int32 Quantity = 1);

	/** Empties the inventory entirely. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();

	// -----------------------------------------------------------------------
	// Blueprint extensibility
	// -----------------------------------------------------------------------

	/**
	 * Called when the player uses an item.
	 * Override in Blueprint to implement use logic (animation, sound, effect).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnItemUsed(UItemInstance* Item);

	// -----------------------------------------------------------------------
	// UActorComponent interface
	// -----------------------------------------------------------------------
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Called on owning client when Items replicates from server. */
	UFUNCTION()
	void OnRep_Items();

	/** The actual item stacks held by this inventory. */
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<TObjectPtr<UItemInstance>> Items;
};
