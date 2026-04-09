# Arcade basketball game - Knighthacks


## Overview
This project is a physical arcade-style basketball machine with moving hoops, basket detection, high-score tracking, and timed gameplay. We hope to implement additonal features such as different game modes.

## Current Progress
- CAD modeling of frame and leg assembly completed
- Base assembly in progress
- Placeholder Arduino firmware state machine created
- Basket scoring currently simulated through Serial input

## Planned Features
- IR beam-break basket detection
- 30-second timed game mode
- Score & high score display
- LED feedback
- Motorized hoop movement on linear rail
- Different game modes

## Firmware Status
Current firmware includes:
- `WAITING`, `PLAYING`, `GAME_OVER` states
- 30-second timer
- score tracking
- placeholder basket input using Serial Monitor

## Current Hardware / Parts Status
Current planned parts include:
- ESP32
- IR sensor module
- resistor pack
- 5V to 3.3V logic shifter
- audio amplifier converter
- breadboards and jumper wires
- servo motors
- mini basketballs
- plywood
- assorted bolts, nuts, washers
- T nuts
- timing belt
- linear rails


## Next Steps
- finalize and purchase components
- continue CAD development for backboard, hoop mount, and moving mechanism- finalize parts list
- buy electronics
- replace Serial basket simulation with IR sensor input
- add display output
- integrate LED and motor control
