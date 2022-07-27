// Fill out your copyright notice in the Description page of Project Settings.


#include "BCI.h"

#pragma region Utils

///////////////////////////////////////////////////////////////////////////////////////////
//		USEFUL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////

void UBCI::ParseParams()
{
	if (!SerialPort.Equals(""))
	{
		params.serial_port = std::string(TCHAR_TO_UTF8(*SerialPort));
	}

	if (!MacAddress.Equals(""))
	{
		params.mac_address = std::string(TCHAR_TO_UTF8(*MacAddress));
	}

	if (!IPAdress.Equals(""))
	{
		params.ip_address = std::string(TCHAR_TO_UTF8(*IPAdress));
	}

	if (IPPort != -1)
	{
		params.ip_port = IPPort;
	}

	if (IPProtocol != -1)
	{
		params.ip_port = IPProtocol;
	}

	if (!OtherInfo.Equals(""))
	{
		params.other_info = std::string(TCHAR_TO_UTF8(*OtherInfo));
	}

	if (!SerialNumber.Equals(""))
	{
		params.serial_number = std::string(TCHAR_TO_UTF8(*SerialNumber));
	}

	if (!File.Equals(""))
	{
		params.file = std::string(TCHAR_TO_UTF8(*File));
	}
}

// Try function very repeated in the code
int try_func(int board_id, int (*func)(int))
{
	try
	{
		return func(board_id);
	}
	catch (const BrainFlowException& err)
	{
		BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
		return NULL;
	}
}

void try_func(int board_id, std::vector<int>* res, std::vector<int>(*func)(int))
{
	try
	{
		*res = func(board_id);
	}
	catch (const BrainFlowException& err)
	{
		BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
	}
}

// Function to convert a boolean into a string so we can print it
#define BToS(b) b ? TEXT("true") : TEXT("false")

#pragma endregion Utils

#pragma region Overridden

///////////////////////////////////////////////////////////////////////////////////////////
//		OVERRIDEN FUNCTIONS	
///////////////////////////////////////////////////////////////////////////////////////////

// Sets default values for this component's properties
UBCI::UBCI()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBCI::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the board

	BoardShim::enable_dev_board_logger();

	ParseParams();

	// Create the board object
	board = new BoardShim(BoardID, params);

	// Initialize the mindfulness and restfulness models
	struct BrainFlowModelParams mindfulness_params((int)BrainFlowMetrics::MINDFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
	mindfulness_model = new MLModel(mindfulness_params);


	struct BrainFlowModelParams restfulness_params((int)BrainFlowMetrics::RESTFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
	restfulness_model = new MLModel(restfulness_params);

}


// Called every frame
void UBCI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#pragma endregion Overridden

#pragma region SessionControl

///////////////////////////////////////////////////////////////////////////////////////////
//		FUNCTIONS FOR START AND STOP A BOARD SESSION
///////////////////////////////////////////////////////////////////////////////////////////

bool UBCI::StartSession(const int buffer_size)
{
	bool success = false;
	if (board == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Board not initialized"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Board not initialized")));
		return false;
	}

	if (!board->is_prepared())
	{
		board->prepare_session();
	}

	if (board->is_prepared())
	{
		board->start_stream(buffer_size);
		restfulness_model->prepare();
		mindfulness_model->prepare();
		success = true;
	}
		
	return success;
}

void UBCI::StopSession(const bool Terminate)
{
	if (board != NULL && board->is_prepared())
	{
		board->stop_stream();

		if (Terminate) // Remove or not the session
		{
			board->release_session();
			MLModel::release_all();
		}
	}
}

void UBCI::SaveSession(const FString Path, const bool append)
{
	try
	{
		BrainFlowArray<double, 2> data = board->get_board_data();

		std::string path;

		if (Path.IsEmpty())
		{
			path = std::string(TCHAR_TO_UTF8(*SavePath));
		}
		else
		{
			path = std::string(TCHAR_TO_UTF8(*Path));
		}

		if (append)
		{
			DataFilter::write_file(data, path, "a");
		}
		else
		{
			DataFilter::write_file(data, path, "w");
		}
			
	}
	catch (const BrainFlowException& err)
	{
		BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
		UE_LOG(LogTemp, Error, TEXT("Unable to save"));
	}
}

