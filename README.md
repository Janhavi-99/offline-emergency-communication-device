# Offline Emergency Communication Device

## Overview

The Offline Emergency Communication Device is a portable communication system designed to provide communication during emergencies when mobile networks and internet connectivity are unavailable.

The system uses ESP32 and LoRa technology to transmit emergency messages directly between devices without depending on cellular networks or the internet.

## Objective

The main objective is to provide a low-cost and portable communication solution for emergency situations such as natural disasters, network failures, remote-area emergencies, and rescue operations.

## System Architecture

Transmitter ESP32
        |
        | LoRa Wireless Communication
        ↓
Receiver ESP32
        |
        ├── OLED Display
        ├── Buzzer
        ├── Red LED
        ├── Green LED
        ├── Accept Button
        └── Emergency History

## Hardware Used

- ESP32
- LoRa module
- 0.96-inch OLED display
- Push buttons
- Buzzer
- Green LED
- Red LED
- Jumper wires and electronic components

## Software and Technologies

- Arduino IDE
- Embedded C/C++
- LoRa communication
- U8g2 OLED library
- SPI communication
- I2C communication

## Key Features

- Offline emergency communication
- ESP32-based transmitter and receiver
- LoRa wireless communication
- Emergency message reception
- OLED status display
- 60-second emergency acceptance timer
- 45-second and 54-second warning alerts
- Buzzer and LED alerts
- Emergency acceptance button
- Emergency history storage
- Accepted / Not Accepted status indication

## Working

1. The transmitter sends an emergency message through the LoRa module.
2. The receiver ESP32 receives the LoRa packet.
3. The received emergency status is displayed on the OLED.
4. The receiver provides a 60-second period to accept the emergency.
5. Warning alerts are generated at 45 seconds and 54 seconds.
6. If the emergency is accepted, the system indicates that help is coming.
7. If the emergency is not accepted within 60 seconds, the system records it as not accepted.
8. Emergency status and history can be viewed using the receiver interface.

## Applications

- Natural disaster situations
- Communication during network failures
- Remote-area emergencies
- Rescue operations
- Emergency response systems

## Future Scope

- GPS location sharing
- Multi-node LoRa communication
- Longer-range communication
- Battery monitoring
- Sensor integration
- Improved emergency message management

## Project Status

Completed prototype.

## Author

Janhavi Sanjay Aher

B.E. Electronics and Telecommunication Engineering

JSPM's Bhivarabai Sawant Institute of Technology and Research, Wagholi, Pune
