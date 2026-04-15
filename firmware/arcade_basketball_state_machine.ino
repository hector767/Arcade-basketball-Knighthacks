#include <Wire.h>
#include "Adafruit_VL53L0X.h"
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <motor_logic.ino>
#include <sensor_code.ino>
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int NAV_PREV_PIN = 18;  // Small Button Left
const int NAV_NEXT_PIN = 19;  // Small Button Right
const int P1_READY_PIN = 25;  // Big Button P1
const int P2_READY_PIN = 26;  // Big Button P2
const int P1_SCORE_PIN = 32;  // TOF Sensor P1
const int P2_SCORE_PIN = 33;  // TOF Sensor P2

const unsigned long GAME_DURATION_MS = 30000;
const unsigned long DEBOUNCE_MS = 60;

enum GameState
{
  WAITING_FOR_PLAYER,
  PLAYER_SELECT,
  PLAYING,
  RESULTS
};

GameState state = WAITING_FOR_PLAYER;

bool p1_joined = false;
bool p2_joined = false;

int p1_score = 0;
int p2_score = 0;

unsigned long game_start_time = 0;

const int NUM_MODES = 4;
String gameModes[NUM_MODES] = {
  "1v1 Classic",
  "1v1 Moving Hoops",
  "Solo Challenge",
  "Solo Moving Hoops"
};
int currentModeIndex = 0;

struct Button {
  int pin;
  bool lastReading;
  bool stableState;
  unsigned long lastTime;
};

Button navPrev = { NAV_PREV_PIN, HIGH, HIGH, 0 };
Button navNext = { NAV_NEXT_PIN, HIGH, HIGH, 0 };
Button p1Ready = { P1_READY_PIN, HIGH, HIGH, 0 };
Button p2Ready = { P2_READY_PIN, HIGH, HIGH, 0 };
Button p1Score = { P1_SCORE_PIN, HIGH, HIGH, 0 };
Button p2Score = { P2_SCORE_PIN, HIGH, HIGH, 0 };

// --- FUNCTION PROTOTYPES ---
bool isPressed(Button& b);
void reset_game();
void start_game();
void show_waiting_screen();
void show_player_select_screen();
void show_playing_screen();
void show_results_screen();


