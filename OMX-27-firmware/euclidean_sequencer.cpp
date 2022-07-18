#include "euclidean_sequencer.h"
#include "MM.h"
#include "noteoffs.h"

namespace euclidean
{
    const float kEuclidNoteLengths[] = {0.1, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8, 16};
    const uint8_t kNumEuclidNoteLengths = 10;
  
  EuclideanMath::EuclideanMath()
  {
  }

  // bool array should be of length kPatternSize
  void EuclideanMath::generateEuclidPattern(bool *pattern, uint8_t events, uint8_t steps)
  {
      clearPattern(pattern);

      // a value of true for each array element indicates a pulse

      uint8_t bucket = 0; // out variable to add pulses together for each step

      // fill array with rhythm
      for (uint8_t i = 0; i < steps; i++)
      {
          bucket += events;
          if (bucket >= steps)
          {
              bucket -= steps;
              pattern[i] = true;
          }
      }

      flipPattern(pattern, steps);
    //   rotatePattern(pattern, steps, rotation);
  }
  // bool array should be of length kPatternSize
  void EuclideanMath::clearPattern(bool *pattern)
  {
    for(int i = 0; i < kPatternSize; i++){
        pattern[i] = false;
    }
  }
  // bool array should be of length kPatternSize
  void EuclideanMath::flipPattern(bool *pattern, uint8_t steps)
  {
      bool temp[steps];

      for (int i = 0; i < steps; i++)
      {
          temp[i] = pattern[steps - 1 - i];
      }

      for (int i = 0; i < steps; i++)
      {
          pattern[i] = temp[i];
      }
  }
  // bool array should be of length kPatternSize
  void EuclideanMath::rotatePattern(bool *pattern, uint8_t steps, uint8_t rotation)
  {
      bool temp[steps];

      uint8_t val = steps - rotation;

      for (uint8_t i = 0; i < steps; i++)
      {
          temp[i] = pattern[abs((i + val) % steps)];
      }
      for (int i = 0; i < steps; i++)
      {
          pattern[i] = temp[i];
      }
  }

  EuclideanSequencer::EuclideanSequencer()
  {
    for(uint8_t i = 0; i < EuclideanMath::kPatternSize; i++){
        pattern_[i] = false;
    }

        regeneratePattern();
      tickCount_ = 0;
    //   for (auto i = 0; i < num_notes; i++)
    //   {
    //       midiChannels_[i] = defaultMidiChannel_;
    //       channelTriggered_[i] = false;
    //       density_[i] = i == 0 ? 128 : 64;
    //       perturbations_[i] = 0;
    //       x_[i] = 128;
    //       y_[i] = 128;
    //   }

      divider_ = 0;
      multiplier_ = 1;
      running_ = false;
  }

  void EuclideanSequencer::regeneratePattern()
  {
        EuclideanMath::generateEuclidPattern(pattern_, events_, steps_);
        EuclideanMath::rotatePattern(pattern_, steps_, rotation_);

        // printEuclidPattern();
  }

  uint32_t EuclideanSequencer::randomValue(uint32_t init)
  {
      uint32_t val = 0x12345;
      if (init)
      {
          val = init;
          return 0;
      }
      val = val * 214013 + 2531011;
      return val;
  }

  void EuclideanSequencer::start()
  {
      tickCount_ = 0;
      seqPos_ = 0;
      running_ = true;

      nextStepTimeP_ = micros();
      lastStepTimeP_ = micros();
  }

  void EuclideanSequencer::stop()
  {
      running_ = false;
  }

  void EuclideanSequencer::proceed()
  {
      running_ = true;
  }

  bool EuclideanSequencer::isDirty()
  {
      return patternDirty_;
  }

//   void EuclideanSequencer::setResolution(uint8_t r)
//   {
//       resolution_ = r;
//       divider_ = 0;
//       if (r == 0)
//       {
//           multiplier_ = 1;
//           divider_ = 1;
//       }
//       else if (r == 1)
//       {
//           multiplier_ = 1;
//       }
//       else if (r == 2)
//       {
//           multiplier_ = 2;
//       }

//       patternDirty_ = true;
//   }

//   uint8_t EuclideanSequencer::getResolution()
//   {
//       return resolution_;
//   }

