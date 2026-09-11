# Real-Time Li-Ion Battery SOC Estimation using EKF

ESP32-based real-time State of Charge (SOC) estimation for a Lithium-ion battery using Coulomb Counting, an experimentally derived OCV-SOC lookup model, and an Extended Kalman Filter (EKF).

---

## Overview

State of Charge (SOC) is a key parameter in Battery Management Systems (BMS), especially for electric vehicle applications.

This project implements a real-time SOC estimation system for a single Lithium-ion cell using an ESP32 microcontroller. The estimator combines:

- Coulomb Counting for SOC prediction
- Battery voltage measurement using the ESP32 ADC
- INA219 current sensing over I2C
- An experimentally derived OCV-SOC lookup table
- Extended Kalman Filter (EKF) based error correction

The system was experimentally evaluated under no-load, LED-load, and DC-motor-load conditions.

---

## Key Features

- Real-time SOC estimation on ESP32
- Coulomb Counting based SOC prediction
- OCV-SOC lookup-table based voltage model
- Extended Kalman Filter correction
- INA219 current measurement
- ADC-based battery voltage measurement
- 64-sample voltage averaging for noise reduction
- Dynamic calculation of elapsed sampling time
- Real-time monitoring of voltage, current, SOC, innovation, Kalman gain, and covariance
- Experimental validation under different load conditions

---

## System Architecture

```text
                    Li-ion Battery
                          |
             +------------+------------+
             |                         |
       Voltage Divider              INA219
             |                         |
             v                         v
        ESP32 ADC                 Current Sensor
             |                         |
             +------------+------------+
                          |
                          v
                 Coulomb Counting
                    Prediction
                          |
                          v
                       SOC_pred
                          |
                          v
                    OCV-SOC Model
                          |
                 +--------+--------+
                 |                 |
             V_predicted           H
                 |                 |
                 +--------+--------+
                          |
                          v
                    Kalman Gain
                          |
                          v
                   EKF Correction
                          |
                          v
                    Estimated SOC
