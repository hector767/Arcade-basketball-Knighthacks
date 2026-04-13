#include "Adafruit_VL53L0X.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <motor_logic.ino>
#include <sensor_code.ino>
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int P1_BUTTON_PIN = 18;
const int P2_BUTTON_PIN = 19;

const int P1_SCORE_PIN = 32;
const int P2_SCORE_PIN = 33;

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

bool last_p1_button_reading = HIGH;
bool last_p2_button_reading = HIGH;
bool last_p1_score_reading = HIGH;
bool last_p2_score_reading = HIGH;

bool p1_button_state = HIGH;
bool p2_button_state = HIGH;
bool p1_score_state = HIGH;
bool p2_score_state = HIGH;

unsigned long last_p1_button_time = 0;
unsigned long last_p2_button_time = 0;
unsigned long last_p1_score_time = 0;
unsigned long last_p2_score_time = 0;

bool p1_button_pressed();
bool p2_button_pressed();
bool p1_score_detected();
bool p2_score_detected();

bool check_press(int pin,
                 bool &last_reading,
                 bool &stable_state,
                 unsigned long &last_time);

void reset_game();
void start_game();

void show_waiting_screen();
void show_player_select_screen();
void show_playing_screen();
void show_results_screen();

void handle_waiting_state();
void handle_player_select_state();
void handle_playing_state();
void handle_results_state();


// setup(); Initializes hardware (LCD, I2C, pins, Serial) and sets the system to a clean starting state with the waiting screen displayed.
void setup()
{
  Wire.begin();
  lcd.init();
  lcd.backlight();

  pinMode(P1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(P2_BUTTON_PIN, INPUT_PULLUP);

  pinMode(P1_SCORE_PIN, INPUT_PULLUP);
  pinMode(P2_SCORE_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  reset_game();
  show_waiting_screen();

  initSensors(); // Initialize sensors

  // initializing the 6 motors and giving it default values for both the hoops
  initAllMotors();
}


// loop(); Continuously checks for button input and routes behavior based on the current game state, acting as the main control flow of the program.
void loop()
{
  loopSensors(); 
  if (p1_button_pressed())
  {
    Serial.println("P1 BUTTON");
    
    if (state == WAITING_FOR_PLAYER)
    {
      p1_joined = true;
      p2_joined = false;
      state = PLAYER_SELECT;
      show_player_select_screen();
    }
    else if (state == PLAYER_SELECT)
    {
      start_game();
    }
    else if (state == RESULTS)
    {
      reset_game();
      show_waiting_screen();
    }
  }

  if (p2_button_pressed())
  {
    Serial.println("P2 BUTTON");

    if (state == PLAYER_SELECT)
    {
      p2_joined = !p2_joined;
      show_player_select_screen();
    }
    else if (state == RESULTS)
    {
      reset_game();
      show_waiting_screen();
    }
  }

  switch (state)
  {
    case WAITING_FOR_PLAYER:
      break;

    case PLAYER_SELECT:
      break;

    case PLAYING:
      handle_playing_state();
      break;

    case RESULTS:
      break;
  }
}


// handle_playing_state(); Runs the active game logic: updates scores, refreshes the display, tracks elapsed time, and transitions to results when time expires.
void handle_playing_state()
{
  unsigned long now = millis();
  unsigned long elapsed = now - game_start_time;

  // added this function call to make the motors move depending on how many players are playing
  updateHoopPhysics(state, p2_joined);

  if (p1_score_detected())
  {
    p1_score++;
    Serial.print("P1 SCORE: ");
    Serial.println(p1_score);
    show_playing_screen();
  }

  if (p2_joined && p2_score_detected())
  {
    p2_score++;
    Serial.print("P2 SCORE: ");
    Serial.println(p2_score);
    show_playing_screen();
  }

  static unsigned long last_refresh = 0;
  if (now - last_refresh >= 250)
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
  p1_score = 0;
  p2_score = 0;
  game_start_time = 0;
  state = WAITING_FOR_PLAYER;
}


// show_waiting_screen(); Displays the idle screen prompting the user to press the button to begin.
void show_waiting_screen()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Arcade Basketball");

  lcd.setCursor(0, 1);
  lcd.print("Left btn to play");

  lcd.setCursor(0, 2);
  lcd.print("Waiting...");
}


// show_player_select_screen(); Displays player selection info and reflects whether Player 2 is active, guiding the user to start the game.
void show_player_select_screen()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Select Players");

  lcd.setCursor(0, 1);
  lcd.print("P1: READY");

  lcd.setCursor(11, 1);
  lcd.print("P2:");
  lcd.print(p2_joined ? "ON " : "OFF");

  lcd.setCursor(0, 2);
  lcd.print("Right btn = P2");

  lcd.setCursor(0, 3);
  lcd.print("Left btn = start");
}


