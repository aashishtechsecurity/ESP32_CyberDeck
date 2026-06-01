#include "games.h"
#include "config.h"

struct SnakeSegment {
  int x;
  int y;
};

// Define variables
SnakeSegment snake[60];
int snakeLen = 3;
int dirX = 1;
int dirY = 0;
int foodX = 0;
int foodY = 0;
int score = 0;
bool gameOver = false;

// Local Grid details
const int GRID_SIZE = 4;
const int GRID_WIDTH = SCREEN_WIDTH / GRID_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / GRID_SIZE;

void spawnFood() {
  foodX = random(1, GRID_WIDTH - 2);
  foodY = random(3, GRID_HEIGHT - 2);
}

void initSnakeGame() {
  snakeLen = 3;
  snake[0] = {10, 8};
  snake[1] = {9, 8};
  snake[2] = {8, 8};
  dirX = 1;
  dirY = 0;
  score = 0;
  gameOver = false;
  spawnFood();
}

void updateSnake(JoyDirection input) {
  if (gameOver) return;

  if (input == JOY_UP && dirY == 0) {
    dirX = 0;
    dirY = -1;
  } else if (input == JOY_DOWN && dirY == 0) {
    dirX = 0;
    dirY = 1;
  } else if (input == JOY_LEFT && dirX == 0) {
    dirX = -1;
    dirY = 0;
  } else if (input == JOY_RIGHT && dirX == 0) {
    dirX = 1;
    dirY = 0;
  }

  int nextX = snake[0].x + dirX;
  int nextY = snake[0].y + dirY;

  if (nextX < 0 || nextX >= GRID_WIDTH || nextY < 2 || nextY >= GRID_HEIGHT) {
    gameOver = true;
    setRGBColor(255, 0, 0);
    delay(400);
    setRGBColor(0, 0, 0);
    return;
  }

  for (int i = 0; i < snakeLen; i++) {
    if (snake[i].x == nextX && snake[i].y == nextY) {
      gameOver = true;
      setRGBColor(255, 0, 0);
      delay(400);
      setRGBColor(0, 0, 0);
      return;
    }
  }

  for (int i = snakeLen - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0].x = nextX;
  snake[0].y = nextY;

  if (snake[0].x == foodX && snake[0].y == foodY) {
    score++;
    if (snakeLen < 60) {
      snakeLen++;
    }
    
    setRGBColor(0, 255, 0);
    spawnFood();
    delay(40);
    setRGBColor(0, 0, 0);
  }
}

void drawSnakeGame(Adafruit_SH1106G &display) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 1);
  display.print("SNAKE CORE | SCORE: ");
  display.print(score);
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (gameOver) {
    display.setCursor(34, 25);
    display.println("GAME OVER");
    display.setCursor(18, 40);
    display.println("Click to Restart");
    display.display();
    return;
  }

  display.fillRect(foodX * GRID_SIZE, foodY * GRID_SIZE, GRID_SIZE, GRID_SIZE, SH110X_WHITE);

  for (int i = 0; i < snakeLen; i++) {
    display.fillRect(snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE, SH110X_WHITE);
  }

  display.display();
}

// --- Flappy Bird Implementation ---
bool flappyGameOver = false;
int flappyScore = 0;

float birdY = 32.0;
float birdVelocity = 0.0;
const float FLAPPY_GRAVITY = 0.35;
const float FLAPPY_FLAP = -2.8;

float pillarX = 128.0;
int pillarGapY = 24;
const int PILLAR_GAP_HEIGHT = 20;
const int PILLAR_WIDTH = 8;
const int BIRD_X = 35;

void initFlappyGame() {
  birdY = 32.0;
  birdVelocity = 0.0;
  pillarX = 128.0;
  pillarGapY = random(14, 34);
  flappyScore = 0;
  flappyGameOver = false;
}

void updateFlappyGame(JoyDirection input) {
  if (flappyGameOver) return;

  // Flap on button press or joystick up
  if (input == JOY_CLICK || input == JOY_UP) {
    birdVelocity = FLAPPY_FLAP;
  }

  // Apply physics
  birdVelocity += FLAPPY_GRAVITY;
  birdY += birdVelocity;

  // Move obstacle pillar
  pillarX -= 2.2;
  if (pillarX < -PILLAR_WIDTH) {
    pillarX = 128.0;
    pillarGapY = random(14, 34);
    flappyScore++;
    
    // Flash green on score
    setRGBColor(0, 255, 0);
    delay(40);
    setRGBColor(0, 0, 0);
  }

  // Border collision checks
  if (birdY >= 61.0 || birdY <= 11.0) {
    flappyGameOver = true;
    setRGBColor(255, 0, 0); // Flash RED
    delay(400);
    setRGBColor(0, 0, 0);
    return;
  }

  // Obstacle collision checks
  // Check if bird (BIRD_X, birdY) overlaps horizontal pillar column
  if (BIRD_X + 3 >= (int)pillarX && BIRD_X - 3 <= (int)pillarX + PILLAR_WIDTH) {
    // Check if bird is outside the gap safety window
    if (birdY - 3 < pillarGapY || birdY + 3 > pillarGapY + PILLAR_GAP_HEIGHT) {
      flappyGameOver = true;
      setRGBColor(255, 0, 0);
      delay(400);
      setRGBColor(0, 0, 0);
      return;
    }
  }
}

void drawFlappyGame(Adafruit_SH1106G &display) {
  display.clearDisplay();

  // Header / Score Layout
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(2, 1);
  display.print("FLAPPY BIRD | SCORE: ");
  display.print(flappyScore);
  display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  if (flappyGameOver) {
    display.setCursor(34, 25);
    display.println("GAME OVER");
    display.setCursor(18, 40);
    display.println("Click to Restart");
    display.display();
    return;
  }

  // Draw Pillar Obstacles
  // Top Pillar
  display.fillRect((int)pillarX, 10, PILLAR_WIDTH, pillarGapY - 10, SH110X_WHITE);
  // Bottom Pillar
  display.fillRect((int)pillarX, pillarGapY + PILLAR_GAP_HEIGHT, PILLAR_WIDTH, 64 - (pillarGapY + PILLAR_GAP_HEIGHT), SH110X_WHITE);

  // Draw Flappy Bird (neat circle with a beak)
  display.fillCircle(BIRD_X, (int)birdY, 3, SH110X_WHITE);
  display.drawPixel(BIRD_X + 4, (int)birdY, SH110X_WHITE); // Beak

  display.display();
}