  void EuclideanSequencer::setClockDivMult(uint8_t m)
  { 
    uint8_t prevDiv = clockDivMultP_;

      clockDivMultP_ = m;
      multiplier_ = multValues[m];

      if(clockDivMultP_ != prevDiv){
        // Serial.println((String)"clockDivMultP_: " + clockDivMultP_);
        patternDirty_ = true;
      }
  }

  uint8_t EuclideanSequencer::getClockDivMult()
  {
      return clockDivMultP_;
  }

  void EuclideanSequencer::setPolyRClockDivMult(uint8_t m)
  {
      uint8_t prevDiv = polyRClockDivMultP_;

      polyRClockDivMultP_ = m;
      multiplierPR_ = multValues[m];

      if (polyRClockDivMultP_ != prevDiv)
      {
          patternDirty_ = true;
      }
  }
  uint8_t EuclideanSequencer::getPolyRClockDivMult()
  {
      return polyRClockDivMultP_;
  }

  void EuclideanSequencer::setRotation(uint8_t newRotation)
  {
      if (newRotation != rotation_)
          patternDirty_ = true;
      rotation_ = newRotation;
  }
  uint8_t EuclideanSequencer::getRotation()
  {
      return rotation_;
  }
  void EuclideanSequencer::setEvents(uint8_t newEvents)
  {
      if (newEvents != events_)
          patternDirty_ = true;
      events_ = newEvents;
  }
  uint8_t EuclideanSequencer::getEvents()
  {
      return events_;
  }

  void EuclideanSequencer::setSteps(uint8_t newSteps)
  {
      if (newSteps != steps_)
          patternDirty_ = true;
      steps_ = newSteps;
  }
  uint8_t EuclideanSequencer::getSteps()
  {
      return steps_;
  }
  void EuclideanSequencer::setNoteNumber(uint8_t newNoteNumber)
  {
      noteNumber_ = newNoteNumber;
  }
  uint8_t EuclideanSequencer::getNoteNumber()
  {
      return noteNumber_;
  }
  void EuclideanSequencer::setNoteLength(uint8_t newNoteLength)
  {
      noteLength_ = newNoteLength;
  }
  uint8_t EuclideanSequencer::getNoteLength()
  {
      return noteLength_;
  }

  void EuclideanSequencer::setPolyRhythmMode(bool enable)
  {
      polyRhythmMode_ = enable;
  }
  bool EuclideanSequencer::getPolyRhythmMode()
  {
      return polyRhythmMode_;
  }

  bool *EuclideanSequencer::getPattern()
  {
      return pattern_;
  }

  void EuclideanSequencer::printEuclidPattern()
  {
      String sOut = "";
      for (uint8_t i = 0; i < steps_; i++)
      {
          sOut += (pattern_[i] ? "X" : "-");
      }
      Serial.println(sOut.c_str());
  }

