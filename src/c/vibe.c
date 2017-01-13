#include <pebble.h>
#include "vibe.h"


void vibe_short() {
  static const uint32_t seg[] = { 100 };
  static VibePattern pat = {
      .durations = seg,
      .num_segments = ARRAY_LENGTH(seg),
  };
  vibes_enqueue_custom_pattern(pat);
}

void vibe_double_short() {
  static const uint32_t seg[] = { 100, 200, 100 };
  static VibePattern pat = {
      .durations = seg,
      .num_segments = ARRAY_LENGTH(seg),
  };
  vibes_enqueue_custom_pattern(pat);
}

void vibe_tripple_short() {
  static const uint32_t seg[] = { 100, 200, 100, 200, 100 };
  static VibePattern pat = {
      .durations = seg,
      .num_segments = ARRAY_LENGTH(seg),
  };
  vibes_enqueue_custom_pattern(pat);
}

void vibe_long_long() {
  static const uint32_t seg[] = { 300, 300, 100 };
  static VibePattern pat = {
      .durations = seg,
      .num_segments = ARRAY_LENGTH(seg),
  };
  vibes_enqueue_custom_pattern(pat);
}


