// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Abilities/AbilityComponent.h"
#include "Abilities/MgsGameplayAbility.h"

UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------

UMgsGameplayAbility* UAbilityComponent::GiveAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		UE_LOG(LogAbility, Warning, TEXT("GiveAbility: AbilityClass is null."));
		return nullptr;
	}

	// Don't duplicate
	if (UMgsGameplayAbility* Existing = FindAbilityByClass(AbilityClass))
	{
		UE_LOG(LogAbility, Verbose, TEXT("GiveAbility: '%s' already granted."),
			   *AbilityClass->GetName());
		return Existing;
	}

	UMgsGameplayAbility* NewAbility = NewObject<UMgsGameplayAbility>(this, AbilityClass);
	GrantedAbilities.Add(NewAbility);

	UE_LOG(LogAbility, Log, TEXT("Granted ability '%s'."), *AbilityClass->GetName());
	OnAbilityGranted.Broadcast(NewAbility);
	return NewAbility;
}

void UAbilityComponent::RemoveAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass)
{
	UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	if (!Ability)
	{
		return;
	}

	if (Ability->IsActive())
	{
		Ability->InternalEnd(/*bWasCancelled=*/true);
	}

	GrantedAbilities.Remove(Ability);
	UE_LOG(LogAbility, Log, TEXT("Removed ability '%s'."), *AbilityClass->GetName());
}

bool UAbilityComponent::TryActivateAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass)
{
	UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	if (!Ability)
	{
		UE_LOG(LogAbility, Warning, TEXT("TryActivateAbility: '%s' not granted."),
			   AbilityClass ? *AbilityClass->GetName() : TEXT("null"));
		return false;
	}

	if (!Ability->CanActivate())
	{
		UE_LOG(LogAbility, Verbose, TEXT("TryActivateAbility: '%s' cannot activate (state=%d)."),
			   *AbilityClass->GetName(), static_cast<int32>(Ability->GetState()));
		return false;
	}

	// Broadcast before activating so listeners see the ability in Active state.
	OnAbilityActivated.Broadcast(Ability);
	Ability->InternalActivate(GetOwner());
	return true;
}

void UAbilityComponent::CancelAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass)
{
	UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	if (Ability && Ability->IsActive())
	{
		Ability->InternalEnd(/*bWasCancelled=*/true);
		NotifyAbilityEnded(Ability);
	}
}

void UAbilityComponent::CancelAllAbilities()
{
	for (TObjectPtr<UMgsGameplayAbility>& Ability : GrantedAbilities)
	{
		if (Ability && Ability->IsActive())
		{
			Ability->InternalEnd(/*bWasCancelled=*/true);
			OnAbilityEnded.Broadcast(Ability.Get());
		}
	}
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool UAbilityComponent::HasAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass) const
{
	return FindAbilityByClass(AbilityClass) != nullptr;
}

bool UAbilityComponent::IsAbilityActive(TSubclassOf<UMgsGameplayAbility> AbilityClass) const
{
	const UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	return Ability && Ability->IsActive();
}

bool UAbilityComponent::IsAbilityOnCooldown(TSubclassOf<UMgsGameplayAbility> AbilityClass) const
{
	const UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	return Ability && Ability->IsOnCooldown();
}

float UAbilityComponent::GetAbilityCooldownRemaining(
	TSubclassOf<UMgsGameplayAbility> AbilityClass) const
{
	const UMgsGameplayAbility* Ability = FindAbilityByClass(AbilityClass);
	return Ability ? Ability->GetCooldownRemaining() : 0.0f;
}

TArray<UMgsGameplayAbility*> UAbilityComponent::GetGrantedAbilities() const
{
	TArray<UMgsGameplayAbility*> Result;
	Result.Reserve(GrantedAbilities.Num());
	for (const TObjectPtr<UMgsGameplayAbility>& Ab : GrantedAbilities)
	{
		if (Ab)
		{
			Result.Add(Ab.Get());
		}
	}
	return Result;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

UMgsGameplayAbility* UAbilityComponent::FindAbilityByClass(
	TSubclassOf<UMgsGameplayAbility> AbilityClass) const
{
	if (!AbilityClass)
	{
		return nullptr;
	}
	for (const TObjectPtr<UMgsGameplayAbility>& Ab : GrantedAbilities)
	{
		if (Ab && Ab->GetClass() == AbilityClass)
		{
			return Ab.Get();
		}
	}
	return nullptr;
}

void UAbilityComponent::NotifyAbilityEnded(UMgsGameplayAbility* Ability)
{
	if (Ability)
	{
		OnAbilityEnded.Broadcast(Ability);
	}
}
