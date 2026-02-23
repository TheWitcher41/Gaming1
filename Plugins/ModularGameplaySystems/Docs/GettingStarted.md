# Modular Gameplay Systems — Getting Started

> **Engine:** Unreal Engine 5.3  
> **Plugin:** `ModularGameplaySystems` (Runtime module)

---

## Installation

1. Copy (or clone) the `Plugins/ModularGameplaySystems` folder into your project's `Plugins/` directory.
2. Right-click your `.uproject` file and choose **Generate Visual Studio Project Files**.
3. Open the solution in Visual Studio (or Rider), build, and launch the editor.
4. In the editor open **Edit → Plugins**, search for **Modular Gameplay Systems**, and ensure it is enabled.

---

## System Overview

| System | Component / Class | Key APIs |
|--------|-------------------|----------|
| Inventory | `UInventoryComponent` | `AddItem`, `RemoveItem`, `GetItemCount`, `CanAddItem` |
| Quest | `UQuestComponent` | `StartQuest`, `CompleteQuest`, `UpdateObjectiveProgress` |
| Save | `USaveGameSubsystem` | `SaveGame`, `LoadGame`, `GetCurrentSaveData` |
| Ability | `UAbilityComponent` | `GiveAbility`, `TryActivateAbility`, `CancelAbility` |

---

## 1. Inventory System

### Setup

Add `UInventoryComponent` to your character (Blueprint or C++):

```cpp
// MyCharacter.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
TObjectPtr<UInventoryComponent> Inventory;

// MyCharacter.cpp — Constructor
Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
Inventory->MaxSlots = 20;
```

### Create an Item Definition

1. In the Content Browser right-click → **Blueprint Class** → search for **ItemDefinition** → name it (e.g. `BP_Item_HealthPotion`).
2. Set `Item ID`, `Display Name`, `Max Stack Size`, `Weight`, etc. in the Details panel.

### Add / Remove Items

```cpp
// Add 5 health potions
bool bAdded = Inventory->AddItem(HealthPotionDef, 5);

// Remove 1
bool bRemoved = Inventory->RemoveItem(HealthPotionDef, 1);

// Query
int32 Count  = Inventory->GetItemCount(HealthPotionDef);
bool  bHasIt = Inventory->HasItem(HealthPotionDef, 3);
```

### Events

```cpp
Inventory->OnItemAdded.AddDynamic(this, &AMyClass::HandleItemAdded);
Inventory->OnItemRemoved.AddDynamic(this, &AMyClass::HandleItemRemoved);
Inventory->OnInventoryChanged.AddDynamic(this, &AMyClass::RefreshUI);
```

---

## 2. Quest System

### Setup

Add `UQuestComponent` to the player controller or character.

### Create a Quest Definition

1. Right-click in Content Browser → **Blueprint Class** → **QuestDefinition** → name it (e.g. `DA_Quest_FindSword`).
2. Fill in `Quest ID`, `Title`, `Description`.
3. Add **Objective** entries (derive from `UObjectiveBase` and set `Objective ID`, `Required Progress`).

### Start / Progress / Complete

```cpp
// Start
QuestComp->StartQuest(FindSwordQuestDef);

// Advance an objective by 1 (e.g. each time the player collects a sword fragment)
QuestComp->UpdateObjectiveProgress(FindSwordQuestDef, FName("CollectFragments"), 1);

// Manual complete
QuestComp->CompleteQuest(FindSwordQuestDef);

// Query
bool bActive    = QuestComp->IsQuestActive(FindSwordQuestDef);
bool bCompleted = QuestComp->IsQuestCompleted(FindSwordQuestDef);
float Progress  = QuestComp->GetQuestProgress(FindSwordQuestDef);  // [0,1]
```

---

## 3. Save System

The save system is available automatically as a `UGameInstanceSubsystem`.

```cpp
USaveGameSubsystem* SaveSys = GetGameInstance()->GetSubsystem<USaveGameSubsystem>();

// Write current data to disk
SaveSys->SaveGame(TEXT("Slot_0"));

// Load
SaveSys->LoadGame(TEXT("Slot_0"));
SaveSys->OnLoadCompleted.AddDynamic(this, &AMyClass::HandleLoaded);

// Access in-memory data
USaveGameData* Data = SaveSys->GetCurrentSaveData();
Data->TotalPlayTime = GetWorld()->GetTimeSeconds();
Data->BoolFlags.Add(FName("TutorialShown"), true);

// Auto-save every 5 minutes
SaveSys->SetAutoSaveInterval(300.0f);
SaveSys->SetAutoSaveEnabled(true);
```

### Extending Save Data

Subclass `USaveGameData` in Blueprint or C++ and add your own `UPROPERTY` fields:

```cpp
UCLASS()
class UMySaveData : public USaveGameData
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    int32 PlayerLevel = 1;
};
```

Pass your subclass to `UGameplayStatics::CreateSaveGameObject()` and cast to it after loading.

---

## 4. Ability System

### Setup

Add `UAbilityComponent` to your character.

### Create an Ability

1. Right-click → **Blueprint Class** → **MgsGameplayAbility** → name it (e.g. `BP_Ability_Fireball`).
2. Set `Ability ID`, `Cooldown Duration`, `Activation Duration`.
3. Override the `ActivateAbility` event to implement effects, then call **Commit End** when done.

### Grant & Activate

```cpp
// Grant
AbilityComp->GiveAbility(UFireballAbility::StaticClass());

// Activate (e.g. on input press)
bool bOk = AbilityComp->TryActivateAbility(UFireballAbility::StaticClass());

// Query
bool  bCooldown  = AbilityComp->IsAbilityOnCooldown(UFireballAbility::StaticClass());
float TimeLeft   = AbilityComp->GetAbilityCooldownRemaining(UFireballAbility::StaticClass());

// Cancel
AbilityComp->CancelAbility(UFireballAbility::StaticClass());
```

---

## Logging

Each system has its own log category. Enable verbose output in `DefaultEngine.ini`:

```ini
[Core.Log]
LogInventory=Verbose
LogQuest=Verbose
LogSaveGame=Verbose
LogAbility=Verbose
```

---

## Further Reading

- [Unreal Engine 5.3 Programming Guide](https://docs.unrealengine.com/5.3/en-US/programming-and-scripting-in-unreal-engine/)
- [Primary Data Assets](https://docs.unrealengine.com/5.3/en-US/asset-management-in-unreal-engine/)
- [Game Instance Subsystems](https://docs.unrealengine.com/5.3/en-US/programming-subsystems-in-unreal-engine/)
