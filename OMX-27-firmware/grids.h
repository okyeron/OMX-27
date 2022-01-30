#pragma once

#include <cstdint>

namespace grids
{
  constexpr uint8_t kStepsPerPattern = 32;

  inline uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance)
  {
    uint16_t mix = b * balance;
    mix += (a * (255 - balance));
    return mix / 255;
  }

  class Channel
  {
  public:

    Channel();

    void setStep(uint8_t step);
    uint8_t level(int selector, uint16_t x, uint16_t y);

  private:
    uint8_t step_;
  };
}
