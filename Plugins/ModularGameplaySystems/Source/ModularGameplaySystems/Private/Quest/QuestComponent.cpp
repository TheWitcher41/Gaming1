// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Quest/QuestComponent.h"
#include "Quest/QuestDefinition.h"
#include "Quest/ObjectiveBase.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------

bool UQuestComponent::StartQuest(UQuestDefinition* QuestDef)
{
	if (!QuestDef)
	{
		UE_LOG(LogQuest, Warning, TEXT("StartQuest: QuestDef is null."));
		return false;
	}

	if (IsQuestActive(QuestDef) || IsQuestCompleted(QuestDef))
	{
		UE_LOG(LogQuest, Verbose, TEXT("StartQuest: '%s' already active or completed."),
			   *QuestDef->QuestID.ToString());
		return false;
	}

	FActiveQuestEntry& Entry = ActiveQuests.AddDefaulted_GetRef();
	Entry.Definition = QuestDef;

	// Instantiate runtime objective copies
	for (const TObjectPtr<UObjectiveBase>& ObjTemplate : QuestDef->Objectives)
	{
		if (ObjTemplate)
		{
			UObjectiveBase* RuntimeObj = DuplicateObject<UObjectiveBase>(ObjTemplate.Get(), this);
			RuntimeObj->Activate();
			Entry.ActiveObjectives.Add(RuntimeObj);
		}
	}

	UE_LOG(LogQuest, Log, TEXT("Quest '%s' started."), *QuestDef->QuestID.ToString());
	OnQuestStarted.Broadcast(QuestDef);
	return true;
}

void UQuestComponent::CompleteQuest(UQuestDefinition* QuestDef)
{
	if (!QuestDef)
	{
		return;
	}

	FActiveQuestEntry* Entry = FindActiveEntry(QuestDef);
	if (!Entry)
	{
		UE_LOG(LogQuest, Warning, TEXT("CompleteQuest: '%s' is not active."),
			   *QuestDef->QuestID.ToString());
		return;
	}

	ActiveQuests.RemoveAll([QuestDef](const FActiveQuestEntry& E)
	{
		return E.Definition == QuestDef;
	});

	CompletedQuestIDs.Add(QuestDef->QuestID);
	UE_LOG(LogQuest, Log, TEXT("Quest '%s' completed."), *QuestDef->QuestID.ToString());
	OnQuestCompleted.Broadcast(QuestDef);
}

void UQuestComponent::FailQuest(UQuestDefinition* QuestDef)
{
	if (!QuestDef)
	{
		return;
	}

	FActiveQuestEntry* Entry = FindActiveEntry(QuestDef);
	if (!Entry)
	{
		return;
	}

	// Fail all active objectives
	for (TObjectPtr<UObjectiveBase>& Obj : Entry->ActiveObjectives)
	{
		if (Obj && !Obj->IsCompleted())
		{
			Obj->Fail();
		}
	}

	ActiveQuests.RemoveAll([QuestDef](const FActiveQuestEntry& E)
	{
		return E.Definition == QuestDef;
	});

	FailedQuestIDs.Add(QuestDef->QuestID);
	UE_LOG(LogQuest, Log, TEXT("Quest '%s' failed."), *QuestDef->QuestID.ToString());
	OnQuestFailed.Broadcast(QuestDef);
}

void UQuestComponent::UpdateObjectiveProgress(UQuestDefinition* QuestDef,
                                               FName ObjectiveID, int32 Delta)
{
	if (!QuestDef || ObjectiveID.IsNone() || Delta <= 0)
	{
		return;
	}

	FActiveQuestEntry* Entry = FindActiveEntry(QuestDef);
	if (!Entry)
	{
		return;
	}

	for (int32 i = 0; i < Entry->ActiveObjectives.Num(); ++i)
	{
		TObjectPtr<UObjectiveBase>& Obj = Entry->ActiveObjectives[i];
		if (Obj && Obj->ObjectiveID == ObjectiveID)
		{
			const bool bJustCompleted = Obj->AddProgress(Delta);
			OnObjectiveProgress.Broadcast(QuestDef, Obj.Get());

			if (bJustCompleted)
			{
				OnQuestObjectiveCompleted(QuestDef, i);
				CheckQuestCompletion(*Entry);
			}
			return;
		}
	}

	UE_LOG(LogQuest, Warning, TEXT("UpdateObjectiveProgress: objective '%s' not found in quest '%s'."),
		   *ObjectiveID.ToString(), *QuestDef->QuestID.ToString());
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool UQuestComponent::IsQuestActive(UQuestDefinition* QuestDef) const
{
	return FindActiveEntry(QuestDef) != nullptr;
}

bool UQuestComponent::IsQuestCompleted(UQuestDefinition* QuestDef) const
{
	if (!QuestDef)
	{
		return false;
	}
	return CompletedQuestIDs.Contains(QuestDef->QuestID);
}

float UQuestComponent::GetQuestProgress(UQuestDefinition* QuestDef) const
{
	const FActiveQuestEntry* Entry = FindActiveEntry(QuestDef);
	if (!Entry || Entry->ActiveObjectives.IsEmpty())
	{
		return IsQuestCompleted(QuestDef) ? 1.0f : 0.0f;
	}

	float Total = 0.0f;
	for (const TObjectPtr<UObjectiveBase>& Obj : Entry->ActiveObjectives)
	{
		if (Obj)
		{
			Total += Obj->GetProgressFraction();
		}
	}
	return Total / Entry->ActiveObjectives.Num();
}

// ---------------------------------------------------------------------------
// Blueprint native events
// ---------------------------------------------------------------------------

void UQuestComponent::OnQuestObjectiveCompleted_Implementation(UQuestDefinition* Quest,
                                                               int32 ObjectiveIndex)
{
	// Default: no-op. Override in Blueprint or C++.
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

FActiveQuestEntry* UQuestComponent::FindActiveEntry(UQuestDefinition* QuestDef)
{
	if (!QuestDef)
	{
		return nullptr;
	}
	return ActiveQuests.FindByPredicate([QuestDef](const FActiveQuestEntry& E)
	{
		return E.Definition == QuestDef;
	});
}

const FActiveQuestEntry* UQuestComponent::FindActiveEntry(UQuestDefinition* QuestDef) const
{
	if (!QuestDef)
	{
		return nullptr;
	}
	return ActiveQuests.FindByPredicate([QuestDef](const FActiveQuestEntry& E)
	{
		return E.Definition == QuestDef;
	});
}

void UQuestComponent::CheckQuestCompletion(FActiveQuestEntry& Entry)
{
	if (!Entry.Definition)
	{
		return;
	}

	const bool bAllCompleted = Entry.ActiveObjectives.ContainsByPredicate(
		[](const TObjectPtr<UObjectiveBase>& Obj) { return Obj && !Obj->IsCompleted(); }) == false;

	if (Entry.Definition->bRequireAllObjectives && bAllCompleted)
	{
		// Copy the pointer before removal invalidates the Entry reference.
		UQuestDefinition* QuestDef = Entry.Definition.Get();
		CompleteQuest(QuestDef);
	}
}
