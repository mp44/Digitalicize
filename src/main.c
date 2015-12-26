#include <pebble.h>

#define KEY_TEMPERATURE 1
#define KEY_CONDITIONS 2
#define KEY_CELSIUS 3
#define KEY_USE_CELSIUS 4

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_weather_layer, *s_bt_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_time_font, s_date_font, s_weather_font, s_bt_font;
static int s_battery_level;
static Layer *s_battery_layer;
static Layer *s_battery_layer2;
static bool use_celsius = 0;

// Bluetooth Status
static void bluetooth_callback(bool connected){
  // If bluetooth disconnected
  layer_set_hidden(text_layer_get_layer(s_bt_layer), connected);
  
  if(!connected){
    vibes_double_pulse();
  }
  
}

// Battery Status
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

// Battery UI
static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.size.w - width, 0, width, bounds.size.h), 0, GCornerNone);
}

static void battery_update_proc2(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}



static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H.%M" : "%I.%M", tick_time);
  // Write the current date into a buffer
  static char date_buffer[10];
  strftime(date_buffer, sizeof(date_buffer), "%e", tick_time);
  text_layer_set_text(s_date_layer, date_buffer);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create temperature Layer
  s_weather_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(32, 32), bounds.size.w, 25));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "...");
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  
  //Create BitmapLayer to display GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Create second custom font, apply it and add to Window
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGII_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  // Create the time TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));


  // Create the date TextLayer with specific bounds
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(120, 109), bounds.size.w, 25));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Create the bluetooth TextLayer with specific bounds
  s_bt_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(145, 134), bounds.size.w, 25));
  text_layer_set_text_color(s_bt_layer, GColorWhite);
  text_layer_set_background_color(s_bt_layer, GColorClear);
  text_layer_set_text(s_bt_layer, "x");
  text_layer_set_text_alignment(s_bt_layer, GTextAlignmentCenter);
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGII_48));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGII_20));
  s_bt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGII_20));


  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_font(s_bt_layer, s_bt_font);


  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bt_layer));
  
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(0, PBL_IF_ROUND_ELSE(66, 58), bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  s_battery_layer2 = layer_create(GRect(0, PBL_IF_ROUND_ELSE(114, 108), bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer2, battery_update_proc2);


  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  layer_add_child(window_get_root_layer(window), s_battery_layer2);
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  if(persist_exists(KEY_USE_CELSIUS)) {
  	use_celsius = persist_read_int(KEY_USE_CELSIUS);
  }


}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_bt_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
  
  // Destroy Battery Layer
  layer_destroy(s_battery_layer);
  layer_destroy(s_battery_layer2);

}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char ctemperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read tuples for data
  APP_LOG(APP_LOG_LEVEL_INFO, "reading tuples");
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *ctemp_tuple = dict_find(iterator, KEY_CELSIUS);
  Tuple *use_celsius_t = dict_find(iterator, KEY_USE_CELSIUS);

  if(use_celsius_t) {
  	APP_LOG(APP_LOG_LEVEL_INFO, "KEY_USECELSIUS received!");

  	use_celsius = use_celsius_t->value->int8;

    APP_LOG(APP_LOG_LEVEL_INFO, "use_celsius converted!");
  	persist_write_int(KEY_USE_CELSIUS, use_celsius);
  }
  
  if(ctemp_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "ctemp_tuple true");
  }
  
  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "temptuple got fahrenheit info");
    
  }
  if(ctemp_tuple && conditions_tuple) {
    snprintf(ctemperature_buffer, sizeof(ctemperature_buffer), "%dC", (int)ctemp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "ctemptuple got celsius info");
  }
  if(use_celsius == 0)
  {
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s.%s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    APP_LOG(APP_LOG_LEVEL_INFO, "fahrenheit!");
  }
  else if (use_celsius == 1)
  {
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s.%s", ctemperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    APP_LOG(APP_LOG_LEVEL_INFO, "celsius!");
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

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Set Background to match Bitmap
  window_set_background_color(s_main_window, GColorBlack);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
 
  // Register callbacks pt.2
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
 

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}