#include "MicroManageTransform.h"
#include "MicroManageSelection.h"
#include "MicroManageConfig.h"
#include "MicroManageEquip.h"
//#include "mod\ModHandler.h"
//#include "mod\SemVersion.h"
//#include "SatisfactoryModLoader.h"

//void UMicroManageTransform::SomeFunc()
//{
	//FVersion Version = SML::GetModHandler().GetLoadedMod(TEXT("9cxELqcC1347T4")).ModInfo.Version;
	//FString VersionString = SML::GetModHandler().GetLoadedMod(TEXT("9cxELqcC1347T4")).ModInfo.Version.String();
	//FModContainer ModContainer;
//}

void UMicroManageTransform::TransformComponent(USceneComponent* SceneComp, const FTransform& Transform)
{
	auto SavedMobility = SceneComp->Mobility;
	SceneComp->SetMobility(EComponentMobility::Movable);
	SceneComp->SetWorldTransform(Transform, false, nullptr, ETeleportType::TeleportPhysics);
	if (SavedMobility != EComponentMobility::Movable) {
		SceneComp->SetMobility(EComponentMobility::Static);
	}
}

void UMicroManageTransform::TransformActor(AActor* Actor, const FTransform& Transform)
{
	TransformComponent(Actor->GetRootComponent(), Transform);
	System->Selection->RefreshMaterial(Actor);
}

void UMicroManageTransform::CalculateTransformData(FMicroManageTransformData& TransformData)
{
	// pre-calculcate remaining parts of TransformData struct as much as possible
	TransformData.IsLoc = !TransformData.Loc.IsNearlyZero(DELTA);
	TransformData.IsRot = !TransformData.Rot.IsNearlyZero(DELTA);
	TransformData.IsScale = !TransformData.Scale.IsNearlyZero(DELTA);
	TransformData.TransformAxis = GetTransformAxis(TransformData.Loc, TransformData.Rot);
	if (TransformData.GroupMode) {
		// calculate anchor quaternion or create pseudo-anchor quaternion if no anchor defined
		if (TransformData.Anchor) {
			TransformData.AnchorQuat = TransformData.Anchor->GetActorQuat();
		} else {
			if (TransformData.ViewRelative) { // create pseudo-anchor aligned with view vector
				TransformData.AnchorQuat = TransformData.ViewVector.Rotation().Quaternion();
			} else { // create pseudo-anchor aligned with north
				TransformData.AnchorQuat = FVector::LeftVector.Rotation().Quaternion();
			}
		}
		CalculateSimplePivot(TransformData);
	}
}

EAxis::Type UMicroManageTransform::GetTransformAxis(const FVector& Loc, const FRotator& Rot)
{
	if (!FMath::IsNearlyZero(Rot.Yaw, DELTA) || !FMath::IsNearlyZero(Loc.Z, DELTA)) {
		return EAxis::Type::Z;
	} else if (!FMath::IsNearlyZero(Rot.Pitch, DELTA) || !FMath::IsNearlyZero(Loc.Y, DELTA)) {
		return EAxis::Type::Y;
	} else if (!FMath::IsNearlyZero(Rot.Roll, DELTA) || !FMath::IsNearlyZero(Loc.X, DELTA)) {
		return EAxis::Type::X;
	} else {
		return EAxis::Type::None;
	}
}

FVector UMicroManageTransform::CalculatePivotLoc(const TArray<AActor*>& Actors, const AActor* Anchor, const AActor* Target)
{
	if (Anchor) { // pivot location set to anchor
		return Anchor->GetActorLocation();
	} else { // pivot location set to calculated center of all selected items
		// switch to Miniball algorithm eventually
		FBox Bounds = FBox(EForceInit::ForceInit);
		for (const auto& Actor : Actors) {
			if (Actor != Target) {
				Bounds += Actor->GetActorLocation();
			}
		}
		return Bounds.GetCenter();
	}
}

