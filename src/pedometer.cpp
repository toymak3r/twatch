#include "pedometer.h"
#include "pins.h"
#include "config.h"
#include <Wire.h>
#include <SensorBMA423.hpp>

static SensorBMA423 accel;
static bool ok = false;

bool pedoInit() {
#if PEDOMETER_ENABLED
    ok = accel.init(Wire, I2C_SDA_PIN, I2C_SCL_PIN);
    if (ok) {
        accel.configAccelerometer();
        accel.enableAccelerometer();
        accel.enablePedometer();          // hardware step counter (low power, persists while powered)
        accel.enableFeature(SensorBMA423::FEATURE_STEP_CNTR, true);
    }
    return ok;
#else
    return false;
#endif
}

uint32_t getSteps() { return ok ? accel.getPedometerCounter() : 0; }
