#pragma once

#include "omx_mode_interface.h"
#include <elapsedMillis.h>

// Easter egg game: obstacle on main lane (keys 11-26)
struct GameObstacle
{
	int8_t leftEdge; // first key index (11-26); while growing, right edge is 26
	uint8_t width;	 // 1-5 full width
	uint8_t growth; // 0 = inactive, 1..width = growing from right (key 26); when growth==width, obstacle moves
	uint32_t color;
	bool active;
};

class OmxScreensaver : public OmxModeInterface
{
public:
	OmxScreensaver() {}
	~OmxScreensaver() {}

	void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;

	void updateLEDs() override;

	void resetCounter();

	void setIntervalMinutes(uint8_t minutes); // 1–60, sets screensaver delay in minutes

	void updateScreenSaverState();
	bool shouldShowScreenSaver();

	/** Next updateScreenSaverState() will enter screensaver immediately (e.g. Keys UI version page + encoder click). */
	void requestImmediateStart();

	void onEncoderChanged(Encoder::Update enc) override;

	void onEncoderButtonDown() override{};
	void onEncoderButtonDownLong() override{};

	void onKeyUpdate(OMXKeypadEvent e) override;
	void onKeyHeldUpdate(OMXKeypadEvent e) override{};

	void onDisplayUpdate() override;

	// Easter egg game (when in screensaver, encoder click toggles)
	void toggleGame();
	bool isGameActive() const { return gameActive_; }
	// Call when leaving screensaver so the next idle period starts in normal saver, not mid-game.
	// Easter egg score is kept until power-off (RAM); not cleared here.
	void clearGameStateOnSaverExit();

private:
	void setScreenSaverColor();

	elapsedMillis screenSaverCounter = 0;
	unsigned long screensaverInterval = 1000 * 60 * 3;
	int ssstep = 0;
	int ssloop = 0;
	volatile unsigned long nextStepTimeSS = 0;
	bool ssreverse = false;

	int sleepTick = 80;

	bool screenSaverActive = false;
	bool pendingImmediateStart_ = false;

	// Game state
	bool gameActive_ = false;
	bool gameStarted_ = false; // false until player holds AUX and releases (to start run)
	bool auxWasHeldLastTick_ = false;
	int8_t gamePlayerPosition_ = 11; // 11-26 main lane; 11 is safe (no collision)
	int gameScore_ = 0;
	unsigned long gameSpeedMs_ = 400;
	unsigned long nextGameTick_ = 0;
	bool playerDodging_ = false;
	static const int kMaxObstacles = 8;
	GameObstacle gameObstacles_[kMaxObstacles];
	unsigned long nextObstacleSpawn_ = 0;
	int obstacleSpawnIntervalMs_ = 1200;
	int losePulseCount_ = 0;
	int hitObstacleIndex_ = -1;
	unsigned long losePulseStartMs_ = 0;
	elapsedMillis playerPulseMs_ = 0;
	static const unsigned long kGameSpeedMaxMs_ = 600;
	static const int kObstacleSpawnIntervalMaxMs_ = 2000;
	const int kPlayerPulsePeriodMs_ = 200;

	void gameUpdate();
	void gameDrawLEDs();
	void gameResetRound();
	void gameSpawnObstacle();
	uint32_t gameRandomBrightColor();
	int gameGetDodgeBlackKey(int mainPosition) const;
	bool gameCanDodgeAt(int mainPosition) const;
};
