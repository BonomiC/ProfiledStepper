#ifndef PROFILEDSTEPPER_H
#define PROFILEDSTEPPER_H

#include "Arduino.h"
#include "math.h"

class ProfiledStepper {
public:
	ProfiledStepper(int ena_low, int ena_high, int dir_pin, int dir_power, int step_pin, int step_power, int steps_per_rev, int min_speed, int top_speed, String accel_type, int accel_length, bool disable);

	void step(int distance);
	void stepByRevolution(float revs);

private:
	int ena_low, dir_pin, step_pin, steps_per_rev, min_speed, top_speed, accel_length;
	float accel_coefficient;
	float a, accel_b, deccel_b, c, d;

	long current_position;
	bool linear_acceleration, disable_motor_on_step;

	void setPins(int ena_low, int ena_high, int dir_pin, int dir_power, int step_pin, int step_power);
	void setStepsPerRevolution(int steps_per_rev);
	void setMinSpeed(int min_speed);
	void setTopSpeed(int top_speed);
	void setAccelerationType(String accel_type);
	void setAccelerationLength(int accel_length);
	void enableMotor();
	void disableMotor();
	void setDisableMotorOnStep(bool disable);

	float calculate_linear_rpm(int distance);
	float calculate_curved_rpm(int distance);
	float rpm_to_delay(float rpm);
	void pulse();
};

#endif // PROFILEDSTEPPER_H
