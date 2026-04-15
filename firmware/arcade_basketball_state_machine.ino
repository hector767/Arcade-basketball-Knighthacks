#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <motor_logic.ino>
#include <sensor_code.ino>
#include <Adafruit_VL53L0X.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_VL53L0X tof1;
Adafruit_VL53L0X tof2;

const int NAV_PREV_PIN = 18;  // Small Nav Button Left
const int NAV_NEXT_PIN = 19;  // Small Nav Button Right
const int P1_BUTTON_PIN = 34; // Main P1 Button (Start/Reset)
const int P2_BUTTON_PIN = 35; // Main P2 Button (Toggle P2)
const int P1_XSHUT_PIN = 4;
const int P2_XSHUT_PIN = 5;

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
int high_score = 0;
unsigned long game_start_time = 0;
unsigned long last_p1_score_ms = 0;
unsigned long last_p2_score_ms = 0;

const unsigned long SCORE_COOLDOWN_MS = 400;
const int P1_SCORE_THRESHOLD_MM = 200;
const int P2_SCORE_THRESHOLD_MM = 200;

const int NUM_MODES = 4;
String gameModes[NUM_MODES] = {
  "1v1 Classic",
  "1v1 Moving Hoops",
  "Solo Challenge",
  "Solo Moving Hoops"
};
int currentModeIndex = 0;

bool last_p1_btn_read = HIGH, last_p2_btn_read = HIGH;
bool last_prev_btn_read = HIGH, last_next_btn_read = HIGH;
bool p1_btn_state = HIGH, p2_btn_state = HIGH;
bool prev_btn_state = HIGH, next_btn_state = HIGH;
unsigned long p1_btn_time = 0, p2_btn_time = 0;
unsigned long prev_btn_time = 0, next_btn_time = 0;

// --- FUNCTION PROTOTYPES ---
bool check_press(int pin, bool& last_reading, bool& stable_state, unsigned long& last_time);
void updateScreen();

