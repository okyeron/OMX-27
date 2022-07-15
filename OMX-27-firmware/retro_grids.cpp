#include "retro_grids.h"
#include "MM.h"

namespace grids
{
  const uint8_t node_0[] = {
    255,      0,      0,      0,      0,      0,    145,      0,
    0,      0,      0,      0,    218,      0,      0,      0,
    72,      0,     36,      0,    182,      0,      0,      0,
    109,      0,      0,      0,     72,      0,      0,      0,
    36,      0,    109,      0,      0,      0,      8,      0,
    255,      0,      0,      0,      0,      0,     72,      0,
    0,      0,    182,      0,      0,      0,     36,      0,
    218,      0,      0,      0,    145,      0,      0,      0,
    170,      0,    113,      0,    255,      0,     56,      0,
    170,      0,    141,      0,    198,      0,     56,      0,
    170,      0,    113,      0,    226,      0,     28,      0,
    170,      0,    113,      0,    198,      0,     85,      0,
  };
  const uint8_t node_1[] = {
    229,      0,     25,      0,    102,      0,     25,      0,
    204,      0,     25,      0,     76,      0,      8,      0,
    255,      0,      8,      0,     51,      0,     25,      0,
    178,      0,     25,      0,    153,      0,    127,      0,
    28,      0,    198,      0,     56,      0,     56,      0,
    226,      0,     28,      0,    141,      0,     28,      0,
    28,      0,    170,      0,     28,      0,     28,      0,
    255,      0,    113,      0,     85,      0,     85,      0,
    159,      0,    159,      0,    255,      0,     63,      0,
    159,      0,    159,      0,    191,      0,     31,      0,
    159,      0,    127,      0,    255,      0,     31,      0,
    159,      0,    127,      0,    223,      0,     95,      0,
  };
  const uint8_t node_2[] = {
    255,      0,      0,      0,    127,      0,      0,      0,
    0,      0,    102,      0,      0,      0,    229,      0,
    0,      0,    178,      0,    204,      0,      0,      0,
    76,      0,     51,      0,    153,      0,     25,      0,
    0,      0,    127,      0,      0,      0,      0,      0,
    255,      0,    191,      0,     31,      0,     63,      0,
    0,      0,     95,      0,      0,      0,      0,      0,
    223,      0,      0,      0,     31,      0,    159,      0,
    255,      0,     85,      0,    148,      0,     85,      0,
    127,      0,     85,      0,    106,      0,     63,      0,
    212,      0,    170,      0,    191,      0,    170,      0,
    85,      0,     42,      0,    233,      0,     21,      0,
  };
  const uint8_t node_3[] = {
    255,      0,    212,      0,     63,      0,      0,      0,
    106,      0,    148,      0,     85,      0,    127,      0,
    191,      0,     21,      0,    233,      0,      0,      0,
    21,      0,    170,      0,      0,      0,     42,      0,
    0,      0,      0,      0,    141,      0,    113,      0,
    255,      0,    198,      0,      0,      0,     56,      0,
    0,      0,     85,      0,     56,      0,     28,      0,
    226,      0,     28,      0,    170,      0,     56,      0,
    255,      0,    231,      0,    255,      0,    208,      0,
    139,      0,     92,      0,    115,      0,     92,      0,
    185,      0,     69,      0,     46,      0,     46,      0,
    162,      0,     23,      0,    208,      0,     46,      0,
  };
  const uint8_t node_4[] = {
    255,      0,     31,      0,     63,      0,     63,      0,
    127,      0,     95,      0,    191,      0,     63,      0,
    223,      0,     31,      0,    159,      0,     63,      0,
    31,      0,     63,      0,     95,      0,     31,      0,
    8,      0,      0,      0,     95,      0,     63,      0,
    255,      0,      0,      0,    127,      0,      0,      0,
    8,      0,      0,      0,    159,      0,     63,      0,
    255,      0,    223,      0,    191,      0,     31,      0,
    76,      0,     25,      0,    255,      0,    127,      0,
    153,      0,     51,      0,    204,      0,    102,      0,
    76,      0,     51,      0,    229,      0,    127,      0,
    153,      0,     51,      0,    178,      0,    102,      0,
  };
  const uint8_t node_5[] = {
    255,      0,     51,      0,     25,      0,     76,      0,
    0,      0,      0,      0,    102,      0,      0,      0,
    204,      0,    229,      0,      0,      0,    178,      0,
    0,      0,    153,      0,    127,      0,      8,      0,
    178,      0,    127,      0,    153,      0,    204,      0,
    255,      0,      0,      0,     25,      0,     76,      0,
    102,      0,     51,      0,      0,      0,      0,      0,
    229,      0,     25,      0,     25,      0,    204,      0,
    178,      0,    102,      0,    255,      0,     76,      0,
    127,      0,     76,      0,    229,      0,     76,      0,
    153,      0,    102,      0,    255,      0,     25,      0,
    127,      0,     51,      0,    204,      0,     51,      0,
  };
  const uint8_t node_6[] = {
    255,      0,      0,      0,    223,      0,      0,      0,
    31,      0,      8,      0,    127,      0,      0,      0,
    95,      0,      0,      0,    159,      0,      0,      0,
    95,      0,     63,      0,    191,      0,      0,      0,
    51,      0,    204,      0,      0,      0,    102,      0,
    255,      0,    127,      0,      8,      0,    178,      0,
    25,      0,    229,      0,      0,      0,     76,      0,
    204,      0,    153,      0,     51,      0,     25,      0,
    255,      0,    226,      0,    255,      0,    255,      0,
    198,      0,     28,      0,    141,      0,     56,      0,
    170,      0,     56,      0,     85,      0,     28,      0,
    170,      0,     28,      0,    113,      0,     56,      0,
  };
  const uint8_t node_7[] = {
    223,      0,      0,      0,     63,      0,      0,      0,
    95,      0,      0,      0,    223,      0,     31,      0,
    255,      0,      0,      0,    159,      0,      0,      0,
    127,      0,     31,      0,    191,      0,     31,      0,
    0,      0,      0,      0,    109,      0,      0,      0,
    218,      0,      0,      0,    182,      0,     72,      0,
    8,      0,     36,      0,    145,      0,     36,      0,
    255,      0,      8,      0,    182,      0,     72,      0,
    255,      0,     72,      0,    218,      0,     36,      0,
    218,      0,      0,      0,    145,      0,      0,      0,
    255,      0,     36,      0,    182,      0,     36,      0,
    182,      0,      0,      0,    109,      0,      0,      0,
  };
  const uint8_t node_8[] = {
    255,      0,      0,      0,    218,      0,      0,      0,
    36,      0,      0,      0,    218,      0,      0,      0,
    182,      0,    109,      0,    255,      0,      0,      0,
    0,      0,      0,      0,    145,      0,     72,      0,
    159,      0,      0,      0,     31,      0,    127,      0,
    255,      0,     31,      0,      0,      0,     95,      0,
    8,      0,      0,      0,    191,      0,     31,      0,
    255,      0,     31,      0,    223,      0,     63,      0,
    255,      0,     31,      0,     63,      0,     31,      0,
    95,      0,     31,      0,     63,      0,    127,      0,
    159,      0,     31,      0,     63,      0,     31,      0,
    223,      0,    223,      0,    191,      0,    191,      0,
  };
  const uint8_t node_9[] = {
    226,      0,     28,      0,     28,      0,    141,      0,
    8,      0,      8,      0,    255,      0,      8,      0,
    113,      0,     28,      0,    198,      0,     85,      0,
    56,      0,    198,      0,    170,      0,     28,      0,
    8,      0,     95,      0,      8,      0,      8,      0,
    255,      0,     63,      0,     31,      0,    223,      0,
    8,      0,     31,      0,    191,      0,      8,      0,
    255,      0,    127,      0,    127,      0,    159,      0,
    115,      0,     46,      0,    255,      0,    185,      0,
    139,      0,     23,      0,    208,      0,    115,      0,
    231,      0,     69,      0,    255,      0,    162,      0,
    139,      0,    115,      0,    231,      0,     92,      0,
  };
  const uint8_t node_10[] = {
    145,      0,      0,      0,      0,      0,    109,      0,
    0,      0,      0,      0,    255,      0,    109,      0,
    72,      0,    218,      0,      0,      0,      0,      0,
    36,      0,      0,      0,    182,      0,      0,      0,
    0,      0,    127,      0,    159,      0,    127,      0,
    159,      0,    191,      0,    223,      0,     63,      0,
    255,      0,     95,      0,     31,      0,     95,      0,
    31,      0,      8,      0,     63,      0,      8,      0,
    255,      0,      0,      0,    145,      0,      0,      0,
    182,      0,    109,      0,    109,      0,    109,      0,
    218,      0,      0,      0,     72,      0,      0,      0,
    182,      0,     72,      0,    182,      0,     36,      0,
  };
  const uint8_t node_11[] = {
    255,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    255,      0,      0,      0,    218,      0,     72,     36,
    0,      0,    182,      0,      0,      0,    145,    109,
    0,      0,    127,      0,      0,      0,     42,      0,
    212,      0,      0,    212,      0,      0,    212,      0,
    0,      0,      0,      0,     42,      0,      0,      0,
    255,      0,      0,      0,    170,    170,    127,     85,
    145,      0,    109,    109,    218,    109,     72,      0,
    145,      0,     72,      0,    218,      0,    109,      0,
    182,      0,    109,      0,    255,      0,     72,      0,
    182,    109,     36,    109,    255,    109,    109,      0,
  };
  const uint8_t node_12[] = {
    255,      0,      0,      0,    255,      0,    191,      0,
    0,      0,      0,      0,     95,      0,     63,      0,
    31,      0,      0,      0,    223,      0,    223,      0,
    0,      0,      8,      0,    159,      0,    127,      0,
    0,      0,     85,      0,     56,      0,     28,      0,
    255,      0,     28,      0,      0,      0,    226,      0,
    0,      0,    170,      0,     56,      0,    113,      0,
    198,      0,      0,      0,    113,      0,    141,      0,
    255,      0,     42,      0,    233,      0,     63,      0,
    212,      0,     85,      0,    191,      0,    106,      0,
    191,      0,     21,      0,    170,      0,      8,      0,
    170,      0,    127,      0,    148,      0,    148,      0,
  };
  const uint8_t node_13[] = {
    255,      0,      0,      0,      0,      0,     63,      0,
    191,      0,     95,      0,     31,      0,    223,      0,
    255,      0,     63,      0,     95,      0,     63,      0,
    159,      0,      0,      0,      0,      0,    127,      0,
    72,      0,      0,      0,      0,      0,      0,      0,
    255,      0,      0,      0,      0,      0,      0,      0,
    72,      0,     72,      0,     36,      0,      8,      0,
    218,      0,    182,      0,    145,      0,    109,      0,
    255,      0,    162,      0,    231,      0,    162,      0,
    231,      0,    115,      0,    208,      0,    139,      0,
    185,      0,     92,      0,    185,      0,     46,      0,
    162,      0,     69,      0,    162,      0,     23,      0,
  };
  const uint8_t node_14[] = {
    255,      0,      0,      0,     51,      0,      0,      0,
    0,      0,      0,      0,    102,      0,      0,      0,
    204,      0,      0,      0,    153,      0,      0,      0,
    0,      0,      0,      0,     51,      0,      0,      0,
    0,      0,      0,      0,      8,      0,     36,      0,
    255,      0,      0,      0,    182,      0,      8,      0,
    0,      0,      0,      0,     72,      0,    109,      0,
    145,      0,      0,      0,    255,      0,    218,      0,
    212,      0,      8,      0,    170,      0,      0,      0,
    127,      0,      0,      0,     85,      0,      8,      0,
    255,      0,      8,      0,    170,      0,      0,      0,
    127,      0,      0,      0,     42,      0,      8,      0,
  };
  const uint8_t node_15[] = {
    255,      0,      0,      0,      0,      0,      0,      0,
    36,      0,      0,      0,    182,      0,      0,      0,
    218,      0,      0,      0,      0,      0,      0,      0,
    72,      0,      0,      0,    145,      0,    109,      0,
    36,      0,     36,      0,      0,      0,      0,      0,
    255,      0,      0,      0,    182,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,    109,
    218,      0,      0,      0,    145,      0,     72,     72,
    255,      0,     28,      0,    226,      0,     56,      0,
    198,      0,      0,      0,      0,      0,     28,     28,
    170,      0,      0,      0,    141,      0,      0,      0,
    113,      0,      0,      0,     85,     85,     85,     85,
  };
  const uint8_t node_16[] = {
    255,      0,      0,      0,      0,      0,     95,      0,
    0,      0,    127,      0,      0,      0,      0,      0,
    223,      0,     95,      0,     63,      0,     31,      0,
    191,      0,      0,      0,    159,      0,      0,      0,
    0,      0,     31,      0,    255,      0,      0,      0,
    0,      0,     95,      0,    223,      0,      0,      0,
    0,      0,     63,      0,    191,      0,      0,      0,
    0,      0,      0,      0,    159,      0,    127,      0,
    141,      0,     28,      0,     28,      0,     28,      0,
    113,      0,      8,      0,      8,      0,      8,      0,
    255,      0,      0,      0,    226,      0,      0,      0,
    198,      0,     56,      0,    170,      0,     85,      0,
  };
  const uint8_t node_17[] = {
    255,      0,      0,      0,      8,      0,      0,      0,
    182,      0,      0,      0,     72,      0,      0,      0,
    218,      0,      0,      0,     36,      0,      0,      0,
    145,      0,      0,      0,    109,      0,      0,      0,
    0,      0,     51,     25,     76,     25,     25,      0,
    153,      0,      0,      0,    127,    102,    178,      0,
    204,      0,      0,      0,      0,      0,    255,      0,
    0,      0,    102,      0,    229,      0,     76,      0,
    113,      0,      0,      0,    141,      0,     85,      0,
    0,      0,      0,      0,    170,      0,      0,      0,
    56,     28,    255,      0,      0,      0,      0,      0,
    198,      0,      0,      0,    226,      0,      0,      0,
  };
  const uint8_t node_18[] = {
    255,      0,      8,      0,     28,      0,     28,      0,
    198,      0,     56,      0,     56,      0,     85,      0,
    255,      0,     85,      0,    113,      0,    113,      0,
    226,      0,    141,      0,    170,      0,    141,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    255,      0,      0,      0,    127,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    63,      0,      0,      0,    191,      0,      0,      0,
    255,      0,      0,      0,    255,      0,    127,      0,
    0,      0,     85,      0,      0,      0,    212,      0,
    0,      0,    212,      0,     42,      0,    170,      0,
    0,      0,    127,      0,      0,      0,      0,      0,
  };
  const uint8_t node_19[] = {
    255,      0,      0,      0,      0,      0,    218,      0,
    182,      0,      0,      0,      0,      0,    145,      0,
    145,      0,     36,      0,      0,      0,    109,      0,
    109,      0,      0,      0,     72,      0,     36,      0,
    0,      0,      0,      0,    109,      0,      8,      0,
    72,      0,      0,      0,    255,      0,    182,      0,
    0,      0,      0,      0,    145,      0,      8,      0,
    36,      0,      8,      0,    218,      0,    182,      0,
    255,      0,      0,      0,      0,      0,    226,      0,
    85,      0,      0,      0,    141,      0,      0,      0,
    0,      0,      0,      0,    170,      0,     56,      0,
    198,      0,      0,      0,    113,      0,     28,      0,
  };
  const uint8_t node_20[] = {
    255,      0,      0,      0,    113,      0,      0,      0,
    198,      0,     56,      0,     85,      0,     28,      0,
    255,      0,      0,      0,    226,      0,      0,      0,
    170,      0,      0,      0,    141,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    255,      0,    145,      0,    109,      0,    218,      0,
    36,      0,    182,      0,     72,      0,     72,      0,
    255,      0,      0,      0,      0,      0,    109,      0,
    36,      0,     36,      0,    145,      0,      0,      0,
    72,      0,     72,      0,    182,      0,      0,      0,
    72,      0,     72,      0,    218,      0,      0,      0,
    109,      0,    109,      0,    255,      0,      0,      0,
  };
  const uint8_t node_21[] = {
    255,      0,      0,      0,    218,      0,      0,      0,
    145,      0,      0,      0,     36,      0,      0,      0,
    218,      0,      0,      0,     36,      0,      0,      0,
    182,      0,     72,      0,      0,      0,    109,      0,
    0,      0,      0,      0,      8,      0,      0,      0,
    255,      0,     85,      0,    212,      0,     42,      0,
    0,      0,      0,      0,      8,      0,      0,      0,
    85,      0,    170,      0,    127,      0,     42,      0,
    109,      0,    109,      0,    255,      0,      0,      0,
    72,      0,     72,      0,    218,      0,      0,      0,
    145,      0,    182,      0,    255,      0,      0,      0,
    36,      0,     36,      0,    218,      0,      8,      0,
  };
  const uint8_t node_22[] = {
    255,      0,      0,      0,     42,      0,      0,      0,
    212,      0,      0,      0,      8,      0,    212,      0,
    170,      0,      0,      0,     85,      0,      0,      0,
    212,      0,      8,      0,    127,      0,      8,      0,
    255,      0,     85,      0,      0,      0,      0,      0,
    226,      0,     85,      0,      0,      0,    198,      0,
    0,      0,    141,      0,     56,      0,      0,      0,
    170,      0,     28,      0,      0,      0,    113,      0,
    113,      0,     56,      0,    255,      0,      0,      0,
    85,      0,     56,      0,    226,      0,      0,      0,
    0,      0,    170,      0,      0,      0,    141,      0,
    28,      0,     28,      0,    198,      0,     28,      0,
  };
  const uint8_t node_23[] = {
    255,      0,      0,      0,    229,      0,      0,      0,
    204,      0,    204,      0,      0,      0,     76,      0,
    178,      0,    153,      0,     51,      0,    178,      0,
    178,      0,    127,      0,    102,     51,     51,     25,
    0,      0,      0,      0,      0,      0,      0,     31,
    0,      0,      0,      0,    255,      0,      0,     31,
    0,      0,      8,      0,      0,      0,    191,    159,
    127,     95,     95,      0,    223,      0,     63,      0,
    255,      0,    255,      0,    204,    204,    204,    204,
    0,      0,     51,     51,     51,     51,      0,      0,
    204,      0,    204,      0,    153,    153,    153,    153,
    153,      0,      0,      0,    102,    102,    102,    102,
  };
  const uint8_t node_24[] = {
    170,      0,      0,      0,      0,    255,      0,      0,
    198,      0,      0,      0,      0,     28,      0,      0,
    141,      0,      0,      0,      0,    226,      0,      0,
    56,      0,      0,    113,      0,     85,      0,      0,
    255,      0,      0,      0,      0,    113,      0,      0,
    85,      0,      0,      0,      0,    226,      0,      0,
    141,      0,      0,      8,      0,    170,     56,     56,
    198,      0,      0,     56,      0,    141,     28,      0,
    255,      0,      0,      0,      0,    191,      0,      0,
    159,      0,      0,      0,      0,    223,      0,      0,
    95,      0,      0,      0,      0,     63,      0,      0,
    127,      0,      0,      0,      0,     31,      0,      0,
  };

