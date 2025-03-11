#pragma once

#include "CoreMinimal.h"
#include "TrainStationDebugDraw.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "CommonTrainTrack.generated.h"

USTRUCT(BlueprintType)
struct FSplineConnection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool StartPointConnection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartPointConnectionDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EndPointConnection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndPointConnectionDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Active = false;
};

USTRUCT(BlueprintType)
struct FTrainStation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StationID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StationRangeDistance = 100.0f;
};

UCLASS(Blueprintable, BlueprintType)
class COMMONTRAIN_API ACommonTrainTrack : public AActor
{
	GENERATED_BODY()

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:
	ACommonTrainTrack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* SplineComponent;

	UPROPERTY()
	UTrainStationDebugDraw* TrainStationDebugDraw;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ACommonTrainTrack* StartPointConnection = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ACommonTrainTrack* EndPointConnection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ACommonTrainTrack*, FSplineConnection> ForkConnections;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTrainStation> Stations;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool ShouldLoop = false;

#pragma region Functions
	void UpdateForkConnections();
	void UpdateStartOrEnd();

	UFUNCTION(BlueprintCallable)
	ACommonTrainTrack* GetNextTrack(bool bForwardOrBackward, float Distance, bool& bForkOrSide, float& NextTrackDistance, float& DistanceBetween);

	UFUNCTION(BlueprintCallable)
	TArray<ACommonTrainTrack*> GetNextTracks(bool bForwardOrBackward, float Distance, TArray<float>& NextTrackDistance, TArray<float>& DistanceBetween);

	UFUNCTION(BlueprintCallable)
	void GetAllStations(bool bForwardOrBackward, float Distance, TArray<FTrainStation>& OutStations, TArray<float>& DistanceBetween);
	
	UFUNCTION(BlueprintCallable)
	FTrainStation GetNextStation(bool bForwardOrBackward, float Distance, float& DistanceBetween, bool& bFound);
#pragma endregion 
};
