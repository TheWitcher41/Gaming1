// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Inventory/ItemInstance.h"
#include "Inventory/ItemDefinition.h"

void UItemInstance::Initialize(UItemDefinition* InDefinition, int32 InQuantity)
{
	check(InDefinition);
	ItemDefinition = InDefinition;
	Quantity = FMath::Clamp(InQuantity, 1, InDefinition->MaxStackSize);
}

void UItemInstance::SetQuantity(int32 NewQuantity)
{
	if (!ItemDefinition)
	{
		return;
	}
	Quantity = FMath::Clamp(NewQuantity, 0, ItemDefinition->MaxStackSize);
}

int32 UItemInstance::AddQuantity(int32 Amount)
{
	if (!ItemDefinition || Amount <= 0)
	{
		return 0;
	}

	const int32 SpaceLeft = ItemDefinition->MaxStackSize - Quantity;
	const int32 ToAdd = FMath::Min(Amount, SpaceLeft);
	Quantity += ToAdd;
	return ToAdd;
}

int32 UItemInstance::RemoveQuantity(int32 Amount)
{
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 ToRemove = FMath::Min(Amount, Quantity);
	Quantity -= ToRemove;
	return ToRemove;
}

bool UItemInstance::IsStackFull() const
{
	if (!ItemDefinition)
	{
		return false;
	}
	return Quantity >= ItemDefinition->MaxStackSize;
}

int32 UItemInstance::GetRemainingCapacity() const
{
	if (!ItemDefinition)
	{
		return 0;
	}
	return FMath::Max(0, ItemDefinition->MaxStackSize - Quantity);
}
