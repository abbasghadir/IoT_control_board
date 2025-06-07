# IoT Input Pedal Project (ESP8266 side)

This project is designed for an ESP8266 microcontroller to connect to a WiFi network, synchronize time using an NTP server, read input from the serial port, and send the data to a specified server endpoint via HTTP POST requests in JSON format.

## Features
- Connects to a WiFi network using predefined SSID and password.
- Synchronizes time with an NTP server (`pool.ntp.org`) with a configurable time offset for Iran (12,600 seconds).
- Reads input from the serial port and sends it to a server via HTTP POST.
- Periodically updates the time using the NTP client every 6 seconds.
- Handles JSON serialization for HTTP payloads.
- Provides feedback through the serial monitor for debugging and status updates.

## Requirements
- **Hardware**: ESP8266-based microcontroller (e.g., NodeMCU, Wemos D1 Mini).
- **Software**:
  - Arduino IDE or compatible development environment.
  - Required Arduino libraries:
    - `ESP8266WiFi`
    - `ESP8266HTTPClient`
    - `WiFiUdp`
    - `NTPClient`
    - `ArduinoJson`
- **Network**:
  - A WiFi network with the specified SSID and password.
  - Access to a server endpoint (e.g., `http://192.168.1.5/IoT_input_detection.php`) to handle POST requests.
- **Serial Input**: A device or program to send data to the ESP8266 via the serial port.

## Setup Instructions
1. **Install Libraries**:
   - Install the required libraries (`ESP8266WiFi`, `ESP8266HTTPClient`, `WiFiUdp`, `NTPClient`, `ArduinoJson`) through the Arduino Library Manager.
2. **Configure WiFi**:
   - Update the `ssid` and `password` constants in the code to match your WiFi network credentials.
3. **Configure Server Endpoint**:
   - Update the `endpoint_url` constant to point to your server's API endpoint.
4. **Set Time Zone**:
   - The `offset_time` is set to 12,600 seconds (3.5 hours ahead for Iran). Adjust if needed for your time zone.
5. **Upload Code**:
   - Connect the ESP8266 to your computer and upload the code using the Arduino IDE.
6. **Monitor Output**:
   - Open the Serial Monitor (baud rate: 115200) to view connection status, IP address, and debugging messages.

## Code Structure
- **`setup()`**: Initializes the serial communication, configures WiFi, and starts the NTP client.
- **`loop()`**: Continuously checks for time updates and processes serial input for HTTP POST requests.
- **`wificonfig()`**: Handles WiFi connection setup and prints the local IP address.
- **`uart_reader()`**: Reads input from the serial port and returns it as a string.
- **`time_update_event()`**: Triggers periodic NTP time updates every 6 seconds.
- **`time_update()`**: Synchronizes time with the NTP server and validates the update.
- **`http_post()`**: Sends serial input as JSON to the specified server endpoint.

## Usage
1. Power on the ESP8266 and ensure it connects to the WiFi network.
2. Send data via the serial port (e.g., using a serial terminal or another microcontroller).
3. The ESP8266 will:
   - Read the serial input.
   - Package it into a JSON payload.
   - Send it to the specified server endpoint.
4. Monitor the Serial Monitor for feedback on WiFi connection, NTP updates, and HTTP response codes.

## Notes
- Ensure the server endpoint (`endpoint_url`) is reachable and configured to accept JSON POST requests.
- The `time_sync_cmd` ("successfully") is used to confirm NTP synchronization via serial input. Modify if needed.
- The `ntp_update_time` (6,000 ms) controls how often the time is updated. Adjust for your requirements.
- If the NTP update fails, the system will restart to attempt reconnection.

## Troubleshooting
- **WiFi Connection Issues**: Verify SSID and password. Ensure the ESP8266 is within the WiFi range.
- **NTP Sync Failure**: Check internet connectivity and the NTP server URL (`pool.ntp.org`).
- **HTTP Errors**: Confirm the server endpoint is correct and the server is running.
- **Serial Input Issues**: Ensure the serial baud rate matches (115200) and the input format is correct.

<h2 style = "font-weight:bold; font-size: 24px; font-family: Times New Roman;">
Languages and Tools:
</h2>
<p align="left">
<a href="https://www.w3schools.com/cpp/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cplusplus/cplusplus-original.svg" alt="cplusplus" width="40" height="40"/> </a>
<a href="https://git-scm.com/" target="_blank" rel="noreferrer"> <img src="https://www.vectorlogo.zone/logos/git-scm/git-scm-icon.svg" alt="git" width="40" height="40"/> </a>
<a href="https://platformio.org//" target="_blank" rel="noreferrer"> <img src="https://cdn.platformio.org/images/platformio-logo.17fdc3bc.png" alt="arduino" width="40" height="40"/> </a>
</p>