// setup(); Initializes hardware (LCD, I2C, pins, Serial) and sets the system to a clean starting state with the waiting screen displayed.
void setup()
{
  Wire.begin();
  lcd.init();
  lcd.backlight();

  pinMode(P1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(P2_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NAV_PREV_PIN, INPUT_PULLUP);
  pinMode(NAV_NEXT_PIN, INPUT_PULLUP);

  Serial.begin(115200);



  pinMode(P1_XSHUT_PIN, OUTPUT);
  pinMode(P2_XSHUT_PIN, OUTPUT);
  digitalWrite(P1_XSHUT_PIN, LOW);
  digitalWrite(P2_XSHUT_PIN, LOW);
  delay(10);

  digitalWrite(P1_XSHUT_PIN, HIGH);
  delay(10);
  if (!tof1.begin(0x30)) Serial.println("Failed P1 ToF");

  digitalWrite(P2_XSHUT_PIN, HIGH);
  delay(10);
  if (!tof2.begin(0x31)) Serial.println("Failed P2 ToF");

  reset_game();
  show_waiting_screen();

  initSensors();
  initAllMotors();
}


// loop(); Continuously checks for button input and routes behavior based on the current game state, acting as the main control flow of the program.
void loop()
{
  loopSensors(); 
  // Check Navigation Buttons (Small Buttons)
  if (check_press(NAV_PREV_PIN, last_prev_btn_read, prev_btn_state, prev_btn_time)) {
    if (state == WAITING_FOR_PLAYER) {
      currentModeIndex = (currentModeIndex - 1 + NUM_MODES) % NUM_MODES;
      show_waiting_screen();
    }
  }

  if (check_press(NAV_NEXT_PIN, last_next_btn_read, next_btn_state, next_btn_time)) {
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
  if (check_press(P1_BUTTON_PIN, last_p1_btn_read, p1_btn_state, p1_btn_time)) {
    if (state == WAITING_FOR_PLAYER) {
      p1_joined = true;
      p2_joined = false;
      state = PLAYER_SELECT;
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

  if (check_press(P2_BUTTON_PIN, last_p2_btn_read, p2_btn_state, p2_btn_time)) {
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

  if (p1_scored_from_tof() && now - last_p1_score_ms > SCORE_COOLDOWN_MS) {
    p1_score++;
    last_p1_score_ms = now;
    show_playing_screen();
  }

  if (p2_joined && p2_scored_from_tof() && now - last_p2_score_ms > SCORE_COOLDOWN_MS) {
    p2_score++;
    last_p2_score_ms = now;
    show_playing_screen();
  }

  static unsigned long last_refresh = 0;
  if (now - last_refresh >= 250) {
    last_refresh = now;
    show_playing_screen();
  }

  if (elapsed >= GAME_DURATION_MS) {
    if (p1_score > high_score) high_score = p1_score;
    if (p2_score > high_score) high_score = p2_score;
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
  state = WAITING_FOR_PLAYER;
}


// show_waiting_screen(); Displays the idle screen prompting the user to press the button to begin.
void show_waiting_screen()
{
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Arcade Basketball");
  lcd.setCursor(0, 1); lcd.print("Mode: "); lcd.print(gameModes[currentModeIndex]);
  lcd.setCursor(0, 2); lcd.print("Press P1 to start");
  lcd.setCursor(0, 3); lcd.print("Daily High: "); lcd.print(high_score);
}


// show_player_select_screen(); Displays player selection info and reflects whether Player 2 is active, guiding the user to start the game.
void show_player_select_screen()
{
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Select Players");
  lcd.setCursor(0, 1); lcd.print("P1: READY");
  lcd.setCursor(11, 1); lcd.print("P2:"); lcd.print(p2_joined ? "ON " : "OFF");
  lcd.setCursor(0, 2); lcd.print("Right Nav = P2 On");
  lcd.setCursor(0, 3); lcd.print("P1 Button = Start");
}


// show_playing_screen(); Displays live game information including time remaining and current scores for each player.
void show_playing_screen()
{
  lcd.clear();
  unsigned long elapsed = millis() - game_start_time;
  int time_left = (GAME_DURATION_MS - elapsed) / 1000;
  if (time_left < 0) time_left = 0;

  lcd.setCursor(0, 0); lcd.print("Time: "); lcd.print(time_left); lcd.print("s");
  lcd.setCursor(11, 0); lcd.print(gameModes[currentModeIndex].substring(0, 9));

  lcd.setCursor(0, 1); lcd.print("P1: "); lcd.print(p1_score);
  lcd.setCursor(10, 1); lcd.print("P2: ");
  if (p2_joined) lcd.print(p2_score); else lcd.print("--");

  lcd.setCursor(0, 3); lcd.print("Shoot!");
}


// show_results_screen(); Displays final scores and determines the outcome (solo, winner, or tie), along with reset instructions.
void show_results_screen()
{
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Final Results");
  lcd.setCursor(0, 1); lcd.print("P1: "); lcd.print(p1_score);
  lcd.setCursor(10, 1); lcd.print("P2: "); lcd.print(p2_joined ? String(p2_score) : "--");

  lcd.setCursor(0, 2);
  if (!p2_joined) lcd.print("Solo game done");
  else if (p1_score > p2_score) lcd.print("Winner: P1");
  else if (p2_score > p1_score) lcd.print("Winner: P2");
  else lcd.print("Tie game");

  lcd.setCursor(0, 3); lcd.print("Press btn to reset");
}

bool check_press(int pin, bool &last_reading, bool &stable_state, unsigned long &last_time) {
  bool reading = digitalRead(pin);
  if (reading != last_reading) last_time = millis();
  if ((millis() - last_time) > DEBOUNCE_MS) {
    if (reading != stable_state) {
      stable_state = reading;
      if (stable_state == LOW) { last_reading = reading; return true; }
    }
  }
  last_reading = reading;
  return false;
}

bool p1_scored_from_tof() {
  VL53L0X_RangingMeasurementData_t measure;
  tof1.rangingTest(&measure, false);
  return (measure.RangeStatus != 4 && measure.RangeMilliMeter < P1_SCORE_THRESHOLD_MM);
}

bool p2_scored_from_tof() {
  VL53L0X_RangingMeasurementData_t measure;
  tof2.rangingTest(&measure, false);
  return (measure.RangeStatus != 4 && measure.RangeMilliMeter < P2_SCORE_THRESHOLD_MM);
}
