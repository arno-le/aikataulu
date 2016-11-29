#include <pebble.h>
#include "main.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static GFont s_date_font;
static TextLayer *s_date_layer;

//Comms
static AppTimer *s_timeout_timer; 
static bool s_js_ready;


//Bitmaps
static BitmapLayer *s_arrival_layer;
static BitmapLayer *s_departure_layer;
static GBitmap *s_arrival;
static GBitmap *s_departure;

//JS-textfelder
static TextLayer *s_nextbus;
static TextLayer *s_nextbushome;
static GFont s_bus_font;

//cONFIG
static bool twenty_four_hour_format = true;
static char time_to_departure[3] = {1,5};
static char stop1[5] = {0, 0, 0, 1};
static char stop2[5] = {0, 0, 0, 2};
static char line1[3];
static char line2[3];
static int refresh_rate = 30;

bool comm_is_js_ready() {
  return s_js_ready;
}

static void no_internet() {
  text_layer_set_text(s_nextbus, "Yhteys-");
  text_layer_set_text(s_nextbushome, "ongelma.");
}
static void update_time() {

  time_t temp = time(NULL);
  struct tm *tick_time = localtime (&temp);
  static char s_buffer[8];
  static char date_buffer[10];
  
  //Time
  if (clock_is_24h_style() == twenty_four_hour_format) {
    strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);
  } else {
    strftime(s_buffer, sizeof(s_buffer), "%I:%M", tick_time);

  }
  //Date
  text_layer_set_text(s_time_layer, s_buffer);
  strftime(date_buffer, sizeof(s_buffer), "%d. %b", tick_time);
  text_layer_set_text(s_date_layer, date_buffer);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  //Refresh data 
  if(tick_time->tm_min % refresh_rate == 0) {
      reload_with_timeout();
  }
}

void reload_with_timeout() {

  DictionaryIterator *iter;
  if(app_message_outbox_begin(&iter) == APP_MSG_OK) {
    dict_write_cstring(iter, KEY_STOP1, stop1);
    dict_write_cstring(iter, KEY_STOP2, stop2);
    dict_write_cstring(iter, KEY_LINE1, line1);
    dict_write_cstring(iter, KEY_LINE2, line2);
    dict_write_cstring(iter, KEY_TIME_TO_DEPARTURE, time_to_departure);
    app_message_outbox_send();
  }
  text_layer_set_text(s_nextbus, "Ladataan...");
  text_layer_set_text(s_nextbushome, "Ladataan...");
  // Schedule the timeout timer
  const int interval_ms = 5000;
  s_timeout_timer = app_timer_register(interval_ms, timeout_timer_handler, NULL);
}

void timeout_timer_handler(void *context) {
  // The timer elapsed because no success was reported
    text_layer_set_text(s_nextbus, "Yhteysvirhe...");
    text_layer_set_text(s_nextbushome, "Retrying...");
  // Retry the message
    reload_with_timeout();
}

