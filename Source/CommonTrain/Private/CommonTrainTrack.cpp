#include "CommonTrainTrack.h"

#include "TrainStationDebugDraw.h"

void ACommonTrainTrack::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	SplineComponent->SetLocationAtSplinePoint(0, FVector(0,0,0), ESplineCoordinateSpace::Local);
	SplineComponent->SetClosedLoop(ShouldLoop);
	UpdateForkConnections();
	UpdateStartOrEnd();
}

ACommonTrainTrack::ACommonTrainTrack()
{
	PrimaryActorTick.bCanEverTick = false;
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	TrainStationDebugDraw = CreateDefaultSubobject<UTrainStationDebugDraw>("TrainStationDebugDraw");
	SetRootComponent(SplineComponent);
	SplineComponent->bInputSplinePointsToConstructionScript = true;
}

void ACommonTrainTrack::UpdateForkConnections()
{
	for (auto Itr : ForkConnections)
	{
		if (!Itr.Key)
		{
			continue;
		}
		
		if (Itr.Value.StartPointConnection)
		{
			Itr.Key->StartPointConnection = this;
			const auto WorldLocation = SplineComponent->GetLocationAtDistanceAlongSpline(Itr.Value.StartPointConnectionDistance, ESplineCoordinateSpace::World);
			const auto WorldTangent = SplineComponent->GetTangentAtDistanceAlongSpline(Itr.Value.StartPointConnectionDistance, ESplineCoordinateSpace::World);

			Itr.Key->SetActorLocation(WorldLocation);
			Itr.Key->SplineComponent->SetTangentAtSplinePoint(0, WorldTangent, ESplineCoordinateSpace::World);
		}

		if (Itr.Value.EndPointConnection)
		{
			Itr.Key->EndPointConnection = this;
			const auto WorldLocation = SplineComponent->GetLocationAtDistanceAlongSpline(Itr.Value.EndPointConnectionDistance, ESplineCoordinateSpace::World);
			const auto WorldTangent = SplineComponent->GetTangentAtDistanceAlongSpline(Itr.Value.EndPointConnectionDistance, ESplineCoordinateSpace::World);

			const int LastIndex = Itr.Key->SplineComponent->GetNumberOfSplinePoints() - 1;
			Itr.Key->SplineComponent->SetLocationAtSplinePoint(LastIndex, WorldLocation, ESplineCoordinateSpace::World);
			Itr.Key->SplineComponent->SetTangentAtSplinePoint(LastIndex, WorldTangent, ESplineCoordinateSpace::World);
		}
	}
}

void ACommonTrainTrack::UpdateStartOrEnd()
{
	if (StartPointConnection)
	{
		auto Found = StartPointConnection->ForkConnections.Find(this);
		if (Found)
		{
			Found->StartPointConnectionDistance = StartPointConnection->SplineComponent->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);
		}

		StartPointConnection->UpdateForkConnections();
	}

	if (EndPointConnection)
	{
		auto Found = EndPointConnection->ForkConnections.Find(this);
		if (Found)
		{
			const int LastIndex = SplineComponent->GetNumberOfSplinePoints() - 1;
			const auto LastIndexLocation = SplineComponent->GetLocationAtSplinePoint(LastIndex, ESplineCoordinateSpace::World);
			Found->EndPointConnectionDistance = EndPointConnection->SplineComponent->GetDistanceAlongSplineAtLocation(LastIndexLocation, ESplineCoordinateSpace::World);
		}

		EndPointConnection->UpdateForkConnections();
	}
}

ACommonTrainTrack* ACommonTrainTrack::GetNextTrack(bool bForwardOrBackward, float Distance, bool& bForkOrSide, float& NextTrackDistance, float& DistanceBetween)
{
	bForkOrSide = false;
	TTuple<ACommonTrainTrack*, FSplineConnection> NextFork;
	float ItrDistance = 0.0f;
	if (bForwardOrBackward)
	{
		ItrDistance = FLT_MAX;
		for (auto Itr : ForkConnections)
		{
			if (Itr.Value.Active && Itr.Value.StartPointConnection)
			{
				if (Itr.Value.StartPointConnectionDistance > Distance &&
					Itr.Value.StartPointConnectionDistance < ItrDistance)
				{
					ItrDistance = Itr.Value.StartPointConnectionDistance;
					NextFork = Itr;
				}
			}
		}
		if (NextFork.Key)
		{
			bForkOrSide = true;
			DistanceBetween = NextFork.Value.StartPointConnectionDistance - Distance;
			NextTrackDistance = bForwardOrBackward ? 0.0f : NextFork.Key->SplineComponent->GetSplineLength();
			return NextFork.Key;
		}
		else if (EndPointConnection)
		{
			bForkOrSide = false;
			DistanceBetween = SplineComponent->GetSplineLength() - Distance;
			NextTrackDistance = bForwardOrBackward ? EndPointConnection->ForkConnections.Find(this)->EndPointConnectionDistance :
			EndPointConnection->ForkConnections.Find(this)->StartPointConnectionDistance;
			return EndPointConnection;
		}
		
		return nullptr;
	}
	else
	{
		for (auto Itr : ForkConnections)
		{
			if (Itr.Value.Active && Itr.Value.EndPointConnection)
			{
				if (Itr.Value.EndPointConnectionDistance < Distance &&
					Itr.Value.EndPointConnectionDistance > ItrDistance)
				{
					ItrDistance = Itr.Value.EndPointConnectionDistance;
					NextFork = Itr;
				}
			}
		}
		if (NextFork.Key)
		{
			bForkOrSide = true;
			DistanceBetween = NextFork.Value.EndPointConnectionDistance - Distance;
			NextTrackDistance = bForwardOrBackward ? 0.0f : NextFork.Key->SplineComponent->GetSplineLength();
			return NextFork.Key;
		}
		else if (StartPointConnection)
		{
			bForkOrSide = false;
			DistanceBetween = -Distance;
			NextTrackDistance = bForwardOrBackward ? StartPointConnection->ForkConnections.Find(this)->EndPointConnectionDistance :
			StartPointConnection->ForkConnections.Find(this)->StartPointConnectionDistance;
			return StartPointConnection;
		}
		
		return nullptr;
	}
}

