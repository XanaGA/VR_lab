# VR_lab

A repository to track the developments done in the VR Lab at UEF

## C++ Code
The **BCI component** (BCI.cpp) is the class used to make some of the BrainFlow functions accessible through Blueprints. It is supposed to appear only **once** in the scene, as this will represent the BCi device that we are using. For now we only support one-device experiments.

It has the following attributes and functions

### Attributes
- **params** (struct BrainFlowInputParams): Struct that we use when initializing the board with the constructor `BoardShim()` in BrainFlow. *NOT accesible from the editor*.
- **board** (BoardShim pointer): This is a pointer to the BrainFlow Boarshim object. We store it as an attribute to access it when needed, for example to read data. *NOT accesible from the editor*.
- **mindfulness_model** (MLModel pointer): A pointer to one of the classifiers provieded in BrainFlow. We store it as an attribute to avoid having to initialize and prepare the model every time we eant to make a prediction. *NOT accesible from the editor*.
- **restfulness_model** (MLModel pointer): The same as the previous one, but this will point to the restfulness classifier.
- **curr_mindfulness** (double): The mindfulness calculated with the newest data. It is useful in case we want to perform some calculations based on the previous measurements, for example a moving average. *NOT accesible from the editor*.
- **curr_restfulness** (double): The same as the previous one, but for the restfulness metric.
- **BoardID** (int8; UPROPERTY): ID from the BCI used, that has to be provided by the user. Check the suported boards [here](https://brainflow.readthedocs.io/en/stable/SupportedBoards.html) and its [IDs](https://brainflow.readthedocs.io/en/stable/UserAPI.html#brainflow-constants). *Accesible from the editor*.
- **SerialPort** (FString; UPROPERTY): Serial port name is used for boards which reads data from serial port. *Accesible from the editor*.
- **MacAddress** (FString; UPROPERTY): Mac address, for example its used for bluetooth based boards. *Accesible from the editor*.
- **IPAdress** (FString; UPROPERTY): Ip address is used for boards which reads data from socket connection. *Accesible from the editor*.
- **IPPort** (int8; UPROPERTY): Ip port for socket connection, for some boards where we know it in front you dont need this parameter. *Accesible from the editor*.
- **IPProtocol** (int8; UPROPERTY): Ip protocol type from IpProtocolTypes enum.  Check the available IP protocols [here](https://brainflow.readthedocs.io/en/stable/UserAPI.html#brainflow-constants). *Accesible from the editor*.
- **OtherInfo** (FString; UPROPERTY): Additional information for some boards. *Accesible from the editor*.
- **SerialNumber** (FString; UPROPERTY): Serial number of the device. *Accesible from the editor*.
- **File** (FString; UPROPERTY): Path to a configuration file. *Accesible from the editor*.
- **SavePath** (FString; UPROPERTY): Default path where store the raw data from the session read as a BrainflowArray. *Accesible from the editor*.

*Note:* Some of the attributes are marked as **UPROPERTY**. This allow us to modify them from the editor so the user is able to change this parameters without the need of diving in the C++ code. In its majority those concern to the parameters needed to initialize the `BoardShim` object.

### Functions
- void **ParseParams**(): It will initialize the **params** object mentioned above. Its functions is to convert the Unreal Engine types to C++ native types, for example from FString to sts::string; and populate the fields of the **params** object.
- 

## Blueprints

## Python Scripts

## Widgets
