# OMX-27 Default Pot CC Mapping and HRCC Compatibility

## Summary

This note explains why changing the default knob CC assignments is recommended for DAW interoperability.

The current firmware sends standard 7-bit MIDI CC messages for knob movement. However, several default CC numbers used by pot banks fall into MIDI ranges that many tools and DAWs interpret as part of 14-bit CC pairs ("high-resolution CC" / HRCC). This can cause confusing behavior in MIDI monitoring and, in some hosts, unreliable MIDI learn.

## What Is Happening

OMX-27 pot movement is sent as standard `Control Change` messages (single CC, 0-127), not intentional 14-bit messages.

In MIDI, controllers are organized as paired ranges:

- MSB controllers: `0-31`
- LSB controllers: `32-63` (paired as `MSB + 32`)

Examples of pairs:

- `22` (MSB) pairs with `54` (LSB)
- `21` (MSB) pairs with `53` (LSB)
- `23` (MSB) pairs with `55` (LSB)

Several existing OMX defaults include values in these paired ranges (for example `21-24`, `50-57`). If a monitor/host sees related values over time, it may reconstruct and display HRCC values even though OMX is only sending regular CC.

## Why This Matters

Depending on the DAW or MIDI middleware, paired-range CCs can lead to:

- MIDI learn binding to 14-bit interpretation unexpectedly
- Learn capturing the paired controller semantics instead of simple 7-bit CC behavior
- Parameter jitter/jumps or confusing scaling when stale pair data is combined
- Inconsistent behavior between DAWs (one host works fine, another is problematic)

This issue appears most often in controller-mapping workflows and monitor tools that infer HRCC from CC number ranges.

## Recommendation

For maximum compatibility, avoid using default knob CCs in `0-63` where 14-bit pair semantics are common.

A safer default strategy is to assign pot CCs in `64-95` (or another non-paired range that does not conflict with transport/meta controls).

This does not remove user flexibility; users can still manually set any CC they need. The goal is simply to make out-of-box defaults more reliable across DAWs.

## Important Clarification

The observed HRCC readouts do not necessarily mean OMX is transmitting explicit 14-bit CC pairs by design. The behavior is primarily a compatibility/interpretation artifact of chosen CC numbers and host-side parsing.
