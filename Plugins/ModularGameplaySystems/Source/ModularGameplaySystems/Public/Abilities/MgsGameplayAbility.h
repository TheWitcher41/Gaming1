// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MgsGameplayAbility.generated.h"

/** Logging category for the Ability system. */
DECLARE_LOG_CATEGORY_EXTERN(LogAbility, Log, All);

/** Current runtime state of an ability. */
UENUM(BlueprintType)
enum class EAbilityState : uint8
{
	Inactive    UMETA(DisplayName = "Inactive"),
	Active      UMETA(DisplayName = "Active"),
	OnCooldown  UMETA(DisplayName = "On Cooldown"),
};

/**
 * Base class for a single gameplay ability.
 *
 * Derive in Blueprint (e.g. BP_Ability_Fireball) or C++ to implement
 * the actual ability logic. Override ActivateAbility and EndAbility.
 *
 * @note This is a lightweight alternative to GAS suited for single-player or
 *       small multiplayer projects. For AAA-scale needs, consider GAS instead.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class MODULARGAMEPLAYSYSTEMS_API UMgsGameplayAbility : public UObject
{
	GENERATED_BODY()

public:
	/** Human-readable ability name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FText AbilityName;

	/** Unique tag / identifier for this ability type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityID;

	/** Cooldown duration in seconds after the ability ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability",
			  meta = (ClampMin = 0.0f))
	float CooldownDuration = 1.0f;

	/** Maximum activation duration (0 = instant). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability",
			  meta = (ClampMin = 0.0f))
	float ActivationDuration = 0.0f;

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/** Current lifecycle state. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	EAbilityState GetState() const { return State; }

	/** True while the ability is actively running. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsActive() const { return State == EAbilityState::Active; }

	/** True while on cooldown. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsOnCooldown() const { return State == EAbilityState::OnCooldown; }

	/** Remaining cooldown time in seconds (0 if not on cooldown). */
	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownRemaining() const;

	/** True if the ability can be activated right now. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool CanActivate() const { return State == EAbilityState::Inactive; }

	// -----------------------------------------------------------------------
	// Lifecycle (called by UAbilityComponent)
	// -----------------------------------------------------------------------

	/** Internal: activate. Do not call directly — use UAbilityComponent::TryActivateAbility. */
	void InternalActivate(UObject* InOwner);

	/** Internal: end / cancel. Do not call directly — use UAbilityComponent::CancelAbility. */
	void InternalEnd(bool bWasCancelled);

	/** Called when the cooldown expires. */
	void InternalCooldownExpired();

	// -----------------------------------------------------------------------
	// Blueprint extensibility
	// -----------------------------------------------------------------------

	/**
	 * Override to implement ability logic.
	 * Call EndAbility() when the ability is done (required for instant abilities).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void ActivateAbility();
	virtual void ActivateAbility_Implementation();

	/** Called when the ability ends (naturally or via cancel). */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void EndAbility(bool bWasCancelled);
	virtual void EndAbility_Implementation(bool bWasCancelled);

	/** Convenience: finish an active ability and start the cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CommitEnd();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UObject> AbilityOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	EAbilityState State = EAbilityState::Inactive;

	/** World time at which the cooldown started (used to compute remaining time). */
	double CooldownStartTime = 0.0;
};
