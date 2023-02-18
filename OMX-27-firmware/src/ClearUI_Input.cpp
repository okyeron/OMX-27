#include "ClearUI_Input.h"

#include <Arduino.h>

Encoder::Encoder(uint32_t pinA, uint32_t pinB)
		: pinA(pinA), pinB(pinB)
{
	pinMode(pinA, INPUT_PULLUP);
	pinMode(pinB, INPUT_PULLUP);
	a = digitalRead(pinA);
	b = digitalRead(pinB);
	quads = 0;
	lastUpdate = 0;
}

Encoder::Update Encoder::update() {
	int newA = digitalRead(pinA);
	int newB = digitalRead(pinB);

	int16_t dir = 0;

	if (newA != a || newB != b) {
		if (newA == a) {
			quads += (newA == newB) ? 1 : -1;
		} else if (newB == b) {
			quads += (newA != newB) ? 1 : -1;
		}

		a = newA;
		b = newB;

		if (a && b) {
			if (quads > 1) {
				dir = 1;
			} else if (quads < -1) {
				dir = -1;
			}
			quads = 0;
		}
	}

	int16_t speedup = 0;
	if (dir != 0) {
		auto now = millis();
		auto delta = now - lastUpdate;
		lastUpdate = now;

		if (delta < 20)       speedup = 2;
		else if (delta < 50)  speedup = 1;
	}
	return Update(dir, speedup);
}

Button::Button(uint32_t pin)
	: pin(pin)
{
	pinMode(pin, INPUT_PULLUP);   // 1 is off, 0 is pressed
	lastRead = -1;        // will cause first update to always set it
	validAtTime = 0;

	state = Up;
	longAtTime = 0;
}

Button::State Button::update()
{
	int read = digitalRead(pin);
	if (read != lastRead) {
		// pin changed, wait for it to be stable
		lastRead = read;
		validAtTime = millis() + validAtTimeDelay;
		return NoChange;
	}

	uint32_t now = millis();
	if (now < validAtTime) {
		// pin stable, not not long enough
		return NoChange;
	}

	State prevState = state;

	switch (state) {
		case Up:
		case UpLong:
			if (lastRead == LOW) {
				state = Down;
				longAtTime = now + longDownTimeout;
			}
			break;

		case Down:
			if (lastRead == LOW) {  // still down?
				if (now > longAtTime) {
					state = DownLong;
					break;
				}
			}
			// fall through

		case DownLong:
			if (lastRead == HIGH) {
				state = (prevState == DownLong) ? UpLong : Up;
			}
			break;

		default:
			break;
	}

	return (state != prevState) ? state : NoChange;
}


IdleTimeout::IdleTimeout(unsigned long period)
	: idle(true), period(period)
	{ }

void IdleTimeout::activity() {
	idle = false;
	idleAtTime = millis() + period;
}

bool IdleTimeout::update() {
	if (idle)
		return false;

	if (millis() > idleAtTime) {
		idle = true;
		return true;
	}

	return false;
}
