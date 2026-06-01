#ifndef GAMES_H
#define GAMES_H

#include <Adafruit_SH110X.h>
#include "hardware.h"

extern bool gameOver;
extern int score;

// Flappy Bird Declarations
extern bool flappyGameOver;
extern int flappyScore;

void initSnakeGame();
void updateSnake(JoyDirection input);
void drawSnakeGame(Adafruit_SH1106G &display);

void initFlappyGame();
void updateFlappyGame(JoyDirection input);
void drawFlappyGame(Adafruit_SH1106G &display);

#endif // GAMES_H
