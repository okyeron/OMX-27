#include "omx_screensaver.h"
#include "../globals.h"
#include "../consts/consts.h"
#include "../consts/colors.h"
#include "../config.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"
#include <cmath>

// Bright colors for obstacles (same as seqColors style)
static const uint32_t kObstacleColors[] = {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, ORANGE, LIME};
static const int kNumObstacleColors = sizeof(kObstacleColors) / sizeof(kObstacleColors[0]);

// Full-wheel hues 1..kFullWheelHueTop; hue 0 = all LEDs off. Raw ADC rarely hits potMinVal exactly at CCW,
// so the bottom ~1/256 of constrained travel maps to off (same idea as using the low 7-bit step as "0").
static const long kFullWheelHueTop = 65527;
// Hue delta per encoder "unit". `Encoder::accel(rate)` scales with spin speed (dir + dir*speedup*rate).
// Midpoint between first encoder mapping (350 / rate 1) and faster pass (2200 / rate 5).
static const long kEncoderHueStep = 1275;
static const int kEncoderHueAccelRate = 3;

void OmxScreensaver::setScreenSaverColor()
{
	int raw = constrain(potSettings.analog[4]->getValue(), potMinVal, potMaxVal);
	long span = (long)potMaxVal - (long)potMinVal;
	if (span <= 0)
	{
		colorConfig.screensaverColor = 0;
		return;
	}
	long pos = (long)raw - (long)potMinVal; // 0 .. span
	const long offSlice = max(1L, span / 256);
	if (pos <= offSlice)
	{
		colorConfig.screensaverColor = 0;
	}
	else
	{
		colorConfig.screensaverColor = (uint32_t)map(pos, offSlice + 1, span, 1L, kFullWheelHueTop);
	}
}

void OmxScreensaver::toggleGame()
{
    gameActive_ = !gameActive_;
    if (gameActive_)
    {
        gameResetRound();
        gameStarted_ = false;  // player must hold AUX and release to start
        auxWasHeldLastTick_ = false;
        nextGameTick_ = millis();
        nextObstacleSpawn_ = millis();
    }
    else
    {
        // Reset screensaver step clock so it doesn't "catch up" (nextStepTimeSS was never updated during the game).
        nextStepTimeSS = millis();
    }
}

void OmxScreensaver::clearGameStateOnSaverExit()
{
	if (!gameActive_)
		return;
	gameActive_ = false;
	gameResetRound();
	losePulseCount_ = 0;
	hitObstacleIndex_ = -1;
	gameStarted_ = false;
	auxWasHeldLastTick_ = false;
	playerDodging_ = false;
	gameSpeedMs_ = 400;
	obstacleSpawnIntervalMs_ = 1200;
	nextStepTimeSS = millis();
}

uint32_t OmxScreensaver::gameRandomBrightColor()
{
    return kObstacleColors[random(0, kNumObstacleColors)];
}

// Dodge "back": player shifts to the black key behind their current main key. Can only dodge when there is a black key "back".
// 12: can't | 13→1, 14→2 | 15: can't | 16→3, 17→4, 18→5 | 19: can't | 20→6, 21→7 | 22: can't | 23→8, 24→9, 25→10 | 26: win
int OmxScreensaver::gameGetDodgeBlackKey(int mainPosition) const
{
    if (mainPosition == 13) return 1;
    if (mainPosition == 14) return 2;
    if (mainPosition == 16) return 3;
    if (mainPosition == 17) return 4;
    if (mainPosition == 18) return 5;
    if (mainPosition == 20) return 6;
    if (mainPosition == 21) return 7;
    if (mainPosition == 23) return 8;
    if (mainPosition == 24) return 9;
    if (mainPosition == 25) return 10;
    return 0;
}

bool OmxScreensaver::gameCanDodgeAt(int mainPosition) const
{
    return gameGetDodgeBlackKey(mainPosition) != 0;
}

void OmxScreensaver::gameSpawnObstacle()
{
    // Rightmost key of any active obstacle (growing obstacles have right=26)
    int maxRight = -1;
    for (int i = 0; i < kMaxObstacles; i++)
    {
        if (!gameObstacles_[i].active)
            continue;
        int r;
        if (gameObstacles_[i].growth < gameObstacles_[i].width)
            r = 26;  // still growing at key 26
        else
            r = gameObstacles_[i].leftEdge + (int)gameObstacles_[i].width - 1;
        if (r > maxRight)
            maxRight = r;
    }
    // New obstacle appears at key 26 first (growth 1), then grows left
    uint8_t w = (uint8_t)random(1, 6);
    int newLeft = 27 - (int)w;  // used once fully grown
    // No overlap: no active obstacle may extend into [newLeft, 26]
    if (maxRight >= newLeft)
        return;
    // Spacing: 1 to 4 empty keys between rightmost existing and where we'll appear (key 26)
    if (maxRight >= 0)
    {
        int gap = 26 - maxRight - 1;  // keys between maxRight and 26
        if (gap < 1 || gap > 4)
            return;
    }
    for (int i = 0; i < kMaxObstacles; i++)
    {
        if (!gameObstacles_[i].active)
        {
            gameObstacles_[i].active = true;
            gameObstacles_[i].width = w;
            gameObstacles_[i].growth = 1;  // first tick: only key 26
            gameObstacles_[i].leftEdge = 26;
            gameObstacles_[i].color = gameRandomBrightColor();
            return;
        }
    }
}