// show_playing_screen(); Displays live game information including time remaining and current scores for each player.
void show_playing_screen()
{
  lcd.clear();

  unsigned long elapsed = millis() - game_start_time;
  unsigned long time_left = 0;

  if (elapsed < GAME_DURATION_MS)
  {
    time_left = (GAME_DURATION_MS - elapsed) / 1000;
  }

  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(time_left);
  lcd.print("s   ");

  lcd.setCursor(0, 1);
  lcd.print("P1: ");
  lcd.print(p1_score);
  lcd.print("   ");

  lcd.setCursor(10, 1);
  lcd.print("P2: ");
  if (p2_joined)
  {
    lcd.print(p2_score);
    lcd.print("   ");
  }
  else
  {
    lcd.print("-- ");
  }

  lcd.setCursor(0, 3);
  lcd.print("Shoot!");
}


// show_results_screen(); Displays final scores and determines the outcome (solo, winner, or tie), along with reset instructions.
void show_results_screen()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Final Results");

  lcd.setCursor(0, 1);
  lcd.print("P1: ");
  lcd.print(p1_score);

  lcd.setCursor(10, 1);
  lcd.print("P2: ");
  if (p2_joined)
  {
    lcd.print(p2_score);
  }
  else
  {
    lcd.print("--");
  }

  lcd.setCursor(0, 2);
  if (!p2_joined)
  {
    lcd.print("Solo game done");
  }
  else if (p1_score > p2_score)
  {
    lcd.print("Winner: P1");
  }
  else if (p2_score > p1_score)
  {
    lcd.print("Winner: P2");
  }
  else
  {
    lcd.print("Tie game");
  }

  lcd.setCursor(0, 3);
  lcd.print("Press btn to reset");
}


// check_press(); Handles button debouncing and returns true only when a clean, new press is detected.
bool check_press(int pin,
                 bool &last_reading,
                 bool &stable_state,
                 unsigned long &last_time)
{
  bool reading = digitalRead(pin);

  if (reading != last_reading)
  {
    last_time = millis();
  }

  if ((millis() - last_time) > DEBOUNCE_MS)
  {
    if (reading != stable_state)
    {
      stable_state = reading;

      if (stable_state == LOW)
      {
        last_reading = reading;
        return true;
      }
    }
  }

  last_reading = reading;
  return false;
}


// p1_button_pressed(); Checks for a debounced press of Player 1’s button using shared debounce logic.
bool p1_button_pressed()
{
  return check_press(P1_BUTTON_PIN,
                     last_p1_button_reading,
                     p1_button_state,
                     last_p1_button_time);
}


// p2_button_pressed(); Checks for a debounced press of Player 2’s button using shared debounce logic.
bool p2_button_pressed()
{
  return check_press(P2_BUTTON_PIN,
                     last_p2_button_reading,
                     p2_button_state,
                     last_p2_button_time);
}


// p1_score_detected(); Detects a scoring event for Player 1 using the debounced input mechanism.
bool p1_score_detected()
{
  return check_press(P1_SCORE_PIN,
                     last_p1_score_reading,
                     p1_score_state,
                     last_p1_score_time);
}


// p2_score_detected(); Detects a scoring event for Player 2 using the debounced input mechanism.
bool p2_score_detected()
{
  return check_press(P2_SCORE_PIN,
                     last_p2_score_reading,
                     p2_score_state,
                     last_p2_score_time);
}