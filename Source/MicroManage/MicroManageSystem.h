#pragma once

#include "CoreMinimal.h"
#include "SML/util/Logging.h"
#include "FGPlayerController.h"
#include "FGBuildable.h"
#include "MicroManageSystem.generated.h"

UENUM(BlueprintType)
enum EIncrementSize
{
	Tiny		UMETA(DisplayName = "Tiny"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	Huge		UMETA(DisplayName = "Huge")
};

UENUM(BlueprintType)
enum EActionNameIdx
{
	NoAction			UMETA(DisplayName = "No Action"),
	SelectTarget		UMETA(DisplayName = "Select Target"),
	DeselectTarget		UMETA(DisplayName = "Deselect Target"),
	Undo				UMETA(DisplayName = "Undo"),
	ChangeIncSize		UMETA(DisplayName = "Change Increment Size"),
	KnowNotes			UMETA(DisplayName = "Know Notes"),
	ShowTools			UMETA(DisplayName = "Show Tool UI"),
	SetAnchor			UMETA(DisplayName = "Set Anchor"),
	SetTarget			UMETA(DisplayName = "Set Target"),

	// keep SpinLeft to Grow together
	SpinLeft			UMETA(DisplayName = "Spin Left"),
	SpinRight			UMETA(DisplayName = "Spin Right"),
	MoveUp				UMETA(DisplayName = "Move Up"),
	MoveDown			UMETA(DisplayName = "Move Down"),
	MoveLeft			UMETA(DisplayName = "Move Left"),
	MoveRight			UMETA(DisplayName = "Move Right"),
	MoveAway			UMETA(DisplayName = "Move Away"),
	MoveToward			UMETA(DisplayName = "Move Toward"),
	RollLeft			UMETA(DisplayName = "Roll Left"),
	RollRight			UMETA(DisplayName = "Roll Right"),
	PitchAway			UMETA(DisplayName = "Pitch Away"),
	PitchToward			UMETA(DisplayName = "Pitch Toward"),
	Shrink				UMETA(DisplayName = "Shrink"),
	Grow				UMETA(DisplayName = "Grow"),
	// keep SpinLeft to Grow together

	// toolbar actions
	AlignLeft			UMETA(DisplayName = "Align Left Edges To Anchor"),
	AlignLRCenter		UMETA(DisplayName = "Align Left-Right Centers To Anchor"),
	AlignRight			UMETA(DisplayName = "Align Right Edges To Anchor"),
	SpaceLR				UMETA(DisplayName = "Space Objects Equally Left To Right"),
	StackLR				UMETA(DisplayName = "Stack Objects Left To Right"),
	LockScaleLR			UMETA(DisplayName = "Lock Scaling Left To Right"),
	AlignTop			UMETA(DisplayName = "Align Tops To Anchor"),
	AlignTBCenter		UMETA(DisplayName = "Align Top-Bottom Centers To Anchor"),
	AlignBottom			UMETA(DisplayName = "Align Bottoms To Anchor"),
	SpaceTB				UMETA(DisplayName = "Space Objects Equally Top To Bottom"),
	StackTB				UMETA(DisplayName = "Stack Objects Top To Bottom"),
	LockScaleTB			UMETA(DisplayName = "Lock Scaling Top To Bottom"),
	AlignFront			UMETA(DisplayName = "Align Front Edges To Anchor"),
	AlignFBCenter		UMETA(DisplayName = "Align Front-Back Centers To Anchor"),
	AlignBack			UMETA(DisplayName = "Align Back Edges To Anchor"),
	SpaceFB				UMETA(DisplayName = "Space Objects Equally Front To Back"),
	StackFB				UMETA(DisplayName = "Stack Objects Front To Back"),
	LockScaleFB			UMETA(DisplayName = "Lock Scaling Front To Back"),

	SameRotation		UMETA(DisplayName = "Set Selection to Same Rotation as Target"),
	SameScale			UMETA(DisplayName = "Set Selection to Same Scale as Target"),
	SamePaint			UMETA(DisplayName = "Set Selection to Same Paint as Target"),

	Connect				UMETA(DisplayName = "Connect Anchor Output to Target Input"),
	Disconnect			UMETA(DisplayName = "Disconnect Anchor Outputs from Target Inputs"),
	SelectBoxSides		UMETA(DisplayName = "Select items between Anchor and Target Sides"),
	SelectBoxPivot		UMETA(DisplayName = "Select items between Anchor and Target Centers"),
	MoveSelection		UMETA(DisplayName = "Move Selection from Anchor to Target"),
	CopySelection		UMETA(DisplayName = "Copy Selection from Anchor to Target"),
	NewSelection		UMETA(DisplayName = "Start New Selection"),
	DeleteSelection		UMETA(DisplayName = "Delete Selection"),
	SaveSelection		UMETA(DisplayName = "Save Selection"),
	LoadSelection		UMETA(DisplayName = "Load Selection"),
	ClearUndo			UMETA(DisplayName = "Clear Saved Undo Information"),

	IsGrouped			UMETA(DisplayName = "Group Selection"),
	IsViewBased			UMETA(DisplayName = "Use View Based Actions"),
	NextHologram		UMETA(DisplayName = "Switch To Next Hologram Style"),
	Settings			UMETA(DisplayName = "Settings")
};

class AMicroManageEquip;
class UMicroManageRCO;

// UMicroManageSystem -----------------------------------------------------------------------------

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageSystem : public UObject
{
	GENERATED_BODY()

private:
	static UMicroManageSystem* MicroManageSystemSingleton;

public:
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	static UMicroManageSystem* Get();

public: // Components =============================================================================
	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageUndo* Undo;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageConfiguration* Config;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageSelection* Selection;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageTransform* Transform;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageUI* UI;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageAction* Action;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage Component")
	class UMicroManageInput* Input;

public: // System Access Properties ===============================================================
	AFGPlayerController* Controller;
	AFGCharacterPlayer* Character;
	AMicroManageEquip* Manager;
	UMicroManageRCO* MMRCO;
	int ID;

private:
	void BasicTransform(EActionNameIdx ActionIndex);

public:
	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void ExecuteAction(EActionNameIdx ActionIndex);

public:
	FORCEINLINE ~UMicroManageSystem() = default;
};

UMicroManageSystem* UMicroManageSystem::MicroManageSystemSingleton = 0;

// UMicroManageComponent --------------------------------------------------------------------------

UCLASS()
class MICROMANAGE_API UMicroManageComponent : public UObject
{
	GENERATED_BODY()

protected:
	class UMicroManageSystem* System;

public:
	virtual void Init() { System = UMicroManageSystem::Get(); }

public:
	FORCEINLINE ~UMicroManageComponent() = default;
};

template<typename C>
typename C* InitComponent() {
	UMicroManageComponent* Comp = NewObject<C>();
	Comp->Init();
	return Cast<C>(Comp);
}
