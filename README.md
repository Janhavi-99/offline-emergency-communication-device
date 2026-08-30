# Offline Emergency Communication Device

## Overview

The Offline Emergency Communication Device is a portable communication system designed to provide communication during emergencies when mobile networks and the internet are unavailable.

The system uses ESP32 and ESP-NOW technology to transmit short emergency messages directly between nearby devices without requiring a cellular network or Wi-Fi router.

## Objectives

- Enable communication during emergencies and network failures
- Provide direct device-to-device communication
- Display received messages and system status
- Develop a low-cost and portable communication solution

## Hardware

- ESP32
- OLED Display
- Electronic components

## Technologies

- ESP-NOW
- ESP32
- Embedded C/C++
- Wireless communication

## Working

1. An emergency message is entered into the device.
2. ESP32 processes the message.
3. ESP-NOW transmits the message directly to another ESP32.
4. The receiving ESP32 displays the message on the OLED.
5. Multiple devices can be used to relay messages and increase communication range.

## Applications

- Natural disasters
- Network failures
- Remote-area emergencies
- Rescue operations

## Future Scope

- Multi-hop communication
- GPS location sharing
- Battery optimization
- Emergency sensor integration
- Improved communication range

## Project Status

Completed
