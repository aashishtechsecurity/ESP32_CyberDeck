#ifndef MATRIX_BOOT_H
#define MATRIX_BOOT_H

#include <Adafruit_SH110X.h>

void initMatrix(Adafruit_SH1106G &display);
void drawMatrixDissolveFrame(Adafruit_SH1106G &display, bool dissolve);
void playBootSequence(Adafruit_SH1106G &display);

#endif // MATRIX_BOOT_H