// setup(); Initializes hardware (LCD, I2C, pins, Serial) and sets the system to a clean starting state with the waiting screen displayed.
void setup()
{
  Wire.begin();
  lcd.init();
  lcd.backlight();

  pinMode(NAV_PREV_PIN, INPUT_PULLUP);
  pinMode(NAV_NEXT_PIN, INPUT_PULLUP);
  pinMode(P1_READY_PIN, INPUT_PULLUP);
  pinMode(P2_READY_PIN, INPUT_PULLUP);
  pinMode(P1_SCORE_PIN, INPUT_PULLUP);
  pinMode(P2_SCORE_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  reset_game();
  show_waiting_screen();
  
  initSensors(); // Initialize sensors
  // initializing the 6 motors and giving them default values for both the hoops
  initAllMotors();
}


// loop(); Continuously checks for button input and routes behavior based on the current game state, acting as the main control flow of the program.
void loop()
{
  loopSensors(); 
  // Check Navigation Buttons (Small Buttons)
  if (isPressed(navPrev)) {
    if (state == WAITING_FOR_PLAYER) {
      currentModeIndex = (currentModeIndex - 1 + NUM_MODES) % NUM_MODES;
      show_waiting_screen();
    }
  }

  if (isPressed(navNext)) {
    if (state == WAITING_FOR_PLAYER) {
      currentModeIndex = (currentModeIndex + 1) % NUM_MODES;
      show_waiting_screen();
    }
    else if (state == PLAYER_SELECT) {
      p2_joined = !p2_joined;
      show_player_select_screen();
    }
  }

  // Check Action Buttons (Big Buttons)
  if (isPressed(p1Ready)) {
    if (state == WAITING_FOR_PLAYER) {
      state = PLAYER_SELECT;
      p1_joined = true;
      show_player_select_screen();
    }
    else if (state == PLAYER_SELECT) {
      start_game();
    }
    else if (state == RESULTS) {
      reset_game();
      show_waiting_screen();
    }
  }

  if (isPressed(p2Ready)) {
    if (state == RESULTS) {
      reset_game();
      show_waiting_screen();
    }
  }
  // Handle game logic if in PLAYING state
  if (state == PLAYING) {
    handle_playing_state();
  }
}


// handle_playing_state(); Runs the active game logic: updates scores, refreshes the display, tracks elapsed time, and transitions to results when time expires.
void handle_playing_state()
{
  unsigned long now = millis();
  unsigned long elapsed = now - game_start_time;

  // added this function call to make the motors move depending on how many players are playing
  updateHoopPhysics(state, p2_joined);

  if (isPressed(p1Score)) {
    p1_score++;
    show_playing_screen();
  }

  if (p2_joined && isPressed(p2Score)) {
    p2_score++;
    show_playing_screen();
  }

  static unsigned long last_refresh = 0;
  if (now - last_refresh >= 500)
  {
    last_refresh = now;
    show_playing_screen();
  }

  if (elapsed >= GAME_DURATION_MS)
  {
    state = RESULTS;
    show_results_screen();
  }
}


// start_game(); Resets scores, records the start time, switches the state to PLAYING, and updates the display to begin a new game.
void start_game()
{
  p1_score = 0;
  p2_score = 0;
  game_start_time = millis();
  state = PLAYING;
  show_playing_screen();

  Serial.println("GAME STARTED");
}


// reset_game(); Clears all player data, scores, and timers, returning the system to the initial WAITING state.
void reset_game()
{
  p1_joined = false;
  p2_joined = false;
  state = WAITING_FOR_PLAYER;
}


// show_waiting_screen(); Displays the idle screen prompting the user to press the button to begin.
void show_waiting_screen()
{
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("BASKETBALL ARCADE");
  lcd.setCursor(0, 1);
  lcd.print("Mode: ");
  lcd.print(gameModes[currentModeIndex]);
  lcd.setCursor(0, 2);
  lcd.print("Use Sm. Btns to Nav"); //TODO: better instructions
  lcd.setCursor(0, 3);
  lcd.print("Press BIG BTN to Play");
}


// show_player_select_screen(); Displays player selection info and reflects whether Player 2 is active, guiding the user to start the game.
void show_player_select_screen()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("PLAYER SELECT");
  lcd.setCursor(0, 1);
  lcd.print("P1: READY");
  lcd.setCursor(0, 2);
  lcd.print("P2: ");
  lcd.print(p2_joined ? "JOINED" : "PRESS RIGHT SM BTN");
  lcd.setCursor(0, 3);
  lcd.print("BIG BTN = START!");
}


// show_playing_screen(); Displays live game information including time remaining and current scores for each player.
void show_playing_screen()
{
  lcd.clear();

  unsigned long elapsed = millis() - game_start_time;
  int time_left = (GAME_DURATION_MS - elapsed) / 1000;
  if (time_left < 0) time_left = 0;


  lcd.setCursor(0, 0);
  lcd.print("MODE: "); lcd.print(gameModes[currentModeIndex]);
  lcd.setCursor(0, 1);
  lcd.print("TIME LEFT: "); lcd.print(time_left); lcd.print("s");

  lcd.setCursor(0, 2);
  lcd.print("P1 SCORE: "); lcd.print(p1_score);

  if (p2_joined) {
    lcd.setCursor(0, 3);
    lcd.print("P2 SCORE: "); lcd.print(p2_score);
  }
}


// show_results_screen(); Displays final scores and determines the outcome (solo, winner, or tie), along with reset instructions.
void show_results_screen()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("FINAL SCORE");
  lcd.setCursor(0, 1);
  lcd.print("P1: "); lcd.print(p1_score);
  if (p2_joined) {
    lcd.print(" vs P2: "); lcd.print(p2_score);
  }

  lcd.setCursor(0, 2);
  if (p2_joined) {
    if (p1_score > p2_score) lcd.print("WINNER: PLAYER 1!");
    else if (p2_score > p1_score) lcd.print("WINNER: PLAYER 2!");
    else lcd.print("IT'S A TIE!");
  }
  else {
    lcd.print("GREAT JOB!");
  }

  lcd.setCursor(0, 3);
  lcd.print("BIG BTN TO RESET");
}



bool isPressed(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastTime = millis();
  }
  if ((millis() - b.lastTime) > DEBOUNCE_MS) {
    if (reading != b.stableState) {
      b.stableState = reading;
      if (b.stableState == LOW) {
        b.lastReading = reading;
        return true;
      }
    }
  }
  b.lastReading = reading;
  return false;
}
