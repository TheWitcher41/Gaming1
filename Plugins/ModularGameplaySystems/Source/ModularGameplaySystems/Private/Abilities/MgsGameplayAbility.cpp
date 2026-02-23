// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#include "Abilities/MgsGameplayAbility.h"
#include "Abilities/AbilityComponent.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogAbility);

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float UMgsGameplayAbility::GetCooldownRemaining() const
{
	if (State != EAbilityState::OnCooldown || CooldownDuration <= 0.0f)
	{
		return 0.0f;
	}

	const UWorld* World = AbilityOwner ? AbilityOwner->GetWorld() : nullptr;
	if (!World)
	{
		return 0.0f;
	}

	const double Elapsed = World->GetTimeSeconds() - CooldownStartTime;
	return FMath::Max(0.0f, CooldownDuration - static_cast<float>(Elapsed));
}

// ---------------------------------------------------------------------------
// Internal lifecycle
// ---------------------------------------------------------------------------

void UMgsGameplayAbility::InternalActivate(UObject* InOwner)
{
	AbilityOwner = InOwner;
	State = EAbilityState::Active;
	UE_LOG(LogAbility, Verbose, TEXT("Ability '%s' activating."), *AbilityID.ToString());
	ActivateAbility();
}

void UMgsGameplayAbility::InternalEnd(bool bWasCancelled)
{
	if (State != EAbilityState::Active)
	{
		return;
	}

	EndAbility(bWasCancelled);

	// Start cooldown (if any)
	if (CooldownDuration > 0.0f && !bWasCancelled)
	{
		const UWorld* World = AbilityOwner ? AbilityOwner->GetWorld() : nullptr;
		CooldownStartTime = World ? World->GetTimeSeconds() : 0.0;
		State = EAbilityState::OnCooldown;

		UE_LOG(LogAbility, Verbose, TEXT("Ability '%s' cooling down for %.1fs."),
			   *AbilityID.ToString(), CooldownDuration);

		// Schedule cooldown expiry via timer on the outer UAbilityComponent
		if (UAbilityComponent* Comp = Cast<UAbilityComponent>(GetOuter()))
		{
			if (UWorld* MutableWorld = Comp->GetWorld())
			{
				FTimerHandle CooldownTimer;
				MutableWorld->GetTimerManager().SetTimer(
					CooldownTimer,
					FTimerDelegate::CreateUObject(this, &UMgsGameplayAbility::InternalCooldownExpired),
					CooldownDuration, /*bLoop=*/false);
			}
		}
	}
	else
	{
		State = EAbilityState::Inactive;
	}
}

void UMgsGameplayAbility::InternalCooldownExpired()
{
	State = EAbilityState::Inactive;
	UE_LOG(LogAbility, Verbose, TEXT("Ability '%s' cooldown expired."), *AbilityID.ToString());
}

// ---------------------------------------------------------------------------
// Blueprint native events
// ---------------------------------------------------------------------------

void UMgsGameplayAbility::ActivateAbility_Implementation()
{
	// Default: instant ability — immediately commits end.
	CommitEnd();
}

void UMgsGameplayAbility::EndAbility_Implementation(bool bWasCancelled)
{
	// Default: no-op. Override in Blueprint or C++ for cleanup logic.
}

void UMgsGameplayAbility::CommitEnd()
{
	if (State == EAbilityState::Active)
	{
		InternalEnd(/*bWasCancelled=*/false);

		// Notify the owning component
		if (UAbilityComponent* Comp = Cast<UAbilityComponent>(GetOuter()))
		{
			Comp->NotifyAbilityEnded(this);
		}
	}
}