FVector UMicroManageTransform::CalcPivotAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat)
{
	auto ProcessAxes = [&](const FVector& VAxis, const FVector& Axis1, const FVector& Axis2) -> FVector
	{
		int Inverted = FMath::Sign(VAxis.Z);
		if (DesiredAxis == EAxis::Z) {
			return Inverted * VAxis;
		}
		float Check1 = FVector(Axis1.X, Axis1.Y, 0.f).GetSafeNormal() | ViewVector;
		float Check2 = FVector(Axis2.X, Axis2.Y, 0.f).GetSafeNormal() | ViewVector;
		if (FMath::Abs(Check1) >= FMath::Abs(Check2)) {
			return FMath::Sign(Check1) * ((DesiredAxis == EAxis::X) ? Axis1 : (Inverted * Axis2));
		}
		return FMath::Sign(Check2) * ((DesiredAxis == EAxis::X) ? Axis2 : (Inverted * -Axis1));
	};

	FVector XAxis = ActorQuat.GetAxisX();
	FVector YAxis = ActorQuat.GetAxisY();
	FVector ZAxis = ActorQuat.GetAxisZ();
	if (FMath::Abs(ZAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(ZAxis, XAxis, YAxis);
	} else if (FMath::Abs(YAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(YAxis, ZAxis, XAxis);
	}
	return ProcessAxes(XAxis, YAxis, ZAxis);
}

void UMicroManageTransform::CalcViewAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat, EAxis::Type& FoundAxis, bool& Inverted)
{
	auto ProcessAxes = [&](const FVector& VAxis, const FVector& Axis1, const FVector& Axis2) -> FVector
	{
		int Inverted = FMath::Sign(VAxis.Z);
		if (DesiredAxis == EAxis::Z) {
			return Inverted * VAxis;
		}
		float Check1 = FVector(Axis1.X, Axis1.Y, 0.f).GetSafeNormal() | ViewVector;
		float Check2 = FVector(Axis2.X, Axis2.Y, 0.f).GetSafeNormal() | ViewVector;
		if (FMath::Abs(Check1) >= FMath::Abs(Check2)) {
			return FMath::Sign(Check1) * ((DesiredAxis == EAxis::X) ? Axis1 : (Inverted * Axis2));
		}
		return FMath::Sign(Check2) * ((DesiredAxis == EAxis::X) ? Axis2 : (Inverted * -Axis1));
	};

	FVector XAxis = ActorQuat.GetAxisX();
	FVector YAxis = ActorQuat.GetAxisY();
	FVector ZAxis = ActorQuat.GetAxisZ();
	if (FMath::Abs(ZAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		//return ProcessAxes(ZAxis, XAxis, YAxis);
	} else if (FMath::Abs(YAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		//return ProcessAxes(YAxis, ZAxis, XAxis);
	}
	//return ProcessAxes(XAxis, YAxis, ZAxis);
}

void UMicroManageTransform::CalculateSimplePivot(FMicroManageTransformData& TransformData)
{
	if (TransformData.IsRot || TransformData.IsLoc) {
		TransformData.PivotAxis = CalcPivotAxis(TransformData.TransformAxis, TransformData.ViewVector, TransformData.AnchorQuat);
	}
	if (TransformData.IsRot) {
		TransformData.PivotAngle = TransformData.Rot.GetComponentForAxis(TransformData.TransformAxis);
		TransformData.PivotQuat = FQuat(TransformData.PivotAxis, FMath::DegreesToRadians(TransformData.PivotAngle));
	}
	if (TransformData.IsLoc) {
		TransformData.PivotTranslation = TransformData.PivotAxis * (TransformData.Loc | FVector::OneVector);
	}
}

void UMicroManageTransform::CalculateTargetPivot(FMicroManageTransformData& TransformData)
{
	// More generic calculation of PivotQuat, PivotAxis and PivotAngle.  Useful in move where angles are more complex.
	// Q(end) = Q(rotation) * Q(start);  pivot quat = Q(delta) = Q(end) * Q(start)^-1

	// PivotQuat = Q(delta) = Q(end) * Q(start)^-1
	TransformData.PivotQuat = TransformData.Target->GetActorQuat() * TransformData.AnchorQuat.Inverse();
	TransformData.PivotQuat.Normalize();

	// pivot axis and angle to rotate around axis
	TransformData.PivotQuat.ToAxisAndAngle(TransformData.PivotAxis, TransformData.PivotAngle);
	TransformData.PivotAxis.Normalize();
	TransformData.PivotAngle = FMath::RadiansToDegrees(TransformData.PivotAngle);

	TransformData.PivotTranslation = TransformData.Target->GetActorLocation() - TransformData.PivotLoc;
	TransformData.IsRot = true;
}

void UMicroManageTransform::TranslateAroundPivot(FTransform& Transform, const FMicroManageTransformData& TransformData)
{
	if (TransformData.IsScale) { // scale the distance to pivot location
		FVector ScaledOffset = (Transform.GetLocation() - TransformData.PivotLoc) * TransformData.Scale;
		Transform.SetLocation(TransformData.PivotLoc + ScaledOffset);
	}
	if (TransformData.IsRot) { // rotate location around pivot axis by pivot angle
		FVector RotationLoc = TransformData.PivotLoc + (TransformData.PivotAxis * ((Transform.GetLocation() - TransformData.PivotLoc) | TransformData.PivotAxis));
		FVector RotationOffset = Transform.GetLocation() - RotationLoc;
		Transform.SetLocation(RotationLoc + RotationOffset.RotateAngleAxis(TransformData.PivotAngle, TransformData.PivotAxis));
	}
}

void UMicroManageTransform::ModifyTransform(FTransform& Transform, FMicroManageTransformData& TransformData)
{
	if (TransformData.IsScale) {
		Transform.MultiplyScale3D(TransformData.Scale);
	}
	if (TransformData.IsRot) { // Q(end) = Q(delta) * Q(start)
		Transform.SetRotation(TransformData.PivotQuat * Transform.GetRotation());
	}
	if (TransformData.IsLoc) {
		Transform.AddToTranslation(TransformData.PivotTranslation);
	}
}

void UMicroManageTransform::ProcessTransform(const TArray<AActor*>& Actors, const FMicroManageTransformData& TransformData)
{
	FQuat TargetQuat;
	if (TransformData.Target) {
		TargetQuat = TransformData.Target->GetRootComponent()->GetComponentTransform().GetRotation();
	}
	FMicroManageTransformData SingleTransformData = TransformData;
	for (const auto& Actor : Actors) {
		if (System->Selection->IsValidActor(Actor)) {
			// process all "root" components
			for (const auto& ActorComp : Actor->GetComponentsByClass(USceneComponent::StaticClass())) {
				USceneComponent* SceneComp = Cast<USceneComponent>(ActorComp);
				if (SceneComp && !SceneComp->GetAttachParent()) {
					FTransform Transform = SceneComp->GetComponentTransform();
					if (TransformData.SetSame) {
						if (TransformData.GroupMode) { // use precalculated pivot values
							TranslateAroundPivot(Transform, TransformData);
							ModifyTransform(Transform, SingleTransformData);
						} else { // calculate pivot values needed for this single actor
							Transform.SetRotation(TargetQuat);
						}
					} else {
						if (TransformData.GroupMode) { // use precalculated pivot values
							TranslateAroundPivot(Transform, TransformData);
						} else { // calculate pivot values needed for this single actor
							SingleTransformData.AnchorQuat = SceneComp->GetComponentQuat();
							CalculateSimplePivot(SingleTransformData);
						}
						ModifyTransform(Transform, SingleTransformData);
					}
					TransformComponent(SceneComp, Transform);
				}
			}
			System->Selection->RefreshMaterial(Actor);
		}
	}
}

void UMicroManageTransform::GetActorOriginAndSize(AActor* Actor, FVector& Origin, FVector& Size)
{
	FVector BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);
	FBox LocalBox = Actor->CalculateComponentsBoundingBoxInLocalSpace();
	Size = (LocalBox.Max - LocalBox.Min) * Actor->GetActorScale();
}

void UMicroManageTransform::AnchorTop()
{
	if (!System->Selection->AnchorActor) {
		return;
	}
	// calculate top plane for AnchorActor
	FVector Normal;
	FVector Loc;
	if (System->Config->MMConfig.IsViewBased) {
		//CalcViewAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat, EAxis::Type& FoundAxis, bool& Inverted)
		Normal = CalcPivotAxis(EAxis::Z, System->Manager->GetCameraViewVector(), System->Selection->AnchorActor->GetActorQuat());
	} else {
		Normal = System->Selection->AnchorActor->GetActorQuat().GetAxisZ();
	}
	FPlane AlignPlane = FPlane(Normal, Loc);


	// move all other actors up or down (perpendicular to the alignment plane) so that their top is on the plane


}
