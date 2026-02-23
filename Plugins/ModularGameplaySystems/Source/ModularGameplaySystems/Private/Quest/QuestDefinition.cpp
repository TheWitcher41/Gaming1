// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Quest/QuestDefinition.h"

FPrimaryAssetId UQuestDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("QuestDefinition"), GetFName());
}