void OmxScreensaver::gameResetRound()
{
    gamePlayerPosition_ = 11;
    for (int i = 0; i < kMaxObstacles; i++)
    {
        gameObstacles_[i].active = false;
        gameObstacles_[i].growth = 0;
    }
}

void OmxScreensaver::gameUpdate()
{
    unsigned long now = millis();

    // Lose state: pulse hit obstacle 5 times (~1200ms) then reset
    if (losePulseCount_ > 0)
    {
        if (losePulseCount_ == 4)
        {
            gameResetRound();
            gameStarted_ = false;  // safe at 11 until hold AUX and release
            auxWasHeldLastTick_ = false;
            gameSpeedMs_ = (unsigned long)constrain((int)gameSpeedMs_ + 25, (int)gameSpeedMs_, (int)kGameSpeedMaxMs_);
            obstacleSpawnIntervalMs_ = (int)constrain(obstacleSpawnIntervalMs_ + 50, obstacleSpawnIntervalMs_, kObstacleSpawnIntervalMaxMs_);
            losePulseCount_ = 0;
            hitObstacleIndex_ = -1;
            nextGameTick_ = millis();  // avoid double-tick after reset
        }
        return;
    }

    if (now < nextGameTick_)
        return;
    nextGameTick_ = now + gameSpeedMs_;

    // Not started yet: at 11, safe. Obstacles still run. Start only on AUX press-and-release (holding AUX keeps them safe at 11).
    if (!gameStarted_)
    {
        if (auxWasHeldLastTick_ && !midiSettings.keyState[0])
            gameStarted_ = true;
        auxWasHeldLastTick_ = midiSettings.keyState[0];
        // Run obstacles and spawn (don't advance player); then return (no collision at 11).
        for (int i = 0; i < kMaxObstacles; i++)
        {
            if (!gameObstacles_[i].active)
                continue;
            if (gameObstacles_[i].growth < gameObstacles_[i].width)
            {
                gameObstacles_[i].growth++;
                if (gameObstacles_[i].growth == gameObstacles_[i].width)
                    gameObstacles_[i].leftEdge = 27 - (int)gameObstacles_[i].width;
            }
            else
            {
                gameObstacles_[i].leftEdge--;
                if (gameObstacles_[i].leftEdge + (int)gameObstacles_[i].width < 11)
                {
                    gameObstacles_[i].active = false;
                    gameObstacles_[i].growth = 0;
                }
            }
        }
        if (now >= nextObstacleSpawn_)
        {
            nextObstacleSpawn_ = now + obstacleSpawnIntervalMs_;
            gameSpawnObstacle();
        }
        return;
    }

    // Dodge "back": AUX held and on a key that can dodge. While dodging, player does not move forward.
    playerDodging_ = (midiSettings.keyState[0] && gameCanDodgeAt(gamePlayerPosition_));

    // Advance only when not dodging
    if (!playerDodging_)
    {
        gamePlayerPosition_++;
        if (gamePlayerPosition_ > 26)
        {
            gamePlayerPosition_ = 11;
            gameScore_++;
            gameStarted_ = false;  // safe at 11 until hold AUX and release (avoid instant lose)
            auxWasHeldLastTick_ = false;
            gameSpeedMs_ = (unsigned long)constrain((int)gameSpeedMs_ - 40, 100, (int)gameSpeedMs_);  // more pronounced speed-up
            obstacleSpawnIntervalMs_ = (int)constrain(obstacleSpawnIntervalMs_ - 100, 350, obstacleSpawnIntervalMs_);
            return;
        }
    }

    // Update obstacles: grow from right (26), then move left
    for (int i = 0; i < kMaxObstacles; i++)
    {
        if (!gameObstacles_[i].active)
            continue;
        if (gameObstacles_[i].growth < gameObstacles_[i].width)
        {
            gameObstacles_[i].growth++;
            if (gameObstacles_[i].growth == gameObstacles_[i].width)
                gameObstacles_[i].leftEdge = 27 - (int)gameObstacles_[i].width;
        }
        else
        {
            gameObstacles_[i].leftEdge--;
            if (gameObstacles_[i].leftEdge + (int)gameObstacles_[i].width < 11)
            {
                gameObstacles_[i].active = false;
                gameObstacles_[i].growth = 0;
            }
        }
    }

    // Spawn new obstacle (only when spacing allows: 1-4 empty keys, no overlap)
    if (now >= nextObstacleSpawn_)
    {
        nextObstacleSpawn_ = now + obstacleSpawnIntervalMs_;
        gameSpawnObstacle();
    }

    auxWasHeldLastTick_ = midiSettings.keyState[0];

    // Collision (only when not dodging): lose round. Position 11 is safe (no collision when spawning).
    if (!playerDodging_ && gamePlayerPosition_ != 11)
    {
        for (int i = 0; i < kMaxObstacles; i++)
        {
            if (!gameObstacles_[i].active)
                continue;
            int le, right;
            if (gameObstacles_[i].growth < gameObstacles_[i].width)
            {
                le = 27 - (int)gameObstacles_[i].growth;
                right = 26;
            }
            else
            {
                le = gameObstacles_[i].leftEdge;
                right = le + (int)gameObstacles_[i].width - 1;
            }
            if (gamePlayerPosition_ >= le && gamePlayerPosition_ <= right)
            {
                losePulseCount_ = 1;
                hitObstacleIndex_ = i;
                losePulseStartMs_ = now;
                return;
            }
        }
    }
}

