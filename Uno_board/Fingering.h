#ifndef FINGERING_H
#define FINGERING_H

#include "JobQueue.h"
#include "Slider.h"
#include "RackMotor.h"

void moveFinger(Slider &slider, RackMotor &rackMotor, int distanceMm);

#endif // FINGERING_H