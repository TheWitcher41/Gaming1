// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Inventory/ItemDefinition.h"

DEFINE_LOG_CATEGORY(LogInventory);

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("ItemDefinition"), GetFName());
}
