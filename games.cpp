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
