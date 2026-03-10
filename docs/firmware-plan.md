# Firmware Plan

## Current Firmware Architecture
The firmware is currently organized as a simple state machine.

### States
- `WAITING`
- `PLAYING`
- `GAME_OVER`

## Current Placeholder Input
Basket detection is currently simulated through Serial input.
Typing `b` in the Serial Monitor triggers a basket event.

This allows firmware logic to be built before real electronics arrive.

## Current Logic
### WAITING
- game is idle
- waits for first basket
- first basket starts the game and sets score to 1

### PLAYING
- timer runs for 30 seconds
- each basket increases score
- time left and score are printed to Serial

### GAME_OVER
- final score is printed
- system waits 5 seconds
- game resets to `WAITING`

## Planned Real Input
Basket detection will eventually use an IR sensor module mounted near the hoop.

Planned sensor behavior:
- beam clear
- beam broken by basketball
- basket event detected once

Future logic should prevent multiple counts from a single shot.

## Planned Future Outputs
- scoreboard display
- LED feedback
- sound output
- motor movement control

## Future Firmware Tasks
1. replace Serial basket simulation with IR sensor input
2. add basket validation logic
3. add display output
4. add LED behavior
5. add audio cues if used
6. add motor control logic

## Notes on Current Parts List
Current electronics likely relevant to firmware:
- ESP32
- IR sensor module
- resistor pack
- 5V to 3.3V logic shifter
- audio amplifier converter
- servo motors

Some display hardware is still not finalized, so display code is still placeholder-only.