void UBCI::InsertMarker(const float marker_id)
{
	if (!board->is_prepared())
	{
		UE_LOG(LogTemp, Error, TEXT("Board not prepared"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Board not prepared")));
		return;
	}

	board->insert_marker(marker_id);
}

#pragma endregion SessionControl

#pragma region BrainState

///////////////////////////////////////////////////////////////////////////////////////////
//		GET DATA & INFO ABOUT BRAIN STATE
///////////////////////////////////////////////////////////////////////////////////////////

void UBCI::GetMetrics(const float TimeWindow, const TEnumAsByte<Prediction> Method, double& Mindfulness, double& Restfulness, double& Timestamp)
{
	if(! board->is_prepared())
	{
		UE_LOG(LogTemp, Error, TEXT("Board not prepared"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Board not prepared")));
		return;
	}

	int sampling_rate =(int) BoardShim::get_sampling_rate(board->board_id);
	int n_samples = (int) (sampling_rate * TimeWindow);
	int time_channel = BoardShim::get_timestamp_channel(BoardID);

	if (board->get_board_data_count() >= n_samples)
	{
		try
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Data packages: %d"), board->get_board_data_count()));

			BrainFlowArray<double, 2> data = board->get_current_board_data(n_samples);

			if (time_channel != NULL)
				// Get the timestamp from the last sample
				Timestamp = data.get_address(time_channel)[n_samples - 1];

			std::vector<int> eeg_channels = BoardShim::get_eeg_channels(board->board_id);
			std::pair<double*, double*> bands = DataFilter::get_avg_band_powers(data, eeg_channels, sampling_rate, true);

			if (Method == Prediction::BF_CLASSIFIER)
			{
				Mindfulness = mindfulness_model->predict(bands.first, 5)[0];
				curr_mindfulness = Mindfulness;
				Restfulness = restfulness_model->predict(bands.first, 5)[0];
				curr_restfulness = Restfulness;
			}
			else if (Method == Prediction::RUNNING_AVG)
			{
				// normalize ratios between - 1 and 1.
				// Ratios are centered around 1.0.Tune scale to taste
				float normalize_offset = -1;
				float normalize_scale = 1.3;

				// Smoothing params
				float smoothing_weight = 0.05;

				// Tanh normalize the data
				Mindfulness = bands.first[BAND_POWERS::Beta] / bands.first[BAND_POWERS::Theta];
				Mindfulness = tanh(normalize_scale * (Mindfulness + normalize_offset));

				Restfulness = tanh(bands.first[BAND_POWERS::Alpha] / bands.first[BAND_POWERS::Theta]);
				Restfulness = tanh(normalize_scale * (Restfulness + normalize_offset));

				// Use linear interpolation with the previous measurement
				Mindfulness = (1.0 - smoothing_weight) * curr_mindfulness + smoothing_weight * Mindfulness;
				Restfulness = (1.0 - smoothing_weight) * curr_restfulness + smoothing_weight * Restfulness;

				// Update the values
				curr_mindfulness = Mindfulness;
				curr_restfulness = Restfulness;
			}


			delete[] bands.first;
			delete[] bands.second;
		}
		catch (const BrainFlowException& err)
		{
			BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
			if (board->is_prepared())
			{
				StopSession(true);
			}
		}
	}
	
}

void UBCI::GetBands(const float Window, double& Alpha, double& Beta, double& Gamma, double& Delta, double& Theta, double& Timestamp, const bool Detrend)
{
	if (!board->is_prepared())
	{
		UE_LOG(LogTemp, Error, TEXT("Board not initialized"));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Board not initialized")));
		return;
	}

	std::vector<int> eeg_channels = BoardShim::get_eeg_channels(board->board_id);

	int sampling_rate = (int)BoardShim::get_sampling_rate(board->board_id);
	int n_samples = (int)(sampling_rate * Window);

	if (board->get_board_data_count() >= n_samples)
	{
		try
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Data packages: %d"), board->get_board_data_count()));

			BrainFlowArray<double, 2> data = board->get_current_board_data(n_samples);

			if (Detrend)
			{
				for (int i = 0; i < eeg_channels.size(); i++)
					DataFilter::detrend(data.get_address(i), n_samples, (int)DetrendOperations::LINEAR);

			}

			std::pair<double*, double*> bands = DataFilter::get_avg_band_powers(data, eeg_channels, sampling_rate, true);

			Alpha = bands.first[BAND_POWERS::Alpha];
			Beta = bands.first[BAND_POWERS::Beta];
			Gamma = bands.first[BAND_POWERS::Gamma];
			Delta = bands.first[BAND_POWERS::Delta];
			Theta = bands.first[BAND_POWERS::Theta];
		}
		catch (const BrainFlowException& err)
		{
			BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("ERROR CODE: %d"), err.exit_code));
			if (board->is_prepared())
			{
				StopSession(true);
			}
		}
	}
	
}