  static const uint8_t *drum_map[5][5] =
      {
          {node_10, node_8, node_0, node_9, node_11},
          {node_15, node_7, node_13, node_12, node_6},
          {node_18, node_14, node_4, node_5, node_3},
          {node_23, node_16, node_21, node_1, node_2},
          {node_24, node_19, node_17, node_20, node_22},
  };

  GridsChannel::GridsChannel()
  {
  }

  uint8_t GridsChannel::U8Mix(uint8_t a, uint8_t b, uint8_t balance)
  {
      uint16_t mix = b * balance;
      mix += (a * (255 - balance));
      return mix / 255;
  }

  void GridsChannel::setStep(uint8_t step)
  {
      step_ = step;
  }

  uint8_t GridsChannel::level(int selector, uint16_t x, uint16_t y)
  {
      uint16_t xmap = x % 256;
      uint16_t ymap = y % 256;
      int part = selector % NumParts;
      return ReadDrumMap(step_, part, xmap, ymap);
  }

  /* static */
  uint8_t GridsChannel::ReadDrumMap(uint8_t step, uint8_t instrument, uint8_t x, uint8_t y)
  {
      uint8_t i = x >> 6;
      uint8_t j = y >> 6;
      const uint8_t *a_map = drum_map[i][j];
      const uint8_t *b_map = drum_map[i + 1][j];
      const uint8_t *c_map = drum_map[i][j + 1];
      const uint8_t *d_map = drum_map[i + 1][j + 1];
      uint8_t offset = (instrument * kStepsPerPattern) + step;
      uint8_t a = *(a_map + offset);
      uint8_t b = *(b_map + offset);
      uint8_t c = *(c_map + offset);
      uint8_t d = *(d_map + offset);
      return U8Mix(U8Mix(a, b, x << 2), U8Mix(c, d, x << 2), y << 2);
  }

