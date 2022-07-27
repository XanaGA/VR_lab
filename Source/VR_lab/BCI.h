// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>  

//#include <unistd.h>

#include "board_shim.h"
#include "BrainFlowPlugin.h"
#include "data_filter.h"
#include "ml_model.h"

#include "BCI.generated.h"

UENUM(BlueprintType)
enum Prediction {
	BF_CLASSIFIER = 0 UMETA(DisplayName = "BF_CLASSIFIER"),
	RUNNING_AVG = 1  UMETA(DisplayName = "RUNNING_AVG"),
};

enum BAND_POWERS {
	Gamma = 4,
	Beta = 3,
	Alpha = 2,
	Theta = 1,
	Delta = 0
};
	

UCLASS( ClassGroup=(BCI), meta=(BlueprintSpawnableComponent) )
class VR_LAB_API UBCI : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBCI();

private:
	// Functions that interface with BrainFlow
	UFUNCTION(BlueprintCallable)
		bool StartSession(const int buffer_size = 450000);

	UFUNCTION(BlueprintCallable)
		void StopSession(const bool Terminate = false);

	UFUNCTION(BlueprintCallable)
		void SaveSession(const FString Path, const bool append=false);

	UFUNCTION(BlueprintCallable)
		void InsertMarker(const float marker_id);

	UFUNCTION(BlueprintCallable)
		void GetMetrics(const float TimeWindow, const TEnumAsByte<Prediction> Method, double& Mindfulness, double& Restfulness, double& Timestamp);

	UFUNCTION(BlueprintCallable)
		void GetMotion(FVector& Gyroscope, FVector& Accelerometer, double& Timestamp);

	UFUNCTION(BlueprintCallable)
		void GetDeviceStatus(float& Battery, bool& Connected, double& Timestamp);

	UFUNCTION(BlueprintCallable)
		void GetBands(const float Window, double& Alpha, double& Beta, double& Gamma, double& Delta, double& Theta, double& Timestamp, const bool Detrend=true);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
		void QueryMetrics(double& Mindfulness, double& Restfulness);

// private variables
private:
	struct BrainFlowInputParams params;

	BoardShim* board;

	MLModel* mindfulness_model;

	MLModel* restfulness_model;

	double curr_mindfulness = 0.0;
	double curr_restfulness = 0.0;


	UPROPERTY(EditAnywhere)
		int8 BoardID = -1;

	UPROPERTY(EditAnywhere)
		FString SerialPort;

	UPROPERTY(EditAnywhere)
		FString MacAddress;

	UPROPERTY(EditAnywhere)
		FString IPAdress;

	UPROPERTY(EditAnywhere)
		int8 IPPort = -1;

	UPROPERTY(EditAnywhere)
		int8 IPProtocol = -1;

	UPROPERTY(EditAnywhere)
		FString OtherInfo;

	UPROPERTY(EditAnywhere)
		FString SerialNumber;

	UPROPERTY(EditAnywhere)
		FString File;

	UPROPERTY(EditAnywhere)
		FString SavePath;

// private functions
private:

	void ParseParams();
};

