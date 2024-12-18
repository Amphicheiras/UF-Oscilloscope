# Oscilloscope Audio Plugin

  <img src="https://github.com/Amphicheiras/UF-Oscilloscope/blob/dev-amphicheiras/github/uf00.png?raw=true" alt="right" width="250">

## Features

- Input gain to scale Y axis
- Input buffer length to scale X axis
- Sync button to match the draw rate with the BPM of the DAW
- Multi-Channel Monitoring (TBA)
  - Sidechain (only 1 channel)
  - Utility plugin instances on every channel you want to draw it's waveform (messy)
  - Route "Audio To" the Plugin's channel (audio goes only through that channel (not master channel directly), cannot send to more than 1 oscilloscope instances)
  - ?

## Getting Started

### Prerequisites

- JUCE Framework: Required for building and running the plugin. Download from JUCE.
- Supported DAWs: Any Digital Audio Workstation that supports VST, AU, or standalone plugins.
- Compiler: Ensure a compatible C++ compiler is installed. For Windows, MSVC is recommended, while Xcode works well on macOS.

### Installation

1. Clone the repository:
    ```sh
     git clone https://github.com/Amphicheiras/UF-Oscilloscope.git
     cd UF-Oscilloscope
    ```

2. Have a look at the /root and /plugin CMakeLists.txt

2. Configure with CMake:
   ```sh
   cmake -S . -B build
   ```

3. Compile with CMake:
   ```sh
   cmake --build build
   ```

4. Place "UF0/UF00/resources/images" in your OS's "Documents" folder.

### Usage

1. Load the Plugin:
   After compiling, load the plugin in a supported DAW or run as a standalone application if built as such.
2. Place the oscillator in a channel and voila!

### Code Overview


### Parameters


### Contributing

Contributions are welcome! Please fork this repository, create a branch, and submit a pull request for any improvements or bug fixes.

### Issues

If you encounter bugs or have feature requests, please report them in the Issues section of this repository.

### License

This project is licensed under the GNU GPL-3.0 License - see the LICENSE file for details.

### Acknowledgments

Special thanks to the open-source audio community and JUCE for their continuous support and resources.

Super special thanks to Jan Wilczek!

# Useful Links

- [JUCE with CMake](https://www.youtube.com/watch?v=Uq7Hwt18s3s)
