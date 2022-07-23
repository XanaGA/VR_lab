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

Note:* Some of the attributes are marked as **UPROPERTY**. This allow us to modify them from the editor so the user is able to change this parameters without the need of diving in the C++ code. In its majority those concern to the parameters needed to initialize the `BoardShim` object. This way, by clicking on the BCI component placed in the level, we can see that those are accesible:
IMAGEN

### Functions
- void **ParseParams**(): It will initialize the **params** object mentioned above. Its functions is to convert the Unreal Engine types to C++ types, for example from FString to sts::string; and populate the fields of the **params** object. This is because BrainFlows functions work with C++ types, not with UE's ones. *Not Blueprint Callable*

- void  **BeginPlay**(): This is an overriden function that is called for every component in the level once the game start. Inside that we will use the `ParseParams` function to initialize the BoardShim object, which will be making the connection with the BCI device through BrainFlow. We will also initialize the `MLModel` object. *Not Blueprint Callable*

- bool  **StartSession**(*const  int  buffer_size = 450000*): This will prepare a board session, and it will start a new data stream with the desidered buffeer size. It will also prepare the `MLModel` already initialied in the `BeginPlay` function. If there are no errors I will return true, as it has succeeded.*Blueprint Callable*. IMAGEN

- void  **StopSession**(*const  bool  Terminate = false*): Stop a session if there is one active. The `Terminate` argument determines whether or not the `board` and `MLModel` are released (so they would have to be prepared again if we want to use them). *Blueprint Callable*. IMAGEN

- void  **SaveSession**(*const  FString  Path, const  bool  append=false*): Saves and flush all the data that is in the buffer. It is saved in a .csv format that the location specified by `Path`. The `append` parameter determines the how to write the data. If `true` and the file specified already exist, the data will be added at the end of this file. If `false`, it will overwrite the existing file. *Blueprint Callable*. IMAGEN

- void  **InsertMarker**(*const  float  marker_id*): It will insert a marker into the data stream. This is represented as a positive float numer that will be written in the marker channel at the timestamp that was introduced. This channel (remember channels are rows in the BrainflowArray) is always 0, except when a marker is introduces, then it takes the value of the marker. It behaves like a STIM channel. To know more about them check this [resource](https://mne.tools/stable/auto_tutorials/intro/20_events_from_raw.html#what-is-a-stim-channel) from mne documentation. This allows us to link BCI readings with events that we place in our level. **Warning:** If you are using a wireless connection you should consider that may be some latency. This fact can make some scenarios unusable. *Blueprint Callable*. IMAGEN

- void  **GetMetrics**(*const  float  TimeWindow, const  TEnumAsByte<Prediction> Method, double&  Mindfulness, double&  Restfulness, double&  Timestamp*): This function gets the data from the `BoardShim` object, `board`. With that calculates the *Mindfulness* and *Resfulness*. The number of samples taken for the calculation are such that they cover the last *n seconds* determined by the `TimeWindow` argument. It also returns the timestamp from the last sample sent by the device. The `Method` argument allows us to choose which algorithm to use in order to get the metrics. Only two methods are supported by default, but you can add your custom algorithms or classifiers:
		- *BrainFlow classifiers*: Those are the `MLModel`objects provided by dafault by BrainFlow. Those are the ones that we initialize in `BeginPlay` and prepare in `StartSession`.
		- *Running Average*: Algorithm based on the project by ChilloutCharles using [Muse in VRChat](https://github.com/ChilloutCharles/BrainFlowsIntoVRChat). It basically takes into consideration the Beta and Alpha waves ratio against Theta waves.
 You are encouraged to try your own models. BrainFlow [supports onnx](https://brainflow.org/2022-06-09-onnx/) format and UE introduced the [NNI](https://docs.unrealengine.com/5.0/en-US/API/Plugins/NeuralNetworkInference/) to use Neural Networks, which is also built on top on onnx. *Blueprint Callable*. IMAGEN

- void  **GetMotion**(*FVector&  Gyroscope, FVector&  Accelerometer, double&  Timestamp*)

## Blueprints

## Python Scripts

## Widgets