void OmxScreensaver::gameDrawLEDs()
{
    // Clear all
    for (int i = 0; i < 27; i++)
        strip.setPixelColor(i, 0);

    unsigned long now = millis();
    bool inLosePulse = (losePulseCount_ > 0 && losePulseCount_ <= 3);
    if (inLosePulse && hitObstacleIndex_ >= 0 && hitObstacleIndex_ < kMaxObstacles && gameObstacles_[hitObstacleIndex_].active)
    {
        // After 5 pulses (~1200ms), signal done so next gameUpdate resets
        if (now - losePulseStartMs_ > 1200)
            losePulseCount_ = 4;
        // Pulse hit obstacle: bright / dim every 120ms
        int phase = (now / 120) % 2;
        uint32_t c = gameObstacles_[hitObstacleIndex_].color;
        uint8_t r = (c >> 16) & 0xff, g = (c >> 8) & 0xff, b = c & 0xff;
        if (!phase)
        {
            r = (uint8_t)(r * 0.3f);
            g = (uint8_t)(g * 0.3f);
            b = (uint8_t)(b * 0.3f);
            c = strip.Color(r, g, b);
        }
        else
            c = strip.gamma32(c);
        int le, count;
        if (gameObstacles_[hitObstacleIndex_].growth < gameObstacles_[hitObstacleIndex_].width)
        {
            le = 27 - (int)gameObstacles_[hitObstacleIndex_].growth;
            count = gameObstacles_[hitObstacleIndex_].growth;
        }
        else
        {
            le = gameObstacles_[hitObstacleIndex_].leftEdge;
            count = gameObstacles_[hitObstacleIndex_].width;
        }
        for (int k = 0; k < count && le + k <= 26; k++)
        {
            int idx = le + k;
            if (idx >= 11)
                strip.setPixelColor(idx, c);
        }
    }
    else if (!inLosePulse)
    {
    // Draw obstacles (solid bright); growing ones occupy 27-growth..26
    for (int i = 0; i < kMaxObstacles; i++)
    {
        if (!gameObstacles_[i].active)
            continue;
        int le, count;
        if (gameObstacles_[i].growth < gameObstacles_[i].width)
        {
            le = 27 - (int)gameObstacles_[i].growth;
            count = gameObstacles_[i].growth;
        }
        else
        {
            le = gameObstacles_[i].leftEdge;
            count = gameObstacles_[i].width;
        }
        uint32_t c = strip.gamma32(gameObstacles_[i].color);
        for (int k = 0; k < count && le + k <= 26; k++)
        {
            int idx = le + k;
            if (idx >= 11)
                strip.setPixelColor(idx, c);
        }
    }

    // Player: pulsing white (or on black key when dodging, using zone mapping) — only when not in lose pulse
    if (!inLosePulse)
    {
        int playerLed = gamePlayerPosition_;
        if (playerDodging_)
        {
            int black = gameGetDodgeBlackKey(gamePlayerPosition_);
            if (black != 0)
                playerLed = black;
        }
        if (playerLed >= 1 && playerLed <= 26)
        {
            float pulse = 0.5f + 0.5f * sinf(2.0f * 3.14159f * (float)(playerPulseMs_ % kPlayerPulsePeriodMs_) / (float)kPlayerPulsePeriodMs_);
            uint32_t white = strip.Color( (uint8_t)(255 * pulse), (uint8_t)(255 * pulse), (uint8_t)(255 * pulse) );
            strip.setPixelColor(playerLed, strip.gamma32(white));
        }
    }
    }
    omxLeds.setDirty();
}

