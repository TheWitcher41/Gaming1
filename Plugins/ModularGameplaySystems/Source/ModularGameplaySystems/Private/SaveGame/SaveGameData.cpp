// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "SaveGame/SaveGameData.h"

DEFINE_LOG_CATEGORY(LogSaveGame);

USaveGameData::USaveGameData()
{
	SaveTimestamp = FDateTime::UtcNow();
}

void USaveGameData::StampCurrentTime()
{
	SaveTimestamp = FDateTime::UtcNow();
}
