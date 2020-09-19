#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageTransform.generated.h"

USTRUCT()
struct MICROMANAGE_API FMicroManageTransformData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector Loc;

	UPROPERTY()
	bool IsLoc;

	UPROPERTY()
	FRotator Rot;

	UPROPERTY()
	bool IsRot;

	UPROPERTY()
	FVector Scale;

	UPROPERTY()
	bool IsScale;

	UPROPERTY()
	TEnumAsByte<EAxis::Type> TransformAxis;

	UPROPERTY()
	FQuat PivotQuat;

	UPROPERTY()
	FVector PivotLoc;

	UPROPERTY()
	FVector PivotAxis;

	UPROPERTY()
	float PivotAngle;

	UPROPERTY()
	FVector PivotTranslation;

	UPROPERTY()
	AActor* Anchor;

	UPROPERTY()
	AActor* Target;

	UPROPERTY()
	FVector ViewVector;

	UPROPERTY()
	FQuat AnchorQuat;

	UPROPERTY()
	bool ViewRelative = true;

	UPROPERTY()
	bool GroupMode = true;

	UPROPERTY()
	bool SetSame = false;

public:
	FORCEINLINE ~FMicroManageTransformData() = default;
};

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageTransform : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	// Modifies the location component of Transform for a grouped component.
	//
	// If there is scaling:
	//    The location is adjusted towards or away from the pivot location to match the
	//    scale change.
	//
	// If there is rotation:
	//    The location is adjusted based on the perpendicular rotation around PivotAxis
	//    by the angle PivotAngle.
	void TranslateAroundPivot(FTransform& Transform, const FMicroManageTransformData& TransformData);

	// Applies the scaling (Scale), rotation (PivotQuat) and translation (PivotTranslation)
	// values from TransformData to Transform for final placement of the component.
	//
	void ModifyTransform(FTransform& Transform, FMicroManageTransformData& TransformData);

public:
	// Applies Transform to the component, SceneComp
	//
	void TransformComponent(USceneComponent* SceneComp, const FTransform& Transform);

	// Applies Transform on the root component of Actor

	void TransformActor(AActor* Actor, const FTransform& Transform);

	// Calculates remaining fields of an initialized TransformData for further processing

	void CalculateTransformData(FMicroManageTransformData& TransformData);

	// Assumes a single non-zero field in Loc and Rot and returns the axis of that field

	EAxis::Type GetTransformAxis(const FVector& Loc, const FRotator& Rot);

	// Returns the axis of Quat designated by Axis

	FVector GetQuatAxis(EAxis::Type Axis, FQuat& Quat);

	// Returns the location of Anchor if defined.  If Anchor is not defined then returns
	// the calculated center of all selected objects.

	FVector CalculatePivotLoc(const TArray<AActor*>& Actors, const AActor* Anchor, const AActor* Target);

	// The X, Y & Z axes are calculated from the provided ActorQuat.  Then those axes
	// are reoriented based on the natural up direction and the provided ViewVector to
	// be front/back, left/right, and up/down normalized.  The pivot axis, based on the
	// rotation DesiredAxis, is then returned.

	FVector CalcPivotAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat);

	void CalcViewAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat, EAxis::Type& FoundAxis, bool& Inverted);

	// Calculates PivotAxis based on the TransformData values: TransformAxis, ViewVector, 
	// and AnchorQuat.  
	//
	// If the transform is a rotation:
	//    PivotAngle is retrieved from the non-zero component of TransformData.Rot and 
	//    PivotQuat is calculated as the rotation around PivotAxis by PivotAngle.
	//
	// If the transform is a translation:
	//    PivotTranslation is calculated as the desired movement, TransformData.Loc, along
	//    PivotAxis.
	void CalculateSimplePivot(FMicroManageTransformData& TransformData);

	// Calculates PivotQuat, PivotAxis, PivotAngle, and PivotTranslation based on the
	// difference between TransformData.Target and the (pseudo)Anchor values.  Used with
	// MoveSelection and SetSameRotation type functions to perform complex rotations.
	void CalculateTargetPivot(FMicroManageTransformData& TransformData);

	// Transforms all "root" components of the supplied Actors based on TransformData.
	//
	// If grouped:
	//    The precalculated transform values based on the (pseudo)Anchor are used.
	//
	// If ungrouped:
	//    Transform values are calculated for each component relative to itself.
	void ProcessTransform(const TArray<AActor*>& Actors, const FMicroManageTransformData& TransformData);


	// Calculated the center and size of an actor's bounding box.  Size is local world values, not rotated world values.
	void GetActorOriginAndSize(AActor* Actor, FVector& Origin, FVector& Size);

	void AlignToActor(AActor* Anchor, int Position, const EAxis::Type DesiredAxis);

public:
	FORCEINLINE ~UMicroManageTransform() = default;
};