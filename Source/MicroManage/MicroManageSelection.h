#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageSelection.generated.h"

USTRUCT()
struct MICROMANAGE_API FSavedMaterialInterfaces
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<class UMaterialInterface*> MaterialInterfaces;

public:
	FORCEINLINE ~FSavedMaterialInterfaces() = default;
};

USTRUCT()
struct MICROMANAGE_API FSelectedActorInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<class UMeshComponent*, FSavedMaterialInterfaces> SavedMaterialInterfaces;

public:
	FORCEINLINE ~FSelectedActorInfo() = default;
};

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageSelection : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TMap<AActor*, FSelectedActorInfo> SelectedMap;

	UPROPERTY()
	TArray<UMaterialInterface*> SelectedMaterials;

	TArray<AActor*> SavedSelection;
	AActor* SavedAnchor;
	AActor* SavedTarget;

	UPROPERTY()
	bool MaterialsInitialized;

	void InitializeMaterials();

	bool SetMarker(AActor* Actor, AActor** Marker1, AActor** Marker2);

	UPROPERTY()
	UMaterialInstanceDynamic* SelectedMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* AnchorMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* TargetMaterial;

public:
	UPROPERTY()
	AActor* AnchorActor;

	UPROPERTY()
	AActor* TargetActor;

	UFUNCTION()
	void SelectNextMaterial();

	//
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void SetSelectedMaterial(TArray<UMaterialInterface*> Materials);

	//
	void ShowHologram(AActor* Actor, FSelectedActorInfo& ActorInfo);

	//
	void HideHologram(AActor* Actor, FSelectedActorInfo& ActorInfo);

	void ResetHologram(AActor* Actor);

	void RefreshMaterial(AActor* Actor);

	void RefreshMaterials(const TArray<AActor*>& Actors);

	FSelectedActorInfo* GetActorInfo(AActor* Actor);

	//
	bool IsValidActor(AActor* Actor);

	// Adds Actor to or removes Actor from the selection and updates the hologram
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	bool SelectActor(AActor* Actor, bool Select = true, bool DeleteFromMap = true);

	//
	UFUNCTION()
	bool SetAnchor(AActor* Actor);

	//
	UFUNCTION()
	bool SetTarget(AActor* Actor);

	//
	UFUNCTION()
	bool Contains(AActor* Actor);

	// Returns the number of Actors currently selected
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	int SelectCount();

	// Returns an array of all currently selected Actors
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void SelectedActors(TArray<AActor*>& Actors);

	UFUNCTION()
	void SelectedActorsNoTarget(TArray<AActor*>& Actors);

	UFUNCTION()
	void GetSelectionOrLineTrace(TArray<AActor*>& Actors);

	UFUNCTION()
	void AddAnchorTargetBoxToSelection(bool UseSides = true);

	//
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void SelectClear(bool ConfirmClicked = true);

	void SaveSelection();

	void LoadSelection();

	AActor* LineTraceFromPlayer();

public:
	FORCEINLINE ~UMicroManageSelection() = default;
};