void OmxScreensaver::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
	// Hue while saving: encoder (onEncoderChanged). Initial hue from knob 5 at saver entry (setScreenSaverColor).
	// Physical knobs 1–5: clear idle to wake — not while AUX is held (grip / mux bleed).
	if (potIndex < potCount && !midiSettings.keyState[0])
	{
		screenSaverCounter = 0;
	}
}

void OmxScreensaver::requestImmediateStart()
{
	pendingImmediateStart_ = true;
}

void OmxScreensaver::updateScreenSaverState()
{
	if (pendingImmediateStart_)
	{
		pendingImmediateStart_ = false;
		screenSaverCounter = screensaverInterval + 1;
		if (!screenSaverActive)
		{
			screenSaverActive = true;
			setScreenSaverColor();
		}
		nextStepTimeSS = millis();
		return;
	}

	if (screenSaverCounter > screensaverInterval)
	{
		if (!screenSaverActive)
		{
			screenSaverActive = true;
			setScreenSaverColor();
		}
	}
	else if (screenSaverCounter < 10)
	{
		ssstep = 0;
		ssloop = 0;
		screenSaverActive = false;
		nextStepTimeSS = millis();
	}
	else
	{
		screenSaverActive = false;
		nextStepTimeSS = millis();
	}
}

bool OmxScreensaver::shouldShowScreenSaver()
{
    return screenSaverActive;
}

void OmxScreensaver::onEncoderChanged(Encoder::Update enc)
{
	if (gameActive_)
		return;

	const int amt = enc.accel(kEncoderHueAccelRate);
	if (amt == 0)
		return;

	// Stay in screensaver: do not touch screenSaverCounter.
	long h = (long)colorConfig.screensaverColor;
	h += (long)amt * kEncoderHueStep;
	if (h < 0)
		h = 0;
	else if (h > kFullWheelHueTop)
		h = kFullWheelHueTop;

	colorConfig.screensaverColor = (uint32_t)h;
	omxLeds.setDirty();
}

void OmxScreensaver::onKeyUpdate(OMXKeypadEvent e)
{
}

void OmxScreensaver::updateLEDs()
{
    if (gameActive_)
    {
        gameUpdate();
        gameDrawLEDs();
        return;
    }
	unsigned long playstepmillis = millis();
	if (playstepmillis > nextStepTimeSS)
	{
		ssstep = ssstep % 16;
		ssloop = ssloop % 16;

		int j = 26 - ssloop;
		int i = ssstep + 11;
		int saturation = 255;
		int brightness = 255;

		for (int z = 1; z < 11; z++)
		{
			strip.setPixelColor(z, 0, 0, 0);
		}
		// Hue 0 = all off (initial CCW slice from knob 5 at saver entry). Nonzero: main-style animation.
		if (colorConfig.screensaverColor != 0)
		{
			if (!ssreverse)
			{
				for (int x = 0; x < 16; x++)
				{
					if (i < j)
					{
						strip.setPixelColor(x + 11, 0, 0, 0);
					}
					if (x + 11 > j)
					{
						strip.setPixelColor(x + 11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor, saturation, brightness)));
					}
				}
				strip.setPixelColor(i + 1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor, saturation, brightness)));
			}
			else
			{
				for (int y = 0; y < 16; y++)
				{
					if (i >= j)
					{
						strip.setPixelColor(y + 11, 0, 0, 0);
					}
					if (y + 11 < j)
					{
						strip.setPixelColor(y + 11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor, saturation, brightness)));
					}
				}
				strip.setPixelColor(i + 1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor, saturation, brightness)));
			}
		}
		else
		{
			for (int w = 0; w < 27; w++)
			{
				strip.setPixelColor(w, 0, 0, 0);
			}
		}
		ssstep++;
		if (ssstep == 16)
		{
			ssloop++;
		}
		if (ssloop == 16)
		{
			ssreverse = !ssreverse;
		}
		nextStepTimeSS = nextStepTimeSS + sleepTick;

		omxLeds.setDirty();
	}
}

void OmxScreensaver::resetCounter()
{
    screenSaverCounter = 0;
}

void OmxScreensaver::setIntervalMinutes(uint8_t minutes)
{
    minutes = (uint8_t)constrain((int)minutes, 1, 60);
    screensaverInterval = (unsigned long)minutes * 60 * 1000;
}

void OmxScreensaver::onDisplayUpdate()
{
    updateLEDs();
    if (gameActive_)
        omxDisp.drawGameScore(gameScore_);
    else
        omxDisp.clearDisplay();
}

