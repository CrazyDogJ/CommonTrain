#include "TrainStationDebugDraw.h"

#include "CommonTrainTrack.h"

FDebugSceneProxy::FDebugSceneProxy(const UPrimitiveComponent* InComponent)
 : FDebugRenderSceneProxy(InComponent)
{
	DrawType = EDrawType::SolidAndWireMeshes;
	ViewFlagName = TEXT("DebugTrainStations");

	if (auto Track = Cast<ACommonTrainTrack>(InComponent->GetOwner()))
	{
		for (auto Itr : Track->Stations)
		{
			auto Loc = Track->SplineComponent->GetLocationAtDistanceAlongSpline(Itr.SplineDistance, ESplineCoordinateSpace::World);
			
			this->Texts.Add({
			Itr.StationID,
			Loc,
			FColor::Yellow
			});

			this->Spheres.Add({
				4.0f, Loc, FColor::Red
			});
		}
	}
}

FPrimitiveViewRelevance FDebugSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	auto Index = View->Family->EngineShowFlags.FindIndexByName(TEXT("DebugTrainStations"));
	
	Result.bDrawRelevance = IsShown(View) && View->Family->EngineShowFlags.GetSingleFlag(Index);
	Result.bDynamicRelevance = true;
	Result.bShadowRelevance = false;
	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	return Result;
}

UTrainStationDebugDraw::UTrainStationDebugDraw(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FEngineShowFlags::RegisterCustomShowFlag(TEXT("DebugTrainStations"), false, SFG_Normal, FText::FromString(TEXT("Debug Train Stations")));
}

FDebugRenderSceneProxy* UTrainStationDebugDraw::CreateDebugSceneProxy()
{
	FDebugSceneProxy* DebugRenderSceneProxy = new FDebugSceneProxy(this);
	return DebugRenderSceneProxy;
}

FBoxSphereBounds UTrainStationDebugDraw::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(GetOwner()->GetComponentsBoundingBox());
}
