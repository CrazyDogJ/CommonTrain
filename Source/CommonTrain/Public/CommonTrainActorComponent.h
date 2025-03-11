#pragma once

#include "CoreMinimal.h"
#include "CommonTrainTrack.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CommonTrainActorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackChanged, ACommonTrainTrack*, PrevTrack, ACommonTrainTrack*, NextTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStationPassed, FTrainStation, TrainStation);

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class COMMONTRAIN_API UCommonTrainActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCommonTrainActorComponent();

	// Startup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Startup")
	ACommonTrainTrack* StartupTrack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Startup")
	float StartupDistance = 0.0f;

	// Train Movement
	/** Max movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Movement")
	float MaxSpeed = 600.0f;
	
	/** Max acceleration to speed up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Movement")
	float MaxAcceleration = 1000.0f;
	
	/** Max deceleration to slow down the train */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Movement")
	float MaxDeceleration = 50.0f;

	/** Max brake deceleration to stop train */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Movement")
	float MaxBrakeDeceleration = 2000.0f;
	
	// Train State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentThrottleInput = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentBrakeInput = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FGameplayTag CurrentStateTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ACommonTrainTrack* CurrentTrack = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "State|Station")
	FTrainStation NextStationInfo;

	UPROPERTY(BlueprintReadOnly, Category = "State|Station")
	float NextStationBetweenDistance = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float CurrentDistance = 0.0f;

	UPROPERTY(BlueprintAssignable)
	FOnTrackChanged OnTrackChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnStationPassed OnStationPassed;

#pragma region Functions
	// Should call it to place train on the track.
	UFUNCTION(BlueprintCallable)
	void OnConstruction();

	// Throttle input
	UFUNCTION(BlueprintCallable)
	void SetThrottleInput(const float Input) {CurrentThrottleInput = Input;}

	UFUNCTION(BlueprintCallable)
	void SetBrakeInput(const float Input) {CurrentBrakeInput = Input;}

	UFUNCTION(BlueprintCallable)
	void SetNextStation(FTrainStation InStation, float BetweenDistance) {NextStationInfo = InStation, NextStationBetweenDistance = BetweenDistance;};
	
	void SetCurrentTrack(ACommonTrainTrack* Track);
	void UpdateCurrentSpeed(float DeltaTime);
	void UpdateTransform(const ACommonTrainTrack* Track, float Distance);
	void UpdateStateTag();
	void UpdateForkConnections(float DeltaTime);
	void UpdateSideConnections(float DeltaTime);
	void UpdateNormalMoving(float DeltaTime);
	void UpdateStationPassing(float DeltaTime);
	float NextFrameDistance(float DeltaTime) const;
#pragma endregion
	
protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void BeginPlay() override;
};
