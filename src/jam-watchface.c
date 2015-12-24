#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static BitmapLayer *s_jam_layer;
static GBitmap *s_jam_bitmap;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char time_buffer[8];

  strftime(
    time_buffer,
    sizeof(time_buffer),
    clock_is_24h_style() ? "%H:%M" : "%I:%M",
    tick_time
  );

  text_layer_set_text(s_time_layer, time_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create Jam image
  s_jam_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JAM);
  s_jam_layer = bitmap_layer_create(
    GRect((bounds.size.w / 2) - 24, 24, 48, 48)
  );
  bitmap_layer_set_compositing_mode(s_jam_layer, GCompOpSet);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_jam_layer, s_jam_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_jam_layer));

  s_time_layer = text_layer_create(
    GRect(0, 24 + 42 + 10, bounds.size.w, 50)
  );

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  gbitmap_destroy(s_jam_bitmap);
  bitmap_layer_destroy(s_jam_layer);
}

static void init(void) {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, (GColor8)(GColorFromHEX(0xeb685b)));

  // Show the Window with animated=true
  window_stack_push(s_main_window, true);

  // We can display time now
  update_time();

  // Register handler
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
	window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
