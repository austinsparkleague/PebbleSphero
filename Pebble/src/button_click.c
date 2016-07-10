#include <pebble.h>
#include <math.h>

//define app keys
#define KEY_BUTTON_UP   0
#define KEY_BUTTON_DOWN 1
#define KEY_BUTTON_SELECT 2
#define CYCLE_COLOR 3

//setup
static Window *s_main_window;
static TextLayer *s_output_layer;
static bool wasLastSentMessageAnX;
static TextLayer *s_textlayer_1;
static TextLayer *s_textlayer_2;

static void send(int key, int value) {
  // Create dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add value to send
  dict_write_int(iter, key, &value, sizeof(int), true);

  // Send dictionary
  app_message_outbox_send();
}

//main window setup
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const int text_height = 60;
  const GEdgeInsets text_insets = GEdgeInsets((bounds.size.h - text_height) / 2, 5);

  s_output_layer = text_layer_create(grect_inset(bounds, text_insets));
  text_layer_set_text(s_output_layer, "Tilt your watch \n to steer");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)  {
    send(CYCLE_COLOR, 0);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    send(KEY_BUTTON_SELECT, 0);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_output_layer);
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  text_layer_set_text(s_output_layer, "Connected successfully");
}

static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  text_layer_set_text(s_output_layer, "Uh oh! Make sure you have launched the iOS app!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Fail reason: %d", (int)reason);
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;

  if(!did_vibrate) {
    // Print it out
    if(!(x == 0 && y == 0)) {
       //APP_LOG(APP_LOG_LEVEL_INFO, "%d %d", x, y);
      
      //alternate between sending x and y values
      if(wasLastSentMessageAnX) {
        send(KEY_BUTTON_DOWN, y);
        wasLastSentMessageAnX = false;
      } else {
        send(KEY_BUTTON_UP, x);
        wasLastSentMessageAnX = true;
      }
    } 
      
  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}

static void init(void) {
  //initialize windows
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

    // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(22, 15, 100, 24));
  text_layer_set_text(s_textlayer_1, "Cycle Color -->");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  //text_layer_set_font(s_textlayer_1, s_res_gothic_18);
  layer_add_child(window_get_root_layer(s_main_window), (Layer *)s_textlayer_1);
  
  // s_textlayer_2
  s_textlayer_2 = text_layer_create(GRect(12, 130, 120, 20));
  text_layer_set_text(s_textlayer_2, "Recalibrate Back -->");
  text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
  //text_layer_set_font(s_textlayer_2, s_res_gothic_18);
  layer_add_child(window_get_root_layer(s_main_window), (Layer *)s_textlayer_2);
  // Subscribe to batched data events
  uint32_t num_samples = 3;  // Number of samples per batch/callback
  accel_data_service_subscribe(num_samples, accel_data_handler);
  
  // sub to click providers
  window_set_click_config_provider(s_main_window, click_config_provider);
  
  //push window
  window_stack_push(s_main_window, true);
  
  // Open AppMessage and register callbacks
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  const int inbox_size = 128;  
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  }

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}