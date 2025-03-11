#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "TrainStationDebugDraw.generated.h"

class FDebugSceneProxy : public FDebugRenderSceneProxy
{
public:
	FDebugSceneProxy(const UPrimitiveComponent* InComponent);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMMONTRAIN_API UTrainStationDebugDraw : public UDebugDrawComponent
{
	GENERATED_BODY()

public:
	UTrainStationDebugDraw(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	virtual bool ShouldRecreateProxyOnUpdateTransform() const override {return true;};
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};
