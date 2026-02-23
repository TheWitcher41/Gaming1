// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Quest/ObjectiveBase.h"

DEFINE_LOG_CATEGORY(LogQuest);

float UObjectiveBase::GetProgressFraction() const
{
	if (RequiredProgress <= 0)
	{
		return 1.0f;
	}
	return FMath::Clamp(static_cast<float>(CurrentProgress) / RequiredProgress, 0.0f, 1.0f);
}

bool UObjectiveBase::AddProgress(int32 Delta)
{
	if (State != EObjectiveState::Active || Delta <= 0)
	{
		return false;
	}

	const int32 OldProgress = CurrentProgress;
	CurrentProgress = FMath::Min(CurrentProgress + Delta, RequiredProgress);

	OnProgressUpdated(OldProgress, CurrentProgress);

	if (CurrentProgress >= RequiredProgress)
	{
		State = EObjectiveState::Completed;
		UE_LOG(LogQuest, Log, TEXT("Objective '%s' completed."), *ObjectiveID.ToString());
		return true;
	}

	return false;
}

void UObjectiveBase::Activate()
{
	State = EObjectiveState::Active;
	CurrentProgress = 0;
}

void UObjectiveBase::Fail()
{
	State = EObjectiveState::Failed;
}

void UObjectiveBase::OnProgressUpdated_Implementation(int32 OldProgress, int32 NewProgress)
{
	// Default: no-op. Override in Blueprint or C++ for custom behaviour.
}
