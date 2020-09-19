#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageConfig.generated.h"

#define FILENAME_CONFIG "MicroManage-Config.cfg"
#define FILENAME_KEYS "MicroManage-Keys.cfg"

USTRUCT(BlueprintType)
struct MICROMANAGE_API FMicroManageKeyConfig
{
	GENERATED_BODY()

public:
	TEnumAsByte<EActionNameIdx> ActionIndex;
	
	FKey Key;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	FString ActionName;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	FString KeyName;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool Ctrl;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool Alt;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool Shift;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool UseRepeats;

public:
	FMicroManageKeyConfig() { }
	FMicroManageKeyConfig(EActionNameIdx InIndex, FKey InKey, bool InCtrl, bool InAlt, bool InShift, bool InRepeats)
		: ActionIndex(InIndex)
		, Key(InKey)
		, Ctrl(InCtrl)
		, Alt(InAlt)
		, Shift(InShift)
		, UseRepeats(InRepeats)
	{
		ActionName = UEnum::GetValueAsString<EActionNameIdx>(InIndex);
		KeyName = InKey.ToString();
	}
	FORCEINLINE ~FMicroManageKeyConfig() = default;
};

USTRUCT(BlueprintType)
struct MICROMANAGE_API FMicroManageKeyConfigs
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	TArray<struct FMicroManageKeyConfig> ActionKeys;

	UPROPERTY()
	FString DoNotEditConfigFormatVersion;

public:
	FORCEINLINE ~FMicroManageKeyConfigs() = default;
};

USTRUCT(BlueprintType)
struct MICROMANAGE_API FMicroManageIncrement
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Size;

	TEnumAsByte<EIncrementSize> IncrementSize;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	float CentimetersToMove;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	float DegreesToRotate;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	float PercentToGrow;

public:
	FMicroManageIncrement() { }
	FMicroManageIncrement(EIncrementSize InIncSize, float InMove, float InRotate, float InGrow)
		: IncrementSize(InIncSize)
		, CentimetersToMove(InMove)
		, DegreesToRotate(InRotate)
		, PercentToGrow(InGrow)
	{
		Size = UEnum::GetValueAsString<EIncrementSize>(InIncSize);
	}
	FORCEINLINE ~FMicroManageIncrement() = default;
};

USTRUCT(BlueprintType)
struct MICROMANAGE_API FMicroManageConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	TArray<struct FMicroManageIncrement> IncrementSettings;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	float MaxTargetRangeMeters;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	float SelectionTolerance;

	TEnumAsByte<EIncrementSize> IncrementSize;

	UPROPERTY(BlueprintReadOnly, Category = "Micro Manage")
	FString CurrentIncrementSize;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	int CurrentSelectedMaterial;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool IsGrouped;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool IsViewBased;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool IsScaleLockedLR;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool IsScaleLockedTB;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool IsScaleLockedFB;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	bool WarningShownForLargeMoveLag;

	UPROPERTY()
	FString DoNotEditConfigFormatVersion;

public:
	FORCEINLINE ~FMicroManageConfig() = default;
};

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageConfiguration : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	void WriteStructToConfig(const FString& ConfigName, void* PtrToStruct, UScriptStruct* ScriptStruct);

	void ReadConfigIntoStruct(const FString& ConfigName, void* StructPtr, UScriptStruct* ScriptStruct);

public:
	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	struct FMicroManageKeyConfigs MMKeyConfigs;

	UPROPERTY(BlueprintReadWrite, Category = "Micro Manage")
	struct FMicroManageConfig MMConfig;

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void SaveMicroManageConfig();

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void SaveKeyConfigs();

	UFUNCTION()
	void LoadMicroManageConfig();

	UFUNCTION()
	void LoadKeyConfigs();

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	void NextIncrementSize();

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	EIncrementSize CurrentIncrementSize();

	UFUNCTION(BlueprintCallable, Category = "Micro Manage")
	FKey GetKeyForAction(EActionNameIdx ActionIndex);

public:
	FORCEINLINE ~UMicroManageConfiguration() = default;
};

