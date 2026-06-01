#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Adafruit_SH110X.h>

void initDashboard();
void updateDashboardData();
void drawDashboard(Adafruit_SH1106G &display);

#endif // DASHBOARD_H