TArray<ACommonTrainTrack*> ACommonTrainTrack::GetNextTracks(bool bForwardOrBackward, float Distance, TArray<float>& NextTrackDistance, TArray<float>& DistanceBetween)
{
	TArray<ACommonTrainTrack*> Result;
	bool bForkSide;
	float TempDistanceBetween = 0.0f;
	float CalcDistance = 0.0f;
	auto FirstCheck = GetNextTrack(bForwardOrBackward, Distance, bForkSide, CalcDistance, TempDistanceBetween);
	if (!FirstCheck)
	{
		return Result;
	}
	
	auto NextTrack = FirstCheck;
	float AddedDistanceBetween = 0.0f;
	while (NextTrack != nullptr)
	{
		AddedDistanceBetween += TempDistanceBetween;
		
		Result.Add(NextTrack);
		DistanceBetween.Add(AddedDistanceBetween);
		NextTrackDistance.Add(CalcDistance);
		// Prevent loop
		if (ForkConnections.Find(NextTrack))
		{
			if (ForkConnections.Find(NextTrack)->EndPointConnection &&
			ForkConnections.Find(NextTrack)->StartPointConnection &&
			ForkConnections.Find(NextTrack)->StartPointConnectionDistance >= ForkConnections.Find(NextTrack)->EndPointConnectionDistance)
			{
				break;
			}
		}
		
		NextTrack = NextTrack->GetNextTrack(bForwardOrBackward, CalcDistance, bForkSide, CalcDistance, TempDistanceBetween);
	}

	return Result;
}

void ACommonTrainTrack::GetAllStations(bool bForwardOrBackward, float Distance, TArray<FTrainStation>& OutStations,
	TArray<float>& DistanceBetween)
{
	TArray<float> NextTrackStartDistance;
	TArray<float> NextTrackBetweenDistance;
	auto NextTracks = GetNextTracks(bForwardOrBackward, Distance, NextTrackStartDistance, NextTrackBetweenDistance);

	for (int i = 0; i < NextTracks.Num(); i++)
	{
		for (auto ItrStation : NextTracks[i]->Stations)
		{
			float Dist = NextTrackBetweenDistance[i] + (ItrStation.SplineDistance - NextTracks[i]->SplineComponent->GetSplineLength());
			DistanceBetween.Add(Dist);
			OutStations.Add(ItrStation);
		}
	}

	for (auto ItrStation : Stations)
	{
		float Dist = ItrStation.SplineDistance - Distance;
		DistanceBetween.Add(Dist);
		OutStations.Add(ItrStation);
	}
}

FTrainStation ACommonTrainTrack::GetNextStation(bool bForwardOrBackward, float Distance, float& DistanceBetween, bool& bFound)
{
	TArray<float> NextTrackStartDistance;
	TArray<float> NextTrackBetweenDistance;
	auto NextTracks = GetNextTracks(bForwardOrBackward, Distance, NextTrackStartDistance, NextTrackBetweenDistance);

	NextTracks.Add(this);
	NextTrackBetweenDistance.Add(0.0f);
	
	FTrainStation Result;
	float NearestStationDistance = bForwardOrBackward ? FLT_MAX : -FLT_MAX;
	bFound = false;
	
	for (int i = 0; i < NextTracks.Num(); i++)
	{
		for (auto ItrStation : NextTracks[i]->Stations)
		{
			if (NextTracks.IsValidIndex(0) && ForkConnections.Find(NextTracks[0]))
			{
				if (bForwardOrBackward ? ItrStation.SplineDistance > ForkConnections.Find(NextTracks[0])->StartPointConnectionDistance &&
					ForkConnections.Find(NextTracks[0])->StartPointConnection :
					ItrStation.SplineDistance < ForkConnections.Find(NextTracks[0])->EndPointConnectionDistance &&
					ForkConnections.Find(NextTracks[0])->EndPointConnection)
				{
					continue;
				}
			}
			
			float TempDist = NextTracks[i] == this ? ItrStation.SplineDistance - Distance : NextTrackBetweenDistance[i] + (ItrStation.SplineDistance - NextTracks[i]->SplineComponent->GetSplineLength());
			if (bForwardOrBackward)
			{
				if (TempDist > 0 && TempDist < NearestStationDistance)
				{
					NearestStationDistance = TempDist;
					bFound = true;
					Result = ItrStation;
				}
			}
			else
			{
				if (TempDist < 0 && TempDist > NearestStationDistance)
				{
					NearestStationDistance = TempDist;
					bFound = true;
					Result = ItrStation;
				}
			}
		}
	}

	DistanceBetween = NearestStationDistance;
	return Result;
}
