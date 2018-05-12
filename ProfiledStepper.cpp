#include "Arduino.h"
#include "ProfiledStepper.h"

ProfiledStepper::ProfiledStepper(int ena_low, int ena_high, int dir_pin, int dir_power, int step_pin, int step_power, int steps_per_rev, int min_speed, int top_speed, String accel_type, int accel_length, bool disable) {
	setPins(ena_low, ena_high, dir_pin, dir_power, step_pin, step_power);
	setStepsPerRevolution(steps_per_rev);
	setMinSpeed(min_speed);
	setTopSpeed(top_speed);
	setAccelerationType(accel_type);
	setAccelerationLength(accel_length);
	setDisableMotorOnStep(disable);
}

void ProfiledStepper::setPins(int ena_low, int ena_high, int dir_pin, int dir_power, int step_pin, int step_power) {
	this->ena_low = ena_low;
	this->dir_pin = dir_pin;
	this->step_pin = step_pin;

	pinMode(ena_low, OUTPUT);
	pinMode(ena_high, OUTPUT);
	pinMode(dir_pin, OUTPUT);
	pinMode(dir_power, OUTPUT);
	pinMode(step_pin, OUTPUT);
	pinMode(step_power, OUTPUT);

	digitalWrite(ena_high, HIGH);
	digitalWrite(dir_power, HIGH);
	digitalWrite(step_pin, HIGH);
}

void ProfiledStepper::setStepsPerRevolution(int steps_per_revolution) {
	this->steps_per_rev = steps_per_rev;
}

void ProfiledStepper::setMinSpeed(int min_speed) {
	this->min_speed = min_speed;
}

void ProfiledStepper::setTopSpeed(int top_speed) {
	this->top_speed = top_speed;
}

void ProfiledStepper::setAccelerationType(String accel_type) {
	accel_type.toLowerCase();
	linear_acceleration = accel_type.equals("linear");
}

void ProfiledStepper::setAccelerationLength(int accel_length) {
	this->accel_length = accel_length;
}

void ProfiledStepper::setDisableMotorOnStep(bool disable) {
	disable_motor_on_step = disable;
}

void ProfiledStepper::enableMotor() {
	digitalWrite(ena_low, HIGH);
}

void ProfiledStepper::disableMotor() {
		digitalWrite(ena_low, LOW);
}

void ProfiledStepper::step(int distance) {
	enableMotor();

	accel_coefficient = ((float)top_speed-(float)min_speed)/(float)accel_length;

	a = top_speed/2-min_speed;
	accel_b = accel_length/2;
	deccel_b = abs(distance)-accel_length/2;
	c = accel_length/8;
	d = top_speed/2+min_speed;

	int profile[distance];

	for(int i = 0; i < sizeof(profile)/sizeof(int); i++) {
		if(linear_acceleration) {
			profile[i] = rpm_to_delay(calculate_linear_rpm(abs(distance)), i);
		}
		else {
			profile[i] = rpm_to_delay(calculate_curved_rpm(abs(distance)), i);
		}
	}

	if(distance >= 0) {
		digitalWrite(dir_pin, HIGH);
	}
	else {
		digitalWrite(dir_pin, LOW);
	}

	while(current_position < abs(distance)) {
		//int dly = rpm_to_delay(calculate_rpm(abs(distance)));

		pulse();
		delayMicroseconds(profile[current_position]);
		current_position++;
	}
	if(disable_motor_on_step) {
		disableMotor();
	}
	current_position = 0;
}

void ProfiledStepper::stepByRevolution(float revs) {
	step(revs*steps_per_rev);
}

float ProfiledStepper::calculate_linear_rpm(int distance, int current) {
	if(current_position < accel_length) {
		if(current > 0) {
			return accel_coefficient*(float)(current);
		}
		else {
			return accel_coefficient*(float)(current+1);
		}
	}
	else if(current > distance-accel_length && current < distance) {
		return -accel_coefficient*(float)(current-distance);
	}
	else {
		return (float)top_speed;
	}
}

float ProfiledStepper::calculate_curved_rpm(int distance) {
	if(current_position < accel_length) {
		return a*tanh((current_position-accel_b)/c)+d;
	}
	else if(current_position > distance-accel_length && current_position < distance) {
		return a*tanh((current_position-deccel_b)/c)+d;
	}
	else {
		return (float)top_speed;
	}
}

float ProfiledStepper::rpm_to_delay(float rpm) {
	return (float)60/((float)steps_per_rev*(float)rpm)*(float)1e6;
}

void ProfiledStepper::pulse() {
	digitalWrite(step_pin, LOW);
	digitalWrite(step_pin, HIGH);
}
