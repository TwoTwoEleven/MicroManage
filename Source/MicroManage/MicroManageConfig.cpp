#include "MicroManageConfig.h"
#include "SML/util/Utility.h"
#include "SML/mod/BlueprintLibrary.h"

void UMicroManageConfiguration::Init()
{
	Super::Init();
	LoadMicroManageConfig();
	LoadKeyConfigs();
	SaveMicroManageConfig();
	SaveKeyConfigs();
}

void UMicroManageConfiguration::WriteStructToConfig(const FString& ConfigName, void* StructPtr, UScriptStruct* ScriptStruct)
{
	const FString ConfigPath = SML::GetConfigDirectory() / ConfigName;
	TSharedPtr<FJsonObject> ConfigValues = USMLBlueprintLibrary::ConvertUStructToJsonObject(ScriptStruct, StructPtr);
	FString ConfigString;
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ConfigString);
	FJsonSerializer JsonSerializer;
	JsonSerializer.Serialize(ConfigValues.ToSharedRef(), JsonWriter);
	FFileHelper::SaveStringToFile(ConfigString, *ConfigPath);
}

void UMicroManageConfiguration::ReadConfigIntoStruct(const FString& ConfigName, void* StructPtr, UScriptStruct* ScriptStruct)
{
	TSharedPtr<FJsonObject> DefaultValues = USMLBlueprintLibrary::ConvertUStructToJsonObject(ScriptStruct, StructPtr);
	TSharedRef<FJsonObject> ConfigValues = DefaultValues.ToSharedRef();
	const FString ConfigPath = SML::GetConfigDirectory() / ConfigName;
	if (FPaths::FileExists(ConfigPath)) {
		FString ConfigString;
		FFileHelper::LoadFileToString(ConfigString, *ConfigPath);
		const TSharedPtr<FJsonObject> LoadedValues = SML::ParseJsonLenient(ConfigString);
		if (LoadedValues.IsValid()) {
			ConfigValues = LoadedValues.ToSharedRef();
			for (const auto& Elem : DefaultValues->Values) {
				if (!ConfigValues->HasField(Elem.Key)) {
					ConfigValues->SetField(Elem.Key, Elem.Value);
				}
			}
		}
	}
	USMLBlueprintLibrary::ConvertJsonObjectToUStruct(ConfigValues, ScriptStruct, StructPtr);
}

void UMicroManageConfiguration::SaveMicroManageConfig()
{
	WriteStructToConfig(FILENAME_CONFIG, &MMConfig, MMConfig.StaticStruct());
}

void UMicroManageConfiguration::SaveKeyConfigs()
{
	WriteStructToConfig(FILENAME_KEYS, &MMKeyConfigs, MMKeyConfigs.StaticStruct());
}

void UMicroManageConfiguration::LoadMicroManageConfig()
{
	// initialize default values and read in any existing configuration
	MMConfig.IncrementSettings.Empty();
	MMConfig.MaxTargetRangeMeters = 100.f;
	MMConfig.SelectionTolerance = 0.5f;
	MMConfig.CurrentSelectedMaterial = 0;
	MMConfig.IsGrouped = true;
	MMConfig.IsViewBased = true;
	MMConfig.IsScaleLockedLR = false;
	MMConfig.IsScaleLockedTB = false;
	MMConfig.IsScaleLockedFB = false;
	MMConfig.WarningShownForLargeMoveLag = true;
	MMConfig.CurrentIncrementSize = "Medium";
	MMConfig.IncrementSize = EIncrementSize::Medium;
	MMConfig.DoNotEditConfigFormatVersion = "1.0";
	ReadConfigIntoStruct(FILENAME_CONFIG, &MMConfig, MMConfig.StaticStruct());

	// use default values if wasn't loaded from the existing configuration
	auto CheckIncrementSetting = [&](const FMicroManageIncrement MMInc)
	{
		for (auto& ThisInc : MMConfig.IncrementSettings) {
			if (ThisInc.Size.Compare(MMInc.Size, ESearchCase::IgnoreCase) == 0) {
				ThisInc.IncrementSize = MMInc.IncrementSize;
				return;
			}
		}
		MMConfig.IncrementSettings.Add(MMInc);
	};

	CheckIncrementSetting(FMicroManageIncrement(EIncrementSize::Tiny, 1.f, 1.f, 1.f));
	CheckIncrementSetting(FMicroManageIncrement(EIncrementSize::Medium, 10.f, 5.f, 5.f));
	CheckIncrementSetting(FMicroManageIncrement(EIncrementSize::Large, 25.f, 10.f, 10.f));
	CheckIncrementSetting(FMicroManageIncrement(EIncrementSize::Huge, 100.f, 45.f, 20.f));

	// convert loaded string CurrentIncrementSize to enum IncrementSize
	for (const auto& ThisInc : MMConfig.IncrementSettings) {
		if (ThisInc.Size.Compare(MMConfig.CurrentIncrementSize, ESearchCase::IgnoreCase) == 0) {
			MMConfig.IncrementSize = ThisInc.IncrementSize;
			break;
		}
	}
}

