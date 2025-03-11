#include "CommonTrainActorComponent.h"

#include "CommonTrainGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"

UCommonTrainActorComponent::UCommonTrainActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCommonTrainActorComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentDistance = StartupDistance;
	SetCurrentTrack(StartupTrack);
}

void UCommonTrainActorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!CurrentTrack)
	{
		return;
	}
	
	UpdateStateTag();
	UpdateCurrentSpeed(DeltaTime);
	UpdateForkConnections(DeltaTime);
	UpdateSideConnections(DeltaTime);
	UpdateNormalMoving(DeltaTime);
	UpdateStationPassing(DeltaTime);
}

void UCommonTrainActorComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCommonTrainActorComponent, StartupTrack) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UCommonTrainActorComponent, StartupDistance))
	{
		UpdateTransform(StartupTrack, StartupDistance);
	}
}

void UCommonTrainActorComponent::OnConstruction()
{
	if (StartupTrack)
	{
		UpdateTransform(StartupTrack, StartupDistance);
	}
}

void UCommonTrainActorComponent::SetCurrentTrack(ACommonTrainTrack* Track)
{
	if (CurrentTrack != Track)
	{
		auto PreTrack = CurrentTrack;
		CurrentTrack = Track;
		OnTrackChangedDelegate.Broadcast(PreTrack, CurrentTrack);
	}
}

void UCommonTrainActorComponent::UpdateCurrentSpeed(float DeltaTime)
{
	if (CurrentSpeed != 0.0f)
	{
		const float Deceleration = MaxDeceleration + MaxBrakeDeceleration * CurrentBrakeInput;
		const float PostDecelerateSpeed = CurrentSpeed - UKismetMathLibrary::SignOfFloat(CurrentSpeed) * DeltaTime * Deceleration;
		if (UKismetMathLibrary::SignOfFloat(PostDecelerateSpeed) != UKismetMathLibrary::SignOfFloat(CurrentSpeed))
		{
			CurrentSpeed = 0.0f;
		}
		else
		{
			CurrentSpeed = PostDecelerateSpeed;
		}
	}
	
	CurrentSpeed += CurrentThrottleInput * DeltaTime * MaxAcceleration;
	CurrentSpeed = FMath::Clamp(CurrentSpeed, -MaxSpeed, MaxSpeed);
}

void UCommonTrainActorComponent::UpdateTransform(const ACommonTrainTrack* Track, float Distance)
{
	if (!Track || !GetOwner())
	{
		return;
	}
	
	GetOwner()->SetActorTransform(FTransform(Track->SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World),
		Track->SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World)));
}

void UCommonTrainActorComponent::UpdateStateTag()
{
	if (CurrentSpeed == 0.0f) CurrentStateTag = CommonTrainTags::Stopping;
	else if (CurrentSpeed > 0.0f) CurrentStateTag = CommonTrainTags::Forward;
	else CurrentStateTag = CommonTrainTags::Backward;
}

void UCommonTrainActorComponent::UpdateForkConnections(float DeltaTime)
{
	if (CurrentSpeed == 0.0f)
	{
		return;
	}

	TTuple<ACommonTrainTrack*, FSplineConnection> PassToTrack;
	for (const auto Itr : CurrentTrack->ForkConnections)
	{
		const float NextDistance = NextFrameDistance(DeltaTime);
		if (CurrentSpeed > 0.0f)
		{
			if (Itr.Value.StartPointConnection)
			{
				const float ForkDistance = Itr.Value.StartPointConnectionDistance;
				if (CurrentDistance < ForkDistance && NextDistance >= ForkDistance && Itr.Value.Active)
				{
					PassToTrack = Itr;
					break;
				}
			}
		}
		else
		{
			if (Itr.Value.EndPointConnection)
			{
				const float ForkDistance = Itr.Value.EndPointConnectionDistance;
				if (CurrentDistance > ForkDistance && NextDistance <= ForkDistance && Itr.Value.Active)
				{
					PassToTrack = Itr;
					break;
				}
			}
		}
	}

	if (PassToTrack.Key && PassToTrack.Key != CurrentTrack)
	{
		const float NextDistance = NextFrameDistance(DeltaTime);
		const float BeginPoint = PassToTrack.Value.StartPointConnectionDistance;
		const float EndPoint = PassToTrack.Value.EndPointConnectionDistance;
		if (CurrentSpeed > 0.0f)
		{
			CurrentDistance = NextDistance - BeginPoint;
		}
		else
		{
			CurrentDistance = NextDistance - EndPoint + PassToTrack.Key->SplineComponent->GetSplineLength();
		}
		SetCurrentTrack(PassToTrack.Key);
		UpdateTransform(CurrentTrack, CurrentDistance);
	}
}

void UCommonTrainActorComponent::UpdateSideConnections(float DeltaTime)
{
	if (CurrentSpeed == 0.0f)
	{
		return;
	}

	const float NextPoint = NextFrameDistance(DeltaTime);
	if (CurrentTrack->EndPointConnection)
	{
		if (CurrentSpeed > 0.0f &&
			NextPoint >= CurrentTrack->SplineComponent->GetSplineLength() &&
			CurrentTrack != CurrentTrack->EndPointConnection)
		{
			if (auto Found = CurrentTrack->EndPointConnection->ForkConnections.Find(CurrentTrack))
			{
				CurrentDistance = NextPoint - CurrentTrack->SplineComponent->GetSplineLength() + Found->EndPointConnectionDistance;
				SetCurrentTrack(CurrentTrack->EndPointConnection);
				UpdateTransform(CurrentTrack, CurrentDistance);
			}
		}
	}

	if (CurrentTrack->StartPointConnection)
	{
		if (CurrentSpeed < 0.0f &&
			NextPoint <= 0.0f &&
			CurrentTrack != CurrentTrack->StartPointConnection)
		{
			if (auto Found = CurrentTrack->StartPointConnection->ForkConnections.Find(CurrentTrack))
			{
				CurrentDistance = NextPoint + Found->StartPointConnectionDistance;
				SetCurrentTrack(CurrentTrack->StartPointConnection);
				UpdateTransform(CurrentTrack, CurrentDistance);
			}
		}
	}
}

void UCommonTrainActorComponent::UpdateNormalMoving(float DeltaTime)
{
	if (CurrentSpeed == 0.0f)
	{
		UpdateTransform(CurrentTrack, CurrentDistance);
		return;
	}
	
	auto NextPoint = NextFrameDistance(DeltaTime);
	auto ClampedDistance = UKismetMathLibrary::FClamp(NextPoint, 0.0f, CurrentTrack->SplineComponent->GetSplineLength());
	if (CurrentDistance != ClampedDistance)
	{
		CurrentDistance = ClampedDistance;
		UpdateTransform(CurrentTrack, CurrentDistance);
	}
	else
	{
		CurrentSpeed = 0.0f;
	}
}

void UCommonTrainActorComponent::UpdateStationPassing(float DeltaTime)
{
	float NextFrameDelta = DeltaTime * CurrentSpeed;
	if (FMath::Abs(NextFrameDelta) > FMath::Abs(NextStationBetweenDistance))
	{
		OnStationPassed.Broadcast(NextStationInfo);
	}
}

float UCommonTrainActorComponent::NextFrameDistance(float DeltaTime) const
{
	return CurrentDistance + DeltaTime * CurrentSpeed;
}
