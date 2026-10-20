# Sustainable Energy Exchange Between Households Using Solar Power Integration

A scaled-down hardware prototype of a **peer-to-peer (P2P) energy-sharing system** in which a household without solar (House B) can buy surplus solar power from a neighbouring household with solar PV (House A) during high-demand periods. ESP32 microcontrollers monitor both houses, make the switching decision automatically, and log every transfer to a real-time IoT webpage.

> **Course:** BEEE304L – Power Systems Engineering (Fall 2025–2026)
> **Guide:** Dr. S. Angalaeswari, Associate Professor, SELECT

---

## Table of Contents

- [Overview](#overview)
- [Problem Statement](#problem-statement)
- [Objectives](#objectives)
- [How It Works](#how-it-works)
- [Hardware Components](#hardware-components)
- [Test Scenarios](#test-scenarios)
- [Results](#results)
- [Conclusion](#conclusion)
- [Scope and Limitations](#scope-and-limitations)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [References](#references)
- [Team](#team)
- [License](#license)

---

## Overview

The rising global demand for electricity and the need to cut greenhouse gas emissions have accelerated the shift toward renewable energy. Solar photovoltaic (PV) technology stands out for its scalability, low environmental impact, and suitability for distributed generation.

In a conventional setup, households depend entirely on the electricity board (EB). This is inefficient during peak demand, while surplus solar energy in some homes goes unused or is fed back to the grid with limited benefit.

This project proposes a P2P power-sharing system between two houses:

- **House A**: has solar PV generation
- **House B**: relies on EB supply only

When House B's demand exceeds what the EB supplies, it procures surplus power from House A. The arrangement simulates a small localized microgrid that demonstrates the technical and economic viability of decentralized power trading.

## Problem Statement

Traditional supply frameworks treat households as independent consumers reliant solely on the EB, which causes inefficiencies at peak load. Surplus solar energy from one household often remains underutilized or is fed back to the grid with limited benefit. This project addresses the challenge of enabling **direct power trading between neighbouring houses** to optimize energy usage, reduce dependence on the central grid, and provide economic and sustainability benefits.

## Objectives

1. Design a P2P power-sharing system that enables House B to buy surplus solar energy from House A during high-demand periods.
2. Develop a control strategy that manages power flow between House A, House B, and the EB, ensuring stable and secure operation.
3. Simulate and analyze the performance, reliability, and economic impact of the system under varying consumption and generation scenarios.
4. Propose metering and billing mechanisms for fair accounting of energy exchanges between the houses.
5. Evaluate the potential reduction in carbon footprint and grid dependency from localized energy sharing.

## How It Works

The ESP32 control system runs the following decision flow:

1. **Monitor:** ESP32 continuously monitors House A and House B using the current and voltage sensors. Both the grid (EB) supply and the solar panel supply feed the system.
2. **Detect demand:** When high demand is detected in House B, the system triggers a **power request**.
3. **Check surplus:** Over Wi-Fi, the ESP32 checks whether House A has surplus solar power available.
4. **Decide:**
   - If surplus is available and meets the demand → the **DPDT contactor** is activated and power is routed from House A to House B.
   - If there is no surplus → House B **falls back to the EB** supply.
5. **Indicate:** An LED shows when a request or transfer is in progress, and the LCD shows live readings.
6. **Log:** Every transfer is recorded on the **IoT webpage** hosted by the ESP32.

> The block diagram is included in the poster (`docs/poster.pdf`).

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 microcontrollers | Core control logic, Wi-Fi communication, and web hosting for the IoT page |
| INA219 current sensor | Real-time current monitoring of the power flow |
| Voltage sensor | Real-time voltage monitoring |
| DPDT / 2-pole changeover contactor (×2) | Switches the supply source for House B |
| Relay module + buck converter | Acts as a circuit breaker and the key switching mechanism, managed by the ESP32 |
| Solar panel / solar cell (5 V) | House A's renewable source |
| 9 V LiPo battery | Simulates the EB (grid) supply |
| LED | Request / transfer indication |
| LCD display | Local display of readings |
| Cables, sockets, plugs | Wiring |
| Panel box, enclosures | Housing |

## Test Scenarios

The prototype was tested under the following conditions:

| Scenario | Condition | Expected behaviour |
|---|---|---|
| **Standby** | Low load on House B | No transfer required |
| **P2P sharing** | High load on House B, surplus available in House A | Power is transferred from A to B |
| **Grid fallback** | High load on House B, no surplus in House A | House B is supplied by the EB only |

Data recorded during testing:

- Energy supplied by the EB
- Energy supplied from House A to House B
- Reduction in EB usage for House B during sharing periods

## Results

- A functional hardware prototype was built successfully.
- The ESP32 control system performed reliably under all three test scenarios (P2P sharing, grid fallback, and standby).
- Real-time data logging through the IoT webpage recorded all energy-sharing behaviour.

<img width="1066" height="848" alt="image" src="https://github.com/user-attachments/assets/142e9c18-4fb9-4c46-8347-a0cc61926d36" />


## Conclusion

The project validated a decentralized P2P energy-sharing framework and successfully integrated distributed solar generation for localized power trading. The prototype confirms the technical viability of this architecture and its ability to optimize local energy utilization, reduce dependency on the central utility grid, and enhance local energy resilience.

## Scope and Limitations

- The project focuses on a simplified microgrid model with **two residential units** connected to a centralized supply (EB).
- Analysis covers house load profiles, solar generation patterns, power flow control, and system coordination.
- **Billing and settlement** mechanisms between households are conceptualized but **not implemented physically**.
- Grid interaction assumes a single point of connection with typical residential voltage levels and supply constraints.
- The environmental impact evaluation is limited to estimated energy savings and associated greenhouse gas reductions.
- The prototype is **scaled down** (5 V solar cells and a 9 V battery standing in for the grid) and is not intended for mains-voltage use.

## Repository Structure

> Adjust this to match your actual files.

```
.
├── README.md
├── firmware/            # ESP32 code (Arduino / PlatformIO)
├── hardware/
│   ├── schematic/       # Circuit diagram and wiring layout
│   └── bom.csv          # Bill of materials
├── webpage/             # IoT dashboard files (if separate from firmware)
├── data/                # Sample logs of real-time energy sharing
├── docs/
│   ├── poster.pdf       # Project poster
│   ├── presentation.pptx
│   └── images/          # Prototype photos, block diagram
└── LICENSE
```

## Getting Started

> Fill in the details below with your actual setup.

### Prerequisites

- Arduino IDE (or PlatformIO) with ESP32 board support
- Libraries: INA219 current sensor, LCD driver, Wi-Fi / web server (list the exact ones you used)

### Build and run

1. Wire the circuit following the schematic in `hardware/schematic/`.
2. Open the firmware in `firmware/` and set your Wi-Fi credentials.
3. Flash the code to each ESP32.
4. Power the system and open the ESP32's IP address in a browser to view the live IoT log.

## References

1. Zhang, C., et al. (2018). *Peer-to-Peer Energy Trading in a Microgrid.*
2. Morstyn, T., et al. (2018). *Using peer-to-peer energy-trading platforms to incentivize prosumers to form federated power plants.*

## Team

| Name | Registration No. |
|---|---|
| Ritvik Anish | 23BEE1018 |
| Sanjh Yadav | 23BEE1043 |

**Guide:** Dr. S. Angalaeswari, Associate Professor, SELECT
