// Copyright (c) 2024 TheWitcher41. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/MgsGameplayAbility.h"
#include "AbilityComponent.generated.h"

class UMgsGameplayAbility;

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityGranted,    UMgsGameplayAbility*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityActivated,  UMgsGameplayAbility*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEnded,      UMgsGameplayAbility*, Ability);

// ---------------------------------------------------------------------------

/**
 * Actor component that manages a set of gameplay abilities for an actor.
 *
 * Attach to a Character or Pawn to give it abilities that can be granted,
 * activated, queried, and cancelled from C++ or Blueprints.
 */
UCLASS(ClassGroup = "ModularGameplaySystems", BlueprintType, Blueprintable,
	   meta = (BlueprintSpawnableComponent))
class MODULARGAMEPLAYSYSTEMS_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityComponent();

	// -----------------------------------------------------------------------
	// Events
	// -----------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityGranted OnAbilityGranted;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityActivated OnAbilityActivated;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityEnded OnAbilityEnded;

	// -----------------------------------------------------------------------
	// Mutations
	// -----------------------------------------------------------------------

	/**
	 * Grant a new ability to this actor. Creates a runtime instance from
	 * AbilityClass. Does nothing if the ability class is already granted.
	 *
	 * @return The new (or existing) ability instance, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UMgsGameplayAbility* GiveAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass);

	/**
	 * Remove and destroy a previously granted ability.
	 * Cancels the ability first if it is currently active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void RemoveAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass);

	/**
	 * Attempt to activate an ability.
	 *
	 * @return true if the ability was successfully activated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool TryActivateAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass);

	/**
	 * Cancel an active ability (triggers EndAbility with bWasCancelled=true).
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CancelAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass);

	/** Cancel all currently active abilities. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void CancelAllAbilities();

	// -----------------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------------

	/** Returns true if AbilityClass has been granted to this component. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool HasAbility(TSubclassOf<UMgsGameplayAbility> AbilityClass) const;

	/** Returns true if the ability is currently active. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsAbilityActive(TSubclassOf<UMgsGameplayAbility> AbilityClass) const;

	/** Returns true if the ability is on cooldown. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsAbilityOnCooldown(TSubclassOf<UMgsGameplayAbility> AbilityClass) const;

	/** Returns the remaining cooldown time in seconds (0 if not on cooldown). */
	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetAbilityCooldownRemaining(TSubclassOf<UMgsGameplayAbility> AbilityClass) const;

	/** Returns all granted ability instances. */
	UFUNCTION(BlueprintPure, Category = "Ability")
	TArray<UMgsGameplayAbility*> GetGrantedAbilities() const;

	// -----------------------------------------------------------------------
	// UActorComponent interface
	// -----------------------------------------------------------------------
	virtual void BeginPlay() override;

private:
	/** Find a granted ability instance by class, or nullptr. */
	UMgsGameplayAbility* FindAbilityByClass(TSubclassOf<UMgsGameplayAbility> AbilityClass) const;

	/** Called by ability instances when they naturally end. */
	void NotifyAbilityEnded(UMgsGameplayAbility* Ability);

	friend class UMgsGameplayAbility;

	UPROPERTY()
	TArray<TObjectPtr<UMgsGameplayAbility>> GrantedAbilities;
};
