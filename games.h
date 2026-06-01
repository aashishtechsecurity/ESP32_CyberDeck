#ifndef GAMES_H
#define GAMES_H

#include <Adafruit_SH110X.h>
#include "hardware.h"

extern bool gameOver;
extern int score;

void initSnakeGame();
void updateSnake(JoyDirection input);
void drawSnakeGame(Adafruit_SH1106G &display);

#endif // GAMES_H
