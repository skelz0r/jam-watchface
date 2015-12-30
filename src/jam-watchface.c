#include <pebble.h>

#define KEY_USERS 0
#define KEY_REQUESTS 1
#define KEY_MESSAGES 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_metric_layer;
static TextLayer *s_icons_layer;
static BitmapLayer *s_jam_layer;
static GBitmap *s_jam_bitmap;

static GFont s_fa_font;

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

  if (tick_time->tm_min % 20 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    dict_write_uint8(iter, 0, 0);

    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create Jam image
  s_jam_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JAM);
  s_jam_layer = bitmap_layer_create(
    GRect((bounds.size.w / 2) - 24, 20, 48, 48)
  );
  bitmap_layer_set_compositing_mode(s_jam_layer, GCompOpSet);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_jam_layer, s_jam_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_jam_layer));

  s_time_layer = text_layer_create(
    GRect(0, 20 + 45, bounds.size.w, 50)
  );

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_metric_layer = text_layer_create(
    GRect(0, bounds.size.h - 44, bounds.size.w, 20)
  );

  text_layer_set_background_color(s_metric_layer, GColorClear);
  text_layer_set_text_color(s_metric_layer, GColorWhite);
  text_layer_set_font(s_metric_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_metric_layer, GTextAlignmentCenter);

  text_layer_set_text(s_metric_layer, "jam");

  layer_add_child(window_layer, text_layer_get_layer(s_metric_layer));

  s_icons_layer = text_layer_create(
    GRect(0, bounds.size.h - 64, bounds.size.w, 20)
  );

  s_fa_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AWESOME_18));

  text_layer_set_background_color(s_icons_layer, GColorClear);
  text_layer_set_text_color(s_icons_layer, GColorWhite);
  text_layer_set_font(s_icons_layer, s_fa_font);
  text_layer_set_text_alignment(s_icons_layer, GTextAlignmentCenter);

  text_layer_set_text(s_icons_layer, "\uf0c0    \uf086    \uf075 ");

  layer_add_child(window_layer, text_layer_get_layer(s_icons_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_metric_layer);
  gbitmap_destroy(s_jam_bitmap);
  bitmap_layer_destroy(s_jam_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char users_count_buffer[9];
  static char requests_count_buffer[9];
  static char messages_count_buffer[9];
  static char metrics_text_buffer[3*9];

  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received");

  Tuple *users_count_tuple = dict_find(iterator, KEY_USERS);
  Tuple *requests_count_tuple = dict_find(iterator, KEY_REQUESTS);
  Tuple *messages_count_tuple = dict_find(iterator, KEY_MESSAGES);

  if (users_count_tuple && requests_count_tuple && messages_count_tuple) {
    snprintf(
      users_count_buffer,
      sizeof(users_count_buffer),
      "%d",
      (int)users_count_tuple->value->int32
    );

    snprintf(
      requests_count_buffer,
      sizeof(requests_count_buffer),
      "%d",
      (int)requests_count_tuple->value->int32
    );

    snprintf(
      messages_count_buffer,
      sizeof(messages_count_buffer),
      "%d",
      (int)messages_count_tuple->value->int32
    );

    snprintf(
      metrics_text_buffer,
      sizeof(metrics_text_buffer),
      "%d   %d   %d",
      (int)users_count_tuple->value->int32,
      (int)requests_count_tuple->value->int32,
      (int)messages_count_tuple->value->int32
    );

    text_layer_set_text(s_metric_layer, metrics_text_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage service
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
	window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