  GridsWrapper::GridsWrapper()
  {
      tickCount_ = 0;
      for (auto i = 0; i < num_notes; i++)
      {
          channelTriggered_[i] = false;
          density_[i] = i == 0 ? 128 : 64;
          perturbations_[i] = 0;
          x_[i] = 128;
          y_[i] = 128;
      }

      accent = 128;
      chaos = 0;
      divider_ = 0;
      multiplier_ = 1;
      running_ = false;
  }

  uint32_t GridsWrapper::randomValue(uint32_t init)
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

  void GridsWrapper::start()
  {
      tickCount_ = 0;
      running_ = true;
  }

  void GridsWrapper::stop()
  {
      running_ = false;
  }

  void GridsWrapper::proceed()
  {
      running_ = true;
  }

  void GridsWrapper::gridsTick()
  {
      if (!running_)
          return;

      uint32_t ticksPerClock = 3 << divider_;
      bool trigger = ((tickCount_ % ticksPerClock) == 0);

      if (trigger)
      {
          const auto step = (tickCount_ / ticksPerClock * multiplier_) % grids::kStepsPerPattern;
          channel_.setStep(step);

          for (auto channel = 0; channel < num_notes; channel++)
          {
              if (step == 0)
              {
                  uint32_t r = randomValue();
                  perturbations_[channel] = ((r & 0xFF) * (chaos >> 2)) >> 8;
              }

              const uint8_t threshold = ~density_[channel];
              auto level = channel_.level(channel, x_[channel], y_[channel]);
              if (level < 255 - perturbations_[channel])
              {
                  level += perturbations_[channel];
              }

              if (level > threshold)
              {
                  uint8_t targetLevel = uint8_t(127.f * float(level - threshold) / float(256 - threshold));
                  uint8_t noteLevel = GridsChannel::U8Mix(127, targetLevel, accent);
                  MM::sendNoteOn(grids_notes[channel], noteLevel, midiChannel_);
                  channelTriggered_[channel] = true;
              }
          }
      }
      else
      {
          for (auto channel = 0; channel < num_notes; channel++)
          {
              if (channelTriggered_[channel])
              {
                  MM::sendNoteOff(grids_notes[channel], 0, midiChannel_);
                  channelTriggered_[channel] = false;
              }
          }
      }
      tickCount_++;
  }