static void main_window_load(Window *window) {
  //Variabeln
  if (persist_read_string(KEY_TIME_TO_DEPARTURE, time_to_departure, sizeof(time_to_departure))) {
    persist_read_string(KEY_TIME_TO_DEPARTURE, time_to_departure, sizeof(time_to_departure));
  }

  if (persist_read_bool(KEY_TWENTY_FOUR_HOUR_FORMAT)) {
    twenty_four_hour_format = persist_read_bool(KEY_TWENTY_FOUR_HOUR_FORMAT);
  }
  if (persist_read_string(KEY_STOP1, stop1, sizeof(stop1))) {
    persist_read_string(KEY_STOP1, stop1, sizeof(stop1));
  }
  if (persist_read_string(KEY_STOP2, stop2, sizeof(stop2))) {
    persist_read_string(KEY_STOP2, stop2, sizeof(stop2));
  }
  if (persist_read_string(KEY_LINE1, line1, sizeof(line1))) {
    persist_read_string(KEY_LINE1, line1, sizeof(line1));
  }
  if (persist_read_string(KEY_LINE2, line2, sizeof(line2))) {
    persist_read_string(KEY_LINE2, line2, sizeof(line2));
  }
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop1: %s", stop1);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop2: %s", stop2);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "TimetoDep: %s", time_to_departure);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Line1: %s", line1);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Line2: %s", line2);
   //Holt die Fenster-Info
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds (window_layer);
  
  //Erstellung der Bilder
  s_arrival = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS_ARRIVAL);
  s_departure = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS_DEPARTURE);
  
  s_arrival_layer = bitmap_layer_create( GRect(7, PBL_IF_ROUND_ELSE(135, 135), 42 , 28));
  s_departure_layer = bitmap_layer_create( GRect(7, PBL_IF_ROUND_ELSE(110, 105), 42 , 28));
  
  bitmap_layer_set_bitmap(s_arrival_layer, s_arrival);
  bitmap_layer_set_bitmap(s_departure_layer, s_departure);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(s_departure_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_arrival_layer));
 

  //Erstellung des Textes
  //Zeit
  s_time_layer = text_layer_create (GRect(0, PBL_IF_ROUND_ELSE(58, 0), bounds.size.w, 60));
  //Datum
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(50,62), bounds.size.w,35));

  //Die Schrift festzustellen
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALDO_50));
  s_bus_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OBELUS_32));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALDO_26));
  
  //Textfelder anpassen
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  
  s_nextbus = text_layer_create( GRect( 55 , PBL_IF_ROUND_ELSE(135, 93), 80 , 35));
  s_nextbushome = text_layer_create( GRect( 55 , PBL_IF_ROUND_ELSE(135, 123), 80 , 35));
  
  
  text_layer_set_background_color(s_nextbus, GColorClear);
  text_layer_set_background_color(s_nextbushome, GColorClear);
  text_layer_set_font(s_nextbus, s_bus_font);
  text_layer_set_font(s_nextbushome, s_bus_font);
  text_layer_set_text_color(s_nextbus, GColorWhite);
  text_layer_set_text_color(s_nextbushome, GColorWhite);
  text_layer_set_text_alignment(s_nextbus, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_nextbushome, GTextAlignmentCenter);
  text_layer_set_text(s_nextbus, "Ladataan...");
  text_layer_set_text(s_nextbushome, "Ladataan...");
  

  layer_add_child(window_layer, text_layer_get_layer(s_nextbus));
  layer_add_child(window_layer, text_layer_get_layer(s_nextbushome));
  
 
  
}

