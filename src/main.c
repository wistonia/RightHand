#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
static int mins_to_show_seconds = 0;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current time and date into buffers
  static char s_time_buffer[9];
	static char s_date_buffer[11];
	strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                               "%H\n%M\n%S" : "%l\n%M\n%S", tick_time);
	strftime(s_date_buffer, sizeof(s_date_buffer), "%a %e\n%b", tick_time);
	
  if(mins_to_show_seconds > 0) {
		layer_set_hidden((Layer *)date_layer, true);
	}	else {
		layer_set_hidden((Layer *)date_layer, false);	
	}

  // Display the buffers on the TextLayers
  text_layer_set_text(time_layer, s_time_buffer);
	text_layer_set_text(date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if ((units_changed & MINUTE_UNIT) != 0) {
		if(!--mins_to_show_seconds) {
			tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
		}
	}
	update_time();
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
	if (mins_to_show_seconds != 0) {
		tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);	
		mins_to_show_seconds = 0;
		update_time();
	} else {
		tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
		mins_to_show_seconds = 2;
		update_time();
	}
}

void handle_init(void) {
  my_window = window_create();
	Layer *window_layer = window_get_root_layer(my_window);
	GRect bounds = layer_get_bounds(window_layer);
  time_layer = text_layer_create(bounds);

	// Size date layer 5/12 of display
	bounds.size.h = (bounds.size.h * 7) / 12;
	bounds.origin.y = bounds.size.h;
	bounds.size.h = (bounds.size.h * 5) / 7;
	date_layer = text_layer_create(bounds);
	
	// Configure layers
	text_layer_set_background_color(time_layer, GColorBlack);
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_font(time_layer,fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_alignment(time_layer, GTextAlignmentRight);
	
	text_layer_set_background_color(date_layer, GColorBlack);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_font(date_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
	
	// Add layers to window
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	
  window_stack_push(my_window, true);
	
	// Respond to taps
	accel_tap_service_subscribe(tap_handler);
	
	// Update window on load and every minute
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	update_time();
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	accel_tap_service_unsubscribe();
  text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}