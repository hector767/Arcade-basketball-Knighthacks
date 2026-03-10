enum GameState { WAITING, PLAYING, GAME_OVER };

GameState state = WAITING;

const unsigned long GAME_DURATION_MS = 30000;
const unsigned long POST_GAME_MS = 5000;

unsigned long gameStartTime = 0;
unsigned long gameOverTime = 0;
unsigned long lastPrintTime = 0;

int score = 0;
bool basketDetected = false;

void readInputs() {
  basketDetected = false;

  // Placeholder basket input through Serial Monitor
  if (Serial.available() > 0) {
    char input = Serial.read();

    if (input == 'b' || input == 'B') {
      basketDetected = true;
    }
  }
}

void startGame(unsigned long now) {
  state = PLAYING;
  gameStartTime = now;
  lastPrintTime = now;
  score = 1;

  Serial.println("STATE: PLAYING");
  Serial.print("Time left: ");
  Serial.print(GAME_DURATION_MS / 1000);
  Serial.print("  Score: ");
  Serial.println(score);
}

void handlePlayingState(unsigned long now) {
  unsigned long elapsed = now - gameStartTime;

  if (basketDetected) {
    score++;
    Serial.print("Score: ");
    Serial.println(score);
  }

  if (now - lastPrintTime >= 1000) {
    lastPrintTime = now;

    unsigned long timeLeft = 0;
    if (elapsed < GAME_DURATION_MS) {
      timeLeft = (GAME_DURATION_MS - elapsed) / 1000;
    }

    Serial.print("Time left: ");
    Serial.print(timeLeft);
    Serial.print("  Score: ");
    Serial.println(score);
  }

  if (elapsed >= GAME_DURATION_MS) {
    state = GAME_OVER;
    gameOverTime = now;

    Serial.print("STATE: GAME_OVER  Final score: ");
    Serial.println(score);
  }
}

void handleGameOverState(unsigned long now) {
  if (now - gameOverTime >= POST_GAME_MS) {
    state = WAITING;
    score = 0;
    basketDetected = false;

    Serial.println("STATE: WAITING");
    Serial.println("Type 'b' in Serial Monitor to start again.");
  }
}

void updateGame(unsigned long now) {
  switch (state) {
    case WAITING:
      if (basketDetected) {
        startGame(now);
      }
      break;

    case PLAYING:
      handlePlayingState(now);
      break;

    case GAME_OVER:
      handleGameOverState(now);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting arcade basketball...");
  Serial.println("Type 'b' in Serial Monitor to simulate a basket.");
}

void loop() {
  unsigned long now = millis();

  readInputs();
  updateGame(now);
}