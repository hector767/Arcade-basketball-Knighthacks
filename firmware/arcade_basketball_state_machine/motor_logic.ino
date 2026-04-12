#include <AccelStepper.h>

// HOOP 1 (I think left side)
#define H1_X_STEP 25
#define H1_X_DIR  26
#define H1_Y_LEFT_STEP 27
#define H1_Y_LEFT_DIR  14
// yaxis ones will move in sync
#define H1_Y_RIGHT_STEP 12 
#define H1_Y_RIGHT_DIR  13

// HOOP 2 (I think right side)
#define H2_X_STEP 32
#define H2_X_DIR  33
#define H2_Y_LEFT_STEP 19
#define H2_Y_LEFT_DIR  21
#define H2_Y_RIGHT_STEP 22
#define H2_Y_RIGHT_DIR  23

// --- Safety Limit Switches ---
#define H1_LIMIT_PIN 13 
#define H2_LIMIT_PIN 15

// Initializing all 6 Steppers
AccelStepper h1X(1, H1_X_STEP, H1_X_DIR);
AccelStepper h1YL(1, H1_Y_LEFT_STEP, H1_Y_LEFT_DIR);
AccelStepper h1YR(1, H1_Y_RIGHT_STEP, H1_Y_RIGHT_DIR);

AccelStepper h2X(1, H2_X_STEP, H2_X_DIR);
AccelStepper h2YL(1, H2_Y_LEFT_STEP, H2_Y_LEFT_DIR);
AccelStepper h2YR(1, H2_Y_RIGHT_STEP, H2_Y_RIGHT_DIR);

void initAllMotors() {
  // Set up limit switches as pullups
  pinMode(H1_LIMIT_PIN, INPUT_PULLUP);
  pinMode(H2_LIMIT_PIN, INPUT_PULLUP);

  AccelStepper* motors[] = {&h1X, &h1YL, &h1YR, &h2X, &h2YL, &h2YR};
  for(int i = 0; i < 6; i++) {
    motors[i]->setMaxSpeed(1000);
    motors[i]->setAcceleration(500);
  }
}

// Pass p2Active (p2_joined) to handle both game modes
void updateHoopPhysics(int state, bool p2Active) {
  if (state == 2) { // PLAYING
    
    // Safety: If limit switch is hit, stop motor X immediately
    if (digitalRead(H1_LIMIT_PIN) == LOW) h1X.stop();
    if (digitalRead(H2_LIMIT_PIN) == LOW) h2X.stop();

    // Player 1 X-Axis
    autoBounce(&h1X, 2000);
    
    // Player 1 Y-Axis for both of the motors
    if (h1YL.distanceToGo() == 0) {
      long target = (h1YL.currentPosition() == 0) ? 1000 : 0;
      h1YL.moveTo(target);
      h1YR.moveTo(target); // forcing right to follow left
    }

    // Player 2 logic: Only runs if game mode is 2 players
    if (p2Active) {
      // Player 2 X-Axis
      autoBounce(&h2X, 2000);
      
      // Player 2 Y-Axis, again for both of the motors
      if (h2YL.distanceToGo() == 0) {
        long target = (h2YL.currentPosition() == 0) ? 1000 : 0;
        h2YL.moveTo(target);
        h2YR.moveTo(target);
      }
    }

    // Run all motors (Pulse the 6 drivers)
    h1X.run(); h1YL.run(); h1YR.run();
    h2X.run(); h2YL.run(); h2YR.run();
    
  } else {
    // Stop all 6 motors if state is not PLAYING (WAITING, RESULTS, etc)
    h1X.stop(); h1YL.stop(); h1YR.stop();
    h2X.stop(); h2YL.stop(); h2YR.stop();
  }
}

void autoBounce(AccelStepper* m, int dist) {
  if (m->distanceToGo() == 0) {
    m->moveTo(m->currentPosition() == 0 ? dist : 0);
  }
}