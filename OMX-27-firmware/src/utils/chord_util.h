#pragma once
#include "../config.h"
#include "../utils/music_scales.h"
#include "../modes/omx_mode_interface.h"
#include "../modes/submodes/submode_clearstorage.h"

// Singleton class for making chords
class ChordUtil
{
public:
	ChordUtil()
	{
	}

private:
};

extern ChordUtil chordUtil;
