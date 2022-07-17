#include "euclidean_sequencer.h"
#include "MM.h"

namespace euclidean
{
  
  EuclideanMath::EuclideanMath()
  {
  }

  // bool array should be of length kPatternSize
  void EuclideanMath::generateEuclidPattern(bool *pattern, uint8_t rotation, uint8_t events, uint8_t steps)
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
      rotatePattern(pattern, steps, rotation);
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
    //   tickCount_ = 0;
    //   for (auto i = 0; i < num_notes; i++)
    //   {
    //       midiChannels_[i] = defaultMidiChannel_;
    //       channelTriggered_[i] = false;
    //       density_[i] = i == 0 ? 128 : 64;
    //       perturbations_[i] = 0;
    //       x_[i] = 128;
    //       y_[i] = 128;
    //   }

    //   accent = 128;
    //   chaos = 0;
    //   divider_ = 0;
    //   multiplier_ = 1;
    //   running_ = false;

    //   // Init default snapshot notes
    //   for(int8_t s = 0; s < 8; s++)
    //   {
    //     for(int8_t i = 0; i < 4; i++)
    //     {
    //       snapshots[s].instruments[i].note = grids_notes[i];
    //     }
    //   }
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
      running_ = true;
  }

  void EuclideanSequencer::stop()
  {
      running_ = false;
  }

  void EuclideanSequencer::proceed()
  {
      running_ = true;
  }

  void EuclideanSequencer::gridsTick()
  {
    //   if (!running_)
    //       return;

    //   uint32_t ticksPerClock = 3 << divider_;
    //   bool trigger = ((tickCount_ % ticksPerClock) == 0);

    //   if (trigger)
    //   {
    //       const auto step = (tickCount_ / ticksPerClock * multiplier_) % grids::kStepsPerPattern;
    //       channel_.setStep(step);

    //       for (auto channel = 0; channel < num_notes; channel++)
    //       {
    //           if (step == 0)
    //           {
    //               uint32_t r = randomValue();
    //               perturbations_[channel] = ((r & 0xFF) * (chaos >> 2)) >> 8;
    //           }

    //           const uint8_t threshold = ~density_[channel];
    //           auto level = channel_.level(channel, x_[channel], y_[channel]);
    //           if (level < 255 - perturbations_[channel])
    //           {
    //               level += perturbations_[channel];
    //           }

    //           if (level > threshold)
    //           {
    //               uint8_t targetLevel = uint8_t(127.f * float(level - threshold) / float(256 - threshold));
    //               uint8_t noteLevel = GridsChannel::U8Mix(127, targetLevel, accent);
    //               MM::sendNoteOn(grids_notes[channel], noteLevel, midiChannels_[channel]);
    //               triggeredNotes_[channel] = grids_notes[channel];
    //               channelTriggered_[channel] = true;
    //           }
    //       }
    //   }
    //   else
    //   {
    //       for (auto channel = 0; channel < num_notes; channel++)
    //       {
    //           if (channelTriggered_[channel])
    //           {
    //               MM::sendNoteOff(triggeredNotes_[channel], 0, midiChannels_[channel]);
    //               // MM::sendNoteOff(grids_notes[channel], 0, midiChannels_[channel]);
    //               channelTriggered_[channel] = false;
    //           }
    //       }
    //   }
    //   tickCount_++;
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