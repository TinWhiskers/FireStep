#ifndef MACHINE_H
#define MACHINE_H

#include "Stroke.h"
#include "Display.h"
#include "pins.h"

extern void test_Home();

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
#define MICROSTEPS_DEFAULT 16

#ifndef DELAY500NS
#define DELAY500NS \
  asm("nop");asm("nop");asm("nop");asm("nop"); asm("nop");asm("nop");asm("nop");asm("nop");
#endif

typedef class Axis {
        friend void ::test_Home();
        friend class Machine;
    private:
        bool		enabled; // true: stepper drivers are enabled and powered
    public:
        PinType 	pinStep; // step pin
        PinType 	pinDir;	// step direction pin
        PinType 	pinMin; // homing minimum limit switch
        PinType 	pinMax;	// maximum limit switch (optional)
        PinType 	pinEnable; // stepper driver enable pin (nENBL)
        StepCoord	home; // home position
        StepCoord 	travelMin; // soft minimum travel limit
        StepCoord 	travelMax; // soft maximum travel limit
        StepCoord 	searchVelocity; // homing velocity (pulses/second)
        StepCoord 	position; // current position (pulses)
        StepCoord 	latchBackoff; // pulses to send for backing off limit switch
        int16_t		usDelay; // minimum time between stepper pulses
        float		stepAngle; // 1.8:200 steps/rev; 0.9:400 steps/rev
        uint8_t		microsteps;	// normally 1,2,4,8,16 or 32
        bool		dirHIGH; // advance on HIGH
        uint8_t 	powerManagementMode;
        bool		atMin; // minimum limit switch (last value read)
        bool		atMax; // maximum limit switch (last value read)
        bool		homing; // true:axis is active for homing 

        Axis() :
            pinStep(NOPIN),
            pinDir(NOPIN),
            pinMin(NOPIN),
            pinMax(NOPIN),
            pinEnable(NOPIN),
            home(0),
            travelMin(0),
            travelMax(10000),
            searchVelocity(200),
            position(0),
            latchBackoff(MICROSTEPS_DEFAULT), 
            usDelay(0), // Suggest 80us (12.8kHz) for microsteps 1
            stepAngle(1.8),
            microsteps(MICROSTEPS_DEFAULT),
            dirHIGH(true), // true:advance on HIGH; false:advance on LOW
            powerManagementMode(0),	// 0:off, 1:on, 2:on in cycle, 3:on when moving
            atMin(false),
            atMax(false),
            enabled(false),
            homing(false)
        {};
        Status enable(bool active);
		bool isEnabled() { return enabled; }
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
typedef int8_t AxisIndex;
typedef QuadIndex MotorIndex;

typedef class Machine : public QuadStepper {
        friend void ::test_Home();
    private:
        bool	pinEnableHigh;
        Display nullDisplay;
        int8_t 	stepHome();
		Axis *	motorAxis[MOTOR_COUNT];
        AxisIndex motor[MOTOR_COUNT];
    public:
        bool	invertLim;
        Display	*pDisplay;
        Axis axis[AXIS_COUNT];
        Stroke stroke;

    public:
        Machine();
        void enable(bool active);
        virtual Status step(const Quad<StepCoord> &pulse);
        void setPin(PinType &pinDst, PinType pinSrc, int16_t mode, int16_t value = LOW);
        Quad<StepCoord> getMotorPosition();
        void setMotorPosition(const Quad<StepCoord> &position);
        virtual Status home();
		Status setMotorAxis(MotorIndex iMotor, AxisIndex iAxis);
		AxisIndex getMotorAxis(MotorIndex iMotor) { return motor[iMotor]; }
} Machine;

} // namespace firestep

#endif
