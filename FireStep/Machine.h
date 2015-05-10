#ifndef MACHINE_H
#define MACHINE_H

#include "Stroke.h"
#include "Display.h"
#include "pins.h"

namespace firestep {

// #define THROTTLE_SPEED /* Throttles driver speed from high (255) to low (0) */

#define A4988_PULSE_DELAY 	DELAY500NS;DELAY500NS
#define DRV8825_PULSE_DELAY DELAY500NS;DELAY500NS;DELAY500NS;DELAY500NS
#define STEPPER_PULSE_DELAY DRV8825_PULSE_DELAY
#define DELTA_COUNT 120
#define MOTOR_COUNT 4
#define AXIS_COUNT 6
#define PIN_ENABLE LOW
#define PIN_DISABLE HIGH

#ifndef DELAY500NS
#define DELAY500NS \
  asm("nop");asm("nop");asm("nop");asm("nop"); asm("nop");asm("nop");asm("nop");asm("nop");
#endif

typedef struct Motor {
    uint8_t	axisMap; 	// index into axis array
    Motor() : axisMap(0) {}
} Motor;

typedef class Axis {
    public:
        PinType 	pinStep; // step pin
        PinType 	pinDir;	// step direction pin
        PinType 	pinMin; // homing minimum limit switch
        PinType 	pinMax;	// maximum limit switch (optional)
        PinType 	pinEnable; // stepper driver enable pin (nENBL)
        StepCoord 	travelMin; // soft minimum travel limit
        StepCoord 	travelMax; // soft maximum travel limit
        StepCoord 	searchVelocity; // homing velocity (pulses/second)
        StepCoord 	position; // current position (pulses)
		int16_t		usDelay; // minimum time between stepper pulses
        float		stepAngle;
        uint8_t		microsteps;
        bool		invertDir;
        uint8_t 	powerManagementMode;
        bool		atMin;
        bool		atMax;
        bool		enabled;
        Axis() :
            pinStep(NOPIN),
            pinDir(NOPIN),
            pinMin(NOPIN),
            pinMax(NOPIN),
            pinEnable(NOPIN),
            travelMin(0),
            travelMax(10000),
            searchVelocity(200),
            position(0),
			usDelay(80), // 12.8kHz conservative 
            stepAngle(1.8),
            microsteps(16),
            invertDir(0),				// 0:normal direction, 1:inverted direction
            powerManagementMode(0),	// 0:off, 1:on, 2:on in cycle, 3:on when moving
            atMin(false),
            atMax(false),
            enabled(false)
        {};
        Status enable(bool active);
        inline Status pinMode(PinType pin, int mode) {
            if (pin == NOPIN) {
                return STATUS_NOPIN;
            }
            ::pinMode(pin, mode);
            return STATUS_OK;
        }
        inline Status readAtMin(bool invertLim) {
            if (pinMin == NOPIN) {
                return STATUS_NOPIN;
            }
            bool minHigh = digitalRead(pinMin);
            bool atMinNew = (invertLim == !minHigh);
            if (atMinNew != atMin) {
                atMin = atMinNew;
            }
			return STATUS_OK;
        }
        inline Status readAtMax(bool invertLim) {
            if (pinMax == NOPIN) {
				return STATUS_NOPIN;
			}
			bool maxHigh = digitalRead(pinMax);
			bool atMaxNew = (invertLim == !maxHigh);
			if (atMaxNew != atMax) {
				atMax = atMaxNew;
            }
			return STATUS_OK;
        }
} Axis;

typedef class Machine : public QuadStepper {
        friend class JsonController;
    private:
        bool	invertLim;
        bool	pinEnableHigh;
        Display nullDisplay;
    public:
        Display	*pDisplay;
        Motor motor[MOTOR_COUNT];
        Axis axis[AXIS_COUNT];
        Stroke stroke;

    public:
        Machine();
        void setup();
        virtual Status step(const Quad<StepCoord> &pulse);
		void setPin(PinType &pinDst, PinType pinSrc, int16_t mode, int16_t value=LOW);
        Quad<StepCoord> getMotorPosition();
		void setMotorPosition(const Quad<StepCoord> &position);
} Machine;

} // namespace firestep

#endif