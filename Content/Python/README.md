# Python Scripts
This folder contains Python code used, for example for visualizations. These are intended to be used to process data from a saved session into a .csv file. You can use the *SaveSession* node to do so. 

Unreal Engine 5 already has the Python plugin activated by default. If it is not the case, you have to activate it.

## Requirements
The following libraries are required:
 -  Brainflow: The Python version of the Brainflow library, to read the data from the .csv file.
 - Mne: Well known library to work with neurophysiological data.
 - Pandas
 - Numpy
 - Matplotlib
 - PyQt5

If you want You can install this libraries by hand, but you have to make sure that are installed in the Ureal Engine interpreter. You can check where is it by running `unreal.get_interpreter_executable_path()` in the python log in UE. Even though, the *init_unreal.py* will check if those are installed and install the ones that are left. 
Moreover, if you are a frequent python user you may have some of them already installed. Other oprion is to add the path where those are to libraries path inside *Python Plugin* in *Project Settings*.

## init_unreal.py
This script is run every time the editor is started. It will check if all the required libraries mentioned above are installed and, if some are not, it will install them. It will also import all the python files that contain blueprint callable functions, so they are accesible from blueprints as nodes.

##  mne_utils.py
It basically make the bridge between Brainflow and MNE. Functions:
- **create_raw_eeg_mne**(*file_path: str, board_id: int, events = False*):It reads de .csv file and creates a MNE `rawArray` object with that data. The `board_id` referes to the board with which the stored data was recorded. If `events = True`, then the marker channel is considered used as a [STIM channel](https://mne.tools/stable/auto_tutorials/intro/20_events_from_raw.html#what-is-a-stim-channel).

## PlottingFunc.py
This contains a class that inherits from `BlueprintFunctionLibrary` so the function we define inside this can be called in blueprints. Note that we need to use the `@unreal.class` and `@unreal.ufunction` decorators. The images correspond to the *Graph* editor from the widget *EUW_Visualizer*
- **PlotRawEeg**(*saved_data, board_id, duration, events*): Plots the raw data since the start until `duration`(seconds). If `events = True` it also plot the events as a vertical line.

![raw_node](https://user-images.githubusercontent.com/88030501/180707778-71513aa2-9718-4aca-9182-0b3d2355d924.png)

- **PlotBandPowers**(*saved_data, board_id, avg*): This will plot the data in the frequency spectrum. You can choose if average between all channels or plot them individually usin the `avg` argument.

![psd_node](https://user-images.githubusercontent.com/88030501/180707822-6d8d484a-73b3-4ad0-85ec-a99f8948f0cc.png)

- **PlotEvents**(*saved_data, board_id*): This will plot only the events occured.

![events_node](https://user-images.githubusercontent.com/88030501/180707859-e74909bf-5981-413d-af20-ed7d70e367a3.png)