  ChannelPatternLEDs GridsWrapper::getChannelLEDS(uint8_t channel)
  {
      ChannelPatternLEDs channelLeds;

      // uint8_t perturbs;

      for (int i = 0; i < 32; i++)
      {
          // const auto step = (i / ticksPerClock * multiplier_) % grids::kStepsPerPattern;
          const auto step = i;
          channel_.setStep(step);

          if (channel < num_notes)
          {

              // if (step == 0)
              // {
              //     uint32_t r = randomValue();
              //     perturbations_[channel] = ((r & 0xFF) * (chaos >> 2)) >> 8;
              // }

              const uint8_t threshold = ~density_[channel];
              auto level = channel_.level(channel, x_[channel], y_[channel]);
              if (level < 255 - perturbations_[channel])
              {
                  level += perturbations_[channel];
              }

              if (level > threshold)
              {
                  uint8_t targetLevel = uint8_t(127.f * float(level - threshold) / float(256 - threshold));
                  uint8_t noteLevel = GridsChannel::U8Mix(127, targetLevel, accent);
                  channelLeds.levels[i] = noteLevel;
                  // MM::sendNoteOn(grids_notes[channel], noteLevel, midiChannel_);
                  // channelTriggered_[channel] = true;
              }
              else
              {
                  channelLeds.levels[i] = 0;
              }
          }
      }

      return channelLeds;
  }