static void main_window_unload(Window *window) {
  //Textlayers und Schrift abbauen
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_nextbus);
  text_layer_destroy(s_nextbushome);
  
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_bus_font);
  fonts_unload_custom_font(s_date_font);
  
  //Bitmaps
  gbitmap_destroy(s_arrival);
  gbitmap_destroy(s_departure);
  bitmap_layer_destroy(s_arrival_layer);
  bitmap_layer_destroy(s_departure_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    
  //JS-ready
  Tuple *ready_tuple = dict_find(iterator, KEY_JS_READY);
  if(ready_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Js-ready");
    s_js_ready = ready_tuple->value->int8;
    reload_with_timeout();
  }
  
  //Nointernet
  
  Tuple *internet_t = dict_find(iterator, KEY_INTERNET);
  if(internet_t) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No internet");
    no_internet();
  }
  //Config
  Tuple *twenty_four_hour_format_t = dict_find(iterator, KEY_TWENTY_FOUR_HOUR_FORMAT);
  Tuple *stop1_tuple = dict_find(iterator, KEY_STOP1);
  Tuple *stop2_tuple = dict_find(iterator, KEY_STOP2);
  Tuple *time_to_departure_t = dict_find(iterator, KEY_TIME_TO_DEPARTURE);
  Tuple *refresh_rate_t = dict_find(iterator, KEY_REFRESH_RATE);
  Tuple *line1_t = dict_find(iterator, KEY_LINE1);
  Tuple *line2_t = dict_find(iterator, KEY_LINE2);
  
  if (twenty_four_hour_format_t) {
    twenty_four_hour_format = twenty_four_hour_format_t->value->int8;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time format now %d", twenty_four_hour_format);
    persist_write_bool(KEY_TWENTY_FOUR_HOUR_FORMAT, twenty_four_hour_format);
    update_time();
  }
  if(time_to_departure_t) {
    snprintf(time_to_departure, sizeof(time_to_departure), "%s", time_to_departure_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time to departure %s", time_to_departure);
    persist_write_string(KEY_TIME_TO_DEPARTURE,time_to_departure);
  }
  if(stop1_tuple) {
    snprintf(stop1, sizeof(stop1), "%s", stop1_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop1 now %s", stop1);
    persist_write_string(KEY_STOP1,stop1);
  }
  if(stop2_tuple) {
    snprintf(stop2, sizeof(stop2), "%s", stop2_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Stop2 now %s", stop2);
    persist_write_string(KEY_STOP2,stop2);
  }
  if(line1_t) {
    snprintf(line1, sizeof(line1), "%s", line1_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Line1 now %s", line1);
    persist_write_string(KEY_LINE1,line1);
  }
  if(line2_t) {
    snprintf(line2, sizeof(line2), "%s", line2_t->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Line2 now %s", line2);
    persist_write_string(KEY_LINE2,line2);
  }
  
  if(refresh_rate_t) {
   static char refresh_rate_buffer[3];
   snprintf(refresh_rate_buffer, sizeof(refresh_rate_buffer), "%s", refresh_rate_t->value->cstring);

    int onedigit = refresh_rate_buffer[1] - '0';
    int tendigit = refresh_rate_buffer[0] - '0';
    int converter = tendigit*10 + onedigit;
    refresh_rate = converter;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Refreshrate saved %d", refresh_rate);
    persist_write_int (KEY_REFRESH_RATE, refresh_rate);
  }
   
  if(time_to_departure_t || stop1_tuple || stop2_tuple || line1_t || line2_t) {
    reload_with_timeout();
  }
  

  
  
  //Timetable
  static char line_buffer[8];
  static char time_buffer[8];
  static char timetable_text_buffer[24];
  
  Tuple *line_tuple = dict_find(iterator, KEY_CODE1);
  Tuple *time_tuple = dict_find(iterator, KEY_TIME1);

  
  if(time_tuple) {
    snprintf(line_buffer, sizeof(line_buffer), "%s", line_tuple->value->cstring);
    snprintf(time_buffer, sizeof(time_buffer), "%s", time_tuple->value->cstring);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Printing text");
    snprintf(timetable_text_buffer,sizeof(timetable_text_buffer), "%s : %s",line_buffer,time_buffer);
    text_layer_set_text(s_nextbus, timetable_text_buffer);
  } 
  
  // Zweite Zeile, faul
   static char line_buffer2[8];
   static char time_buffer2[8];
   static char timetable_text_buffer2[24];
   Tuple *line_tuple2  = dict_find(iterator, KEY_CODE2);
   Tuple *time_tuple2 = dict_find(iterator, KEY_TIME2);
  
  if(time_tuple2) {
    snprintf(line_buffer2, sizeof(line_buffer2), "%s", line_tuple2->value->cstring);
    snprintf(time_buffer2, sizeof(time_buffer2), "%s", time_tuple2->value->cstring);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Printing text");
    snprintf(timetable_text_buffer2,sizeof(timetable_text_buffer2), "%s : %s", line_buffer2,time_buffer2);
    text_layer_set_text(s_nextbushome, timetable_text_buffer2);
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
  if(s_timeout_timer) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cancelling timer.");
    app_timer_cancel(s_timeout_timer);
    }
}

static void init() {
  //Erstellt ein neues fenster
  s_main_window = window_create();
  
  window_set_background_color(s_main_window, GColorBlack);
  // Legt der Handler fest um die Fensterelemente zu steuern
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
    });
 
  //Zeigt das Fenster auf der Anzeige, animated = true
  window_stack_push(s_main_window, true);
  
  //Um festzustellen, dass die Zeit von Anfang an gezeigt wird
  update_time();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 256;
  app_message_open(inbox_size, outbox_size);
  
}

static void deinit () {
  
  tick_timer_service_unsubscribe();
  //Fenster zerstören
  window_destroy(s_main_window);
}


int main (void) {
  init();
  app_event_loop();
  deinit();
}