#include "simple_analog.h"

#include "pebble.h"
#include "vibe.h"
#include "steps.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;
static TextLayer *s_step_layer;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_num_buffer[4], s_day_buffer[6];
static char s_current_steps_buffer[16];
static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;

static bool second_ticks = 1, tick_vibrate = 1;

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
    const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
    gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
    gpath_draw_filled(ctx, s_tick_paths[i]);
  }
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  if (second_ticks) {
  // seconds hand
    GPoint center = grect_center_point(&bounds);
  
    const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2);
    int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    GPoint second_hand = {
      .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
    };
  
    // second hand
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, second_hand, center);
  // ---
  }
  
  // minute/hour hand
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);

  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
  text_layer_set_text(s_num_label, s_num_buffer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));

  if (tick_vibrate && tick_time->tm_sec == 0) {
//     if (tick_time->tm_min % 30 == 0) {
//       vibe_long_long();
//     } else
    if (tick_time->tm_min % 15 == 0) {
      vibe_tripple_short();
    } else if (tick_time->tm_min % 5 == 0) {
      vibe_double_short();
    } else {
      vibe_short();
    }
  } // sec tick
}

static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
//   static char s_emoji[5];

  if(s_step_count >= s_step_average) {
    text_layer_set_text_color(s_step_layer, GColorJaegerGreen);
//     snprintf(s_emoji, sizeof(s_emoji), "\U0001F60C");
  } else {
    text_layer_set_text_color(s_step_layer, GColorPictonBlue);
//     snprintf(s_emoji, sizeof(s_emoji), "\U0001F4A9");
  }

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d,%03d", thousands, hundreds); //, s_emoji); // "%d,%03d %s"
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d", hundreds); //, s_emoji); // "%d %s"
  }

  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventSignificantUpdate) {
//     get_step_goal();
  }
  if (event != HealthEventSleepUpdate) {
    get_step_count();
    display_step_count();
  }
}

static void date_below() {
  layer_set_frame(text_layer_get_layer(s_day_label), PBL_IF_ROUND_ELSE(
    GRect(63, 114, 27, 20),
    GRect(46, 114, 27, 20)
  ));
  layer_set_frame(text_layer_get_layer(s_num_label), PBL_IF_ROUND_ELSE(
    GRect(90, 114, 18, 20),
    GRect(73, 114, 18, 20)
  ));
}
static void date_above() {
  layer_set_frame(text_layer_get_layer(s_day_label), PBL_IF_ROUND_ELSE(
    GRect(63, 44, 27, 20),
    GRect(46, 44, 27, 20)
  ));
  layer_set_frame(text_layer_get_layer(s_num_label), PBL_IF_ROUND_ELSE(
    GRect(90, 44, 18, 20),
    GRect(73, 44, 18, 20)
  ));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);

  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);

  s_day_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(63, 114, 27, 20),
    GRect(46, 114, 27, 20)));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(90, 114, 18, 20),
    GRect(73, 114, 18, 20)));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorBlack);
  text_layer_set_text_color(s_num_label, GColorWhite);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

  date_above();
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
  
  // Create a layer to hold the current step count
  s_step_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(104, 114), bounds.size.w, 38));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_font(s_step_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));

  // Subscribe to health events if we can
  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }
}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);

  layer_destroy(s_hands_layer);
}

static void msg_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read boolean preferences
  Tuple *second_tick_t = dict_find(iter, MESSAGE_KEY_SecondTick);
  if (second_tick_t) {
    second_ticks = second_tick_t->value->int32 == 1;
  }

  Tuple *tick_vibrate_t = dict_find(iter, MESSAGE_KEY_TickVibrate);
  if (tick_vibrate_t) {
    tick_vibrate = tick_vibrate_t->value->int32 == 1;
  }
  
  if (second_ticks) {
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);  
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick); 
  }
}

static void init() {
  
  // Open AppMessage connection
  app_message_register_inbox_received(msg_inbox_received_handler);
  app_message_open(128, 128);
  
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  s_day_buffer[0] = '\0';
  s_num_buffer[0] = '\0';

  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
  }

  if (second_ticks) {
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);  
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick); 
  }
  
}

static void deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }

  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