  void EuclideanSequencer::clockTick(uint32_t stepmicros, uint32_t microsperstep)
  {
      if (patternDirty_)
      {
          regeneratePattern();
          patternDirty_ = false;
      }

      if (!running_ || steps_ == 0)
          return;

      if (stepmicros >= nextStepTimeP_)
      {
          lastStepTimeP_ = nextStepTimeP_;

          if (polyRhythmMode_)
          {
            //   uint8_t stepCount = max(steps_, 1);

              uint32_t timeDelta = ((microsperstep * 16) / steps_) * multiplierPR_;
              nextStepTimeP_ += timeDelta; // Space all triggers across a bar

              Serial.println((String) "microsperstep " + microsperstep);
              Serial.println((String) "timeDelta " + timeDelta);

          }
          else
          {
              nextStepTimeP_ += microsperstep * multiplier_; // calc step based on rate
          }

          bool trigger = pattern_[seqPos_];

          if (trigger)
          {
            playNote();
            // pendingNoteOns.insert(60, 100, 1, stepmicros, false);
            //   Serial.print((String) "X  ");
          }
          else
          {
            //   Serial.print((String) "-  ");
          }

          //   lastPosP_ = (seqPos_ + 15) % 16;

          advanceStep();

          if (seqPos_ == 0)
          {

            //   Serial.print("\n\n\n");
          }

        //   if (lastNote[j][sequencer.timePerPattern[j].lastPosP] > 0)
        //   {
        //       step_off(j, sequencer.timePerPattern[j].lastPosP);
        //   }
        //   if (testProb)
        //   {
        //       if (evaluate_AB(pattern->steps[sequencer.seqPos[j]].condition, j))
        //       {
        //           if (j == sequencer.playingPattern)
        //           {
        //               playNote(j);
        //           }
        //       }
        //   }

        //   playNote(j);

          // No need to have this function call in here.
          // Can put into omx_mode_sequencer and remove extern function
          // if (j == sequencer.playingPattern)
          // { // only show selected pattern
          // 	show_current_step(sequencer.playingPattern);
          // }
        //   new_step_ahead();
      }
    // //   Serial.print((String)" T: " + tickCount_);
    //   uint32_t ticksPerClock = 3 << divider_; // 3, 6, 12, etc
    //   bool trigger = ((tickCount_ % ticksPerClock) == 0);

    //   if (trigger)
    //   {
    //       const auto step = (tickCount_ / ticksPerClock * multiplier_) % kStepsPerPattern;

    //       if (step == 0)
    //       {
    //           Serial.print("\n\n\n");
    //       }
    //       Serial.print((String) " S: " + step);
    //       Serial.print((String) " T: " + tickCount_);
    //       Serial.print("    ");

    //       //   channel_.setStep(step);

    //       //   for (auto channel = 0; channel < num_notes; channel++)
    //       //   {
    //       //       if (step == 0)
    //       //       {
    //       //           uint32_t r = randomValue();
    //       //           perturbations_[channel] = ((r & 0xFF) * (chaos >> 2)) >> 8;
    //       //       }

    //       //       const uint8_t threshold = ~density_[channel];
    //       //       auto level = channel_.level(channel, x_[channel], y_[channel]);
    //       //       if (level < 255 - perturbations_[channel])
    //       //       {
    //       //           level += perturbations_[channel];
    //       //       }

    //       //       if (level > threshold)
    //       //       {
    //       //           uint8_t targetLevel = uint8_t(127.f * float(level - threshold) / float(256 - threshold));
    //       //           uint8_t noteLevel = GridsChannel::U8Mix(127, targetLevel, accent);
    //       //           MM::sendNoteOn(grids_notes[channel], noteLevel, midiChannels_[channel]);
    //       //           triggeredNotes_[channel] = grids_notes[channel];
    //       //           channelTriggered_[channel] = true;
    //       //       }
    //       //   }
    //   }
    //   else
    //   {
    //     //   for (auto channel = 0; channel < num_notes; channel++)
    //     //   {
    //     //       if (channelTriggered_[channel])
    //     //       {
    //     //           MM::sendNoteOff(triggeredNotes_[channel], 0, midiChannels_[channel]);
    //     //           // MM::sendNoteOff(grids_notes[channel], 0, midiChannels_[channel]);
    //     //           channelTriggered_[channel] = false;
    //     //       }
    //     //   }
    //   }
    //   tickCount_++;
  }


void EuclideanSequencer::advanceStep() {

    if(steps_ == 0){
        seqPos_ = 0;
        return;
    }

    seqPos_ = (seqPos_ + 1) % steps_;
    // autoReset();

	// step ONE pattern ahead one place
		// if (sequencer.getPattern(patternNum)->reverse) {
		// 	sequencer.seqPos[patternNum]--;
		// 	auto_reset(patternNum); // determine whether to reset or not based on param settings
		// } else {
		// 	sequencer.seqPos[patternNum]++;
		// 	auto_reset(patternNum); // determine whether to reset or not based on param settings
		// }
}

void EuclideanSequencer::autoReset() {
	// should be conditioned on whether we're in S2!!
	// if (sequencer.seqPos[p] >= sequencer.getPatternLength(p) ||
	// 		(pattern->autoreset && (pattern->autoresetstep > (pattern->startstep) ) && (sequencer.seqPos[p] >= pattern->autoresetstep)) ||
	// 		(pattern->autoreset && (pattern->autoresetstep == 0 ) && (sequencer.seqPos[p] >= pattern->rndstep)) ||
	// 		(pattern->reverse && (sequencer.seqPos[p] < 0)) || // normal reverse reset
	// 		(pattern->reverse && pattern->autoreset && (sequencer.seqPos[p] < pattern->startstep )) // ||
	// 		//(settings->reverse && settings->autoreset && (settings->autoresetstep == 0 ) && (seqPos[p] < settings->rndstep))
	// 	 ) {


	// 	if (pattern->reverse) {
	// 		if (pattern->autoreset){
	// 			if (pattern->autoresetstep == 0){
	// 				sequencer.seqPos[p] = pattern->rndstep-1;
	// 			}else{
	// 				sequencer.seqPos[p] = pattern->autoresetstep-1; // resets pattern in REV
	// 			}
	// 		} else {
	// 			sequencer.seqPos[p] = (sequencer.getPatternLength(p)-pattern->startstep)-1;
	// 		}

	// 	} else {
	// 		sequencer.seqPos[p] = (pattern->startstep); // resets pattern in FWD
	// 	}
	// 	if (pattern->autoresetfreq == pattern->current_cycle){ // reset cycle logic
	// 		if (probResult(pattern->autoresetprob)){
	// 			// chance of doing autoreset
	// 			pattern->autoreset = true;
	// 		} else {
	// 			pattern->autoreset = false;
	// 		}
	// 		pattern->current_cycle = 1; // reset cycle to start new iteration
	// 	} else {
	// 		pattern->autoreset = false;
	// 		pattern->current_cycle++; // advance to next cycle
	// 	}
	// 	pattern->rndstep = (rand() % sequencer.getPatternLength(p)) + 1; // randomly choose step for next cycle
	// }
	// sequencer.patternPage[p] = getPatternPage(sequencer.seqPos[p]); // FOLLOW MODE FOR SEQ PAGE

// return ()
}

void EuclideanSequencer::playNote() {
	bool sendnoteCV = false;
	// if (sequencer.getPattern(patternNum)->sendCV) {
	// 	sendnoteCV = true;
	// }

    // regular note on trigger
    // uint8_t note = 60;
    uint8_t channel = 1;
    uint8_t vel = 100;
    float stepLength = kEuclidNoteLengths[noteLength_];
    uint8_t swing = 0;

    uint32_t noteoff_micros = micros() + (stepLength) * clockConfig.step_micros;
    pendingNoteOffs.insert(noteNumber_, channel, noteoff_micros, sendnoteCV);

    uint32_t noteon_micros;

    if (swing > 0 && seqPos_ % 2 == 0)
    {
        if (swing < 99)
        {
            noteon_micros = micros() + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * swing); // full range swing
        }
        else if (swing == 99)
        {                                // random drunken swing
            uint8_t rnd_swing = rand() % 95 + 1; // rand 1 - 95 // randomly apply swing value
            noteon_micros = micros() + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * rnd_swing);
        }
    }
    else
    {
        noteon_micros = micros();
    }

    // Queue note-on
    pendingNoteOns.insert(noteNumber_, vel, 1, noteon_micros, sendnoteCV);

    // {notenum, vel, notelen, step_type, {p1,p2,p3,p4}, prob}
    // send param locks
    // for (int q = 0; q < 4; q++)
    // {
    //     int tempCC = steps[sequencer.seqPos[patternNum]].params[q];
    //     if (tempCC > -1)
    //     {
    //         MM::sendControlChange(pots[potSettings.potbank][q], tempCC, sequencer.getPatternChannel(patternNum));
    //         seqConfig.prevPlock[q] = tempCC;
    //     }
    //     else if (seqConfig.prevPlock[q] != potSettings.potValues[q])
    //     {
    //         // if (tempCC != seqConfig.prevPlock[q]) {
    //         MM::sendControlChange(pots[potSettings.potbank][q], potSettings.potValues[q], sequencer.getPatternChannel(patternNum));
    //         seqConfig.prevPlock[q] = potSettings.potValues[q];
    //     }
    // }
    // lastNote[patternNum][sequencer.seqPos[patternNum]] = steps[sequencer.seqPos[patternNum]].note;

    // CV is sent from pendingNoteOns/pendingNoteOffs
}

//   uint8_t EuclideanSequencer::getSeqPos()
//   {
//     return 0;
//     //   uint32_t ticksPerClock = 3 << divider_;
//     //   uint8_t step = (tickCount_ / ticksPerClock * multiplier_) % grids::kStepsPerPattern;
//     //   return step;
//   }

//   void EuclideanSequencer::setMidiChan(uint8_t chanIndex, uint8_t channel)
//   {
//     // if (chanIndex < 0 || chanIndex >= num_notes)
//     //   return;

//     // midiChannels_[chanIndex] = channel;
//   }
}