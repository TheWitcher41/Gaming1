// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogInventory, Verbose, TEXT("%s: InventoryComponent ready. MaxSlots=%d"),
		   *GetOwner()->GetName(), MaxSlots);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Items);
	DOREPLIFETIME(UInventoryComponent, MaxSlots);
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool UInventoryComponent::CanAddItem(UItemDefinition* ItemDef, int32 Quantity) const
{
	if (!ItemDef || Quantity <= 0)
	{
		return false;
	}

	int32 Remaining = Quantity;

	// Fill existing stacks first
	for (const TObjectPtr<UItemInstance>& Instance : Items)
	{
		if (Instance && Instance->GetDefinition() == ItemDef)
		{
			Remaining -= Instance->GetRemainingCapacity();
			if (Remaining <= 0)
			{
				return true;
			}
		}
	}

	// Count how many new slots we would need
	const int32 NewSlotsNeeded = FMath::DivideAndRoundUp(Remaining, ItemDef->MaxStackSize);
	const int32 FreeSlots = (MaxSlots > 0) ? (MaxSlots - Items.Num()) : INT_MAX;
	return NewSlotsNeeded <= FreeSlots;
}

int32 UInventoryComponent::GetItemCount(UItemDefinition* ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	int32 Total = 0;
	for (const TObjectPtr<UItemInstance>& Instance : Items)
	{
		if (Instance && Instance->GetDefinition() == ItemDef)
		{
			Total += Instance->GetQuantity();
		}
	}
	return Total;
}

bool UInventoryComponent::HasItem(UItemDefinition* ItemDef, int32 Quantity) const
{
	return GetItemCount(ItemDef) >= Quantity;
}

TArray<UItemInstance*> UInventoryComponent::GetAllItems() const
{
	TArray<UItemInstance*> Result;
	Result.Reserve(Items.Num());
	for (const TObjectPtr<UItemInstance>& Instance : Items)
	{
		if (Instance)
		{
			Result.Add(Instance.Get());
		}
	}
	return Result;
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------

bool UInventoryComponent::AddItem(UItemDefinition* ItemDef, int32 Quantity)
{
	if (!ItemDef || Quantity <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("AddItem: invalid arguments (ItemDef=%s, Quantity=%d)."),
			   ItemDef ? *ItemDef->GetName() : TEXT("null"), Quantity);
		return false;
	}

	int32 Remaining = Quantity;

	// 1. Top up existing partial stacks
	for (TObjectPtr<UItemInstance>& Instance : Items)
	{
		if (Instance && Instance->GetDefinition() == ItemDef && !Instance->IsStackFull())
		{
			Remaining -= Instance->AddQuantity(Remaining);
			if (Remaining <= 0)
			{
				break;
			}
		}
	}

	// 2. Open new slots for the rest
	while (Remaining > 0)
	{
		if (MaxSlots > 0 && Items.Num() >= MaxSlots)
		{
			UE_LOG(LogInventory, Warning,
				   TEXT("AddItem: inventory full (MaxSlots=%d). Could not add %d x %s."),
				   MaxSlots, Remaining, *ItemDef->GetName());
			// Broadcast partial addition if any items were added
			if (Remaining < Quantity)
			{
				OnItemAdded.Broadcast(ItemDef, Quantity - Remaining);
				OnInventoryChanged.Broadcast();
			}
			return false;
		}

		UItemInstance* NewInstance = NewObject<UItemInstance>(this);
		const int32 ToAdd = FMath::Min(Remaining, ItemDef->MaxStackSize);
		NewInstance->Initialize(ItemDef, ToAdd);
		Items.Add(NewInstance);
		Remaining -= ToAdd;
	}

	UE_LOG(LogInventory, Verbose, TEXT("AddItem: added %d x %s."), Quantity, *ItemDef->GetName());
	OnItemAdded.Broadcast(ItemDef, Quantity);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(UItemDefinition* ItemDef, int32 Quantity)
{
	if (!ItemDef || Quantity <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("RemoveItem: invalid arguments."));
		return false;
	}

	if (!HasItem(ItemDef, Quantity))
	{
		UE_LOG(LogInventory, Warning,
			   TEXT("RemoveItem: not enough %s (need %d, have %d)."),
			   *ItemDef->GetName(), Quantity, GetItemCount(ItemDef));
		return false;
	}

	int32 Remaining = Quantity;
	for (int32 i = Items.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		TObjectPtr<UItemInstance>& Instance = Items[i];
		if (Instance && Instance->GetDefinition() == ItemDef)
		{
			Remaining -= Instance->RemoveQuantity(Remaining);
			if (Instance->IsEmpty())
			{
				Items.RemoveAt(i);
			}
		}
	}

	UE_LOG(LogInventory, Verbose, TEXT("RemoveItem: removed %d x %s."), Quantity, *ItemDef->GetName());
	OnItemRemoved.Broadcast(ItemDef, Quantity);
	OnInventoryChanged.Broadcast();
	return true;
}

void UInventoryComponent::ClearInventory()
{
	Items.Empty();
	OnInventoryChanged.Broadcast();
	UE_LOG(LogInventory, Verbose, TEXT("ClearInventory called."));
}