  uint8_t GridsWrapper::getSeqPos()
  {
      uint32_t ticksPerClock = 3 << divider_;
      uint8_t step = (tickCount_ / ticksPerClock * multiplier_) % grids::kStepsPerPattern;
      return step;
  }

  void GridsWrapper::setDensity(uint8_t channel, uint8_t density)
  {
      density_[channel] = density;
  }

  uint8_t GridsWrapper::getDensity(uint8_t channel)
  {
      return density_[channel];
  }

  void GridsWrapper::setX(uint8_t channel, uint8_t x)
  {
      x_[channel] = x;
      // 			Serial.print("setX:");
      // 			Serial.print(channel);
      // 			Serial.print(":");
      // 			Serial.println(x);
  }

  uint8_t GridsWrapper::getX(uint8_t channel)
  {
      return x_[channel];
  }

  void GridsWrapper::setY(uint8_t channel, uint8_t y)
  {
      y_[channel] = y;
  }

  uint8_t GridsWrapper::getY(uint8_t channel)
  {
      return y_[channel];
  }

  void GridsWrapper::setChaos(uint8_t c)
  {
      chaos = c;
  }

  uint8_t GridsWrapper::getChaos()
  {
      return chaos;
  }

  void GridsWrapper::setResolution(uint8_t r)
  {
      divider_ = 0;
      if (r == 0)
      {
          multiplier_ = 1;
          divider_ = 1;
      }
      else if (r == 1)
      {
          multiplier_ = 1;
      }
      else if (r == 2)
      {
          multiplier_ = 2;
          //     } else if (r == 3){
          //     	multiplier_ = 4;
      }
  }

  void GridsWrapper::setAccent(uint8_t a)
  {
      accent = a;
  }
  uint8_t GridsWrapper::getAccent()
  {
      return accent;
  }
}