void UMicroManageConfiguration::LoadKeyConfigs()
{
	// initialize default values and read in any existing configuration
	MMKeyConfigs.ActionKeys.Empty();
	MMKeyConfigs.DoNotEditConfigFormatVersion = "1.0";
	ReadConfigIntoStruct(FILENAME_CONFIG, &MMKeyConfigs, MMKeyConfigs.StaticStruct());

	// use default values if wasn't loaded from the existing configuration
	enum { NoAlt, Alt };
	enum { NoCtrl, Ctrl };
	enum { NoShift, Shift };
	enum { NoRepeat, Repeats };

	auto CheckKeyConfigSettings = [&](FMicroManageKeyConfig KeyConfig)
	{
		for (auto& ThisKey : MMKeyConfigs.ActionKeys) {
			if (ThisKey.ActionName.Compare(KeyConfig.ActionName, ESearchCase::IgnoreCase) == 0) {
				ThisKey.ActionIndex = KeyConfig.ActionIndex;
				ThisKey.Key = FKey(FName(*ThisKey.KeyName));
				return;
			}
		}
		MMKeyConfigs.ActionKeys.Add(KeyConfig);
	};

	CheckKeyConfigSettings(FMicroManageKeyConfig(SelectTarget, EKeys::LeftMouseButton, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(DeselectTarget, EKeys::RightMouseButton, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(Undo, EKeys::Z, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(ChangeIncSize, EKeys::I, Ctrl, Alt, NoShift, NoRepeat));
	CheckKeyConfigSettings(FMicroManageKeyConfig(KnowNotes, EKeys::K, Ctrl, Alt, NoShift, NoRepeat));
	CheckKeyConfigSettings(FMicroManageKeyConfig(ShowTools, EKeys::RightMouseButton, NoCtrl, NoAlt, NoShift, NoRepeat));
	CheckKeyConfigSettings(FMicroManageKeyConfig(SetAnchor, EKeys::LeftMouseButton, NoCtrl, NoAlt, Shift, NoRepeat));
	CheckKeyConfigSettings(FMicroManageKeyConfig(SetTarget, EKeys::RightMouseButton, NoCtrl, NoAlt, Shift, NoRepeat));

	CheckKeyConfigSettings(FMicroManageKeyConfig(SpinLeft, EKeys::J, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(SpinRight, EKeys::L, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveUp, EKeys::I, Ctrl, NoAlt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveDown, EKeys::K, Ctrl, NoAlt, NoShift, Repeats));

	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveLeft, EKeys::J, NoCtrl, Alt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveRight, EKeys::L, NoCtrl, Alt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveAway, EKeys::I, NoCtrl, Alt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(MoveToward, EKeys::K, NoCtrl, Alt, NoShift, Repeats));

	CheckKeyConfigSettings(FMicroManageKeyConfig(RollLeft, EKeys::J, NoCtrl, NoAlt, Shift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(RollRight, EKeys::L, NoCtrl, NoAlt, Shift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(PitchAway, EKeys::I, NoCtrl, NoAlt, Shift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(PitchToward, EKeys::K, NoCtrl, NoAlt, Shift, Repeats));

	CheckKeyConfigSettings(FMicroManageKeyConfig(Shrink, EKeys::J, Ctrl, Alt, NoShift, Repeats));
	CheckKeyConfigSettings(FMicroManageKeyConfig(Grow, EKeys::L, Ctrl, Alt, NoShift, Repeats));

	// add all actions that don't have default keys assigned to them
	EActionNameIdx Actions[] = { AlignLeft, AlignLRCenter, AlignRight, SpaceLR,	StackLR, LockScaleLR,
		AlignTop, AlignTBCenter, AlignBottom, SpaceTB, StackTB, LockScaleTB,
		AlignFront,AlignFBCenter, AlignBack, SpaceFB, StackFB, LockScaleFB,
		SameRotation, SameScale, SamePaint,
		SelectBoxSides, SelectBoxPivot, MoveSelection, CopySelection, NewSelection, DeleteSelection, SaveSelection, LoadSelection,
		ClearUndo, EActionNameIdx::IsGrouped, EActionNameIdx::IsViewBased, NextHologram, Settings };
	TArray<EActionNameIdx> InvalidKeyActions;
	InvalidKeyActions.Append(Actions, ARRAY_COUNT(Actions));
	for (auto ActionIdx : InvalidKeyActions) {
		CheckKeyConfigSettings(FMicroManageKeyConfig(ActionIdx, EKeys::Invalid, NoCtrl, NoAlt, NoShift, NoRepeat));
	}
}

void UMicroManageConfiguration::NextIncrementSize()
{
	MMConfig.IncrementSize = static_cast<EIncrementSize>((MMConfig.IncrementSize + 1) % 4);
	MMConfig.CurrentIncrementSize = MMConfig.IncrementSettings[MMConfig.IncrementSize].Size;
}

EIncrementSize UMicroManageConfiguration::CurrentIncrementSize()
{
	return MMConfig.IncrementSize;
}
