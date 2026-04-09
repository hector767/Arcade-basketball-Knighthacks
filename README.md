# Arcade Basketball Game – KnightHacks

## Overview
This project is a physical arcade-style basketball machine featuring moving hoops, real-time basket detection, score tracking, and timed gameplay. The system integrates mechanical design, embedded firmware, and electronics to recreate an arcade experience using custom-built hardware.

---

## Current Progress

### Build Status (~85% Complete)
- Base structure fully built  
- Backboard installed  
- Ball return ramp attached  
- Control panel cut and prepared  

**Remaining work:**
- Attach leg frame  
- Complete electronics integration  

---

### CAD / Mechanical
- Frame and leg assembly fully designed and implemented  
- Base assembly completed  
- Linear rail and hoop system modeled  
- CAD components integrated into the full assembly  

---

### Firmware
- State machine implemented for game logic  
- Supports:
  - Start screen  
  - Player selection (single-player / multiplayer)  
  - Active gameplay loop  
  - End screen with winner recognition  
- Non-blocking timing using `millis()`  
- Basket input currently simulated prior to hardware integration  

---

### Electronics
- System simulated using selected components  
- Core architecture finalized  
- Progress delayed primarily due to shipping and material costs  

---

## Planned Features

- IR beam-break basket detection  
- 30-second timed gameplay  
- Score and high score display  
- LED feedback system  
- Motorized hoop movement on a linear rail  
- Multiplayer mode  
- Leaderboard system  
- Additional game modes if time permits  

---

## Firmware Status

Current firmware includes:

- Game states:
  - `WAITING`
  - `PLAYING`
  - `GAME_OVER`
- Timer-based gameplay (30 seconds)  
- Score tracking  
- Serial-based basket simulation (temporary)  
- Player mode handling (single-player and multiplayer)  

---

## Hardware / Parts

Current system is built around:

- ESP32 microcontroller  
- IR sensor modules for basket detection  
- Resistor network  
- 5V to 3.3V logic level shifter  
- Audio amplifier module  
- Breadboards and jumper wires (prototype stage)  
- Servo motors for movement  
- Timing belt and linear rail system  
- Plywood and structural wood frame  
- Bolts, nuts, washers, and T-nuts  

---

## Next Steps

### Hardware
- Attach leg frame to base  
- Mount remaining components  
- Complete wiring of electronics  
- Install IR sensors in hoop  

### Firmware
- Replace Serial input with real IR sensor input  
- Implement basket validation logic to prevent double counting  
- Integrate display output  
- Add LED feedback system  
- Implement hoop movement control  

### Additional Features
- Leaderboard system  
- Additional game modes  
- Enhanced multiplayer functionality  

---

## Repository Structure