#pragma endregion BrainState

#pragma region DeviceState

///////////////////////////////////////////////////////////////////////////////////////////
//		GET INFORMATION ABOUT THE DEVICE STATE
///////////////////////////////////////////////////////////////////////////////////////////

void UBCI::GetMotion(FVector& Gyroscope, FVector& Accelerometer, double& Timestamp)
{
	// Boolean flags
	std::vector<int> gyro_cannels;
	std::vector<int> accel_cannels;
	int time_channel;

	// Get the gyro channels
	try_func(BoardID, &gyro_cannels, &BoardShim::get_gyro_channels);

	// Get the accel channels
	try_func(BoardID, &accel_cannels, &BoardShim::get_accel_channels);

	//Get Timestamp channel
	time_channel = try_func(BoardID, &BoardShim::get_timestamp_channel);

	// Get the data	
	try
	{
		BrainFlowArray<double, 2> data = board->get_current_board_data(1);

		if (! gyro_cannels.empty())
		{
			Gyroscope.X = (float)data.get_address(gyro_cannels[0])[0];
			Gyroscope.Y = (float)data.get_address(gyro_cannels[1])[0];
			Gyroscope.Z = (float)data.get_address(gyro_cannels[2])[0];
		}

		if (! accel_cannels.empty())
		{
			Accelerometer.X = (float)data.get_address(accel_cannels[0])[0];
			Accelerometer.Y = (float)data.get_address(gyro_cannels[1])[0];
			Accelerometer.Z = (float)data.get_address(gyro_cannels[2])[0];
		}

		if (time_channel != NULL)
		{
			Timestamp = data.get_address(time_channel)[0];
		}
		
	}
	catch (const BrainFlowException& err)
	{
		BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
	}
}

void UBCI::GetDeviceStatus(float& Battery, bool& Connected, double& Timestamp)
{
	// Boolean flags
	int battery_channel;
	int time_channel;
	//bool success_battery = true, success_time=true;

	// Get the battery channels
	battery_channel = try_func(BoardID, &BoardShim::get_battery_channel);

	//Get Timestamp channel
	time_channel = try_func(BoardID, &BoardShim::get_timestamp_channel);
	// Get the data	
	try
	{
		BrainFlowArray<double, 2> data = board->get_current_board_data(1);

		Connected = true;

		if (battery_channel != NULL)
		{
			Battery = data.get_address(battery_channel)[0];
		}

		if (time_channel != NULL)
		{
			Timestamp = data.get_address(time_channel)[0];
		}

	}
	catch (const BrainFlowException& err)
	{
		BoardShim::log_message((int)LogLevels::LEVEL_ERROR, err.what());
		Connected = false;
	}
}

#pragma endregion DeviceState

#pragma region QueryFunctions

void UBCI::QueryMetrics(double& Mindfulness, double& Restfulness)
{
	Mindfulness = curr_mindfulness;
	Restfulness = curr_restfulness;
}

#pragma endregion QueryFunctions