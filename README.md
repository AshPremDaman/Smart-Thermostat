# Smart Thermostat-
## Overview

The project is a smart thermostat system developed using **Arduino** and **C++** that leverages embedded AI to provide advanced control, monitoring, and energy efficiency features. The system allows users to set a unique heating or cooling schedule for each day of the week, automatically adjusting the temperature based on occupancy, and predicts your HVAC system runtime using local weather data. All captured data is stored in an **InfluxDB** running on a **Raspberry Pi 4**, enabling easy access and analysis via a **Node-Red** interface.

## Key Features:
- **Dynamic Scheduling**: The thermostat allows users to set a different heating schedule for each day of the week, ensuring flexibility to match daily routines.
- **Automatic Adjustment**: Automatically adjusts the heating temperature when the user is away, reducing energy consumption.
- **HVAC Runtime Prediction**: The system predicts how long the furnace will run based on local weather, the user-defined schedule, and the starting temperature, providing feedback on energy efficiency.
- **Data Storage & Visualization**: All thermostat data is saved to an **InfluxDB** on a **Raspberry Pi 4**, and can be visualized through **Node-Red** for analysis and monitoring.
- **Embedded AI**: The thermostat uses AI algorithms to optimize heating patterns and provide insights into the efficiency of the heating schedule.
- **Arduino Platform**: The system is developed on **Arduino** using **FreeRTOS**, which manages multiple tasks such as reading temperature from sensors, controlling the fan, displaying data on a seven-segment display, and interacting with other components like an LED and potentiometer.

## Heating Model Simulation

A custom-built model simulates the thermostat's response to environmental and scheduling factors, estimating how long the furnace will run. After calibration, the model provides predictions that are within 5 to 20 minutes of actual furnace runtime for typical winter days.

## C++ Model for Efficiency

For faster simulations, the predictive model was ported from **Python** to **C++** using the **Boost** library. This C++ version is about 10 times faster than the original Python implementation, significantly speeding up the simulation process for all seven days' schedules, especially during server startup.

## API Endpoints

The system includes several API endpoints to access data:

- **/getDayIDs**: Retrieves the IDs and schedules for each day.
 
- **/getCycles/<day>**: Provides detailed schedule data, including temperature settings and furnace runtime for each day.
  

- **/getEpoch**: Returns the current epoch timestamp.

- **/getForecast**: Provides hourly and daily weather forecasts for accurate simulation.
 
- **/getTemporary**: Returns a temporary temperature reading.

## System Requirements & Setup

To enable the C++ simulation model, set the `use-cpp-sim` flag to `true` in the JSON configuration file. Necessary dependencies include:

- **libboost-all-dev**
- **python3.8-dev** (or the version corresponding to your setup)

You can build the shared object (.so) file required for integration with the web server by running the provided `makefile`.

## Installation

1. Clone the repository:
    ```bash
    git clone https://github.com/yourusername/ThermoStat.git
    ```

2. Install dependencies:
    ```bash
    sudo apt-get install libboost-all-dev python3.8-dev
    ```

3. Build the C++ simulation model:
    ```bash
    cd ThermoStat
    make
    ```

4. Configure the system by editing the `config.json` file to enable C++ simulation (set `"use-cpp-sim": true`).

5. Start the server:
    ```bash
    python3 server.py
    ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
