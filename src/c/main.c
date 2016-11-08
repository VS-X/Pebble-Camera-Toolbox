#include <pebble.h>
#define BG_COL_BLUE PBL_IF_COLOR_ELSE(GColorMidnightGreen, GColorBlack)
#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3

#define NUM_SETTINGS_MENU_SECTIONS 1
#define NUM_SETTINGS_MENU_ITEMS 2

//TODO :
// Depth of Field Calculator
// Design
static GFont s_font;

static Window *s_hyperfocal_window;
static Window *s_menu_window;
//static Window *s_coc_window;
static Window *s_settings_window;
//static Window *s_dof_window;

static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_menu_items[NUM_MENU_ITEMS];

static SimpleMenuLayer *s_settings_menu_layer;
static SimpleMenuSection s_settings_menu_sections[NUM_SETTINGS_MENU_SECTIONS];
static SimpleMenuItem s_settings_menu_items[NUM_SETTINGS_MENU_ITEMS];


static StatusBarLayer *s_status_bar;;

static TextLayer *s_focal_layer;
static TextLayer *s_aperture_layer;
static TextLayer *s_hyperfocal_layer;
static TextLayer *s_coc_layer;

static TextLayer *s_yourFocal_layer;
static TextLayer *s_yourAperture_layer;
static TextLayer *s_yourHyperfocal_layer;
static TextLayer *s_yourCoc_layer;

static char s_focal_text[20];
static char s_aperture_text[20];
static char s_hyperfocal_text[20];
static char s_coc_text[30];

const char yourFocal_text[6] = "Focal\0";
const char yourAperture_text[3] = "f/\0";
const char yourCoc_text[20] = PBL_IF_RECT_ELSE("Circle of Confusion\0", "CoC\0");
const char yourHyperfocal_text_meter[14] = "Hyperfocal(m)\0";
const char yourHyperfocal_text_feet[15] = "Hyperfocal(ft)\0";

uint32_t key_coc = 1;      // divide by 1000 = 0.019
uint32_t key_focal = 2;    // as is = 30mm
uint32_t key_aperture = 3; // divide by 10 = f4.0

static int s_focal;
static int s_aperture;
static int s_coc;
static float s_hyperfocal;

static bool cocEditable = 1;
static bool focalEditable = 0;
static bool apertureEditable = 0;

uint32_t key_cocAdvanced = 4;
static bool cocAdvanced;
uint32_t key_cocSimple = 5;
static int cocSimple; //0=Fullframe=0.025, 1=APS-C=0.019, 2=APS-C(Canon)=0.018, 3=1.5"=0.016, 4=4/3=0.015, 5=1"=0.011, 6=1/1.2"=0.009, 7=2/3=0.007, 8=1/1.7"=0.006, 9=1/2.3"=0.005

uint32_t key_units = 6;
static bool metricUnits = 1;

char* floatToString(char* buffer, int bufferSize, double number)
{
	char decimalBuffer[5];

	if (metricUnits == 1)
	{
		snprintf(buffer, bufferSize, "%d", (int)number);
		if ((int)number < 10) {
			strcat(buffer, ".");
			snprintf(decimalBuffer, 5, "%02d", (int)((float)(number - (int)number) * (float)100));
			strcat(buffer, decimalBuffer);
		}
	}
	else
	{
		number = (number * 328 / 100);
		snprintf(buffer, bufferSize, "%d", (int)number);
		if ((int)number < 10) {
			strcat(buffer, ".");
			snprintf(decimalBuffer, 5, "%02d", (int)((float)(number - (int)number) * (float)100));
			strcat(buffer, decimalBuffer);
		}
	}
	//strcat(buffer, "m");
	return buffer;
}
char* cocToString(char* buffer, int bufferSize, int number)
{

	if (cocAdvanced == 1) {
		char decimalBuffer[5];
		snprintf(buffer, bufferSize, "%d", number / 1000);
		strcat(buffer, ".");
		snprintf(decimalBuffer, 5, "%03d", number);
		strcat(buffer, decimalBuffer);
	}
	else {
		switch (cocSimple) {
		case 0:
			buffer = "Full Frame";
			break;
		case 1:
			buffer = "APS-C";
			break;
		case 2:
			buffer = "APS-C Canon";
			break;
		case 3:
			buffer = "1.5";
			break;
		case 4:
			buffer = "4/3";
			break;
		case 5:
			buffer = "1";
			break;
		case 6:
			buffer = "1/1.2";
			break;
		case 7:
			buffer = "2/3";
			break;
		case 8:
			buffer = "1/1.7";
			break;
		case 9:
			buffer = "1/2.3";
			break;
		default:
			break;
		}
	}
	return buffer;
}
char* focalToString(char* buffer, int bufferSize, int number)
{
	snprintf(buffer, bufferSize, "%d", number);
	return buffer;
}
char* apertureToString(char* buffer, int bufferSize, int number)
{
	char decimalBuffer[3];
	snprintf(buffer, bufferSize, "%d", number / 10);
	if (number < 100) {
		strcat(buffer, ".");
		snprintf(decimalBuffer, 3, "%d", number % 10);
		strcat(buffer, decimalBuffer);
	}
	return buffer;
}

static void hyperfocal_layout_update() {
		if (cocEditable == 1) {

		text_layer_set_background_color(s_coc_layer, BG_COL_BLUE);
		text_layer_set_background_color(s_yourCoc_layer, BG_COL_BLUE);
		text_layer_set_text_color(s_coc_layer, GColorWhite);
		text_layer_set_text_color(s_yourCoc_layer, GColorWhite);
		text_layer_set_background_color(s_focal_layer, GColorClear);
		text_layer_set_background_color(s_yourFocal_layer, GColorClear);
		text_layer_set_text_color(s_focal_layer, GColorBlack);
		text_layer_set_text_color(s_yourFocal_layer, GColorBlack);
		text_layer_set_background_color(s_aperture_layer, GColorClear);
		text_layer_set_background_color(s_yourAperture_layer, GColorClear);
		text_layer_set_text_color(s_aperture_layer, GColorBlack);
		text_layer_set_text_color(s_yourAperture_layer, GColorBlack);
	}
	else if (focalEditable == 1) {
		text_layer_set_background_color(s_coc_layer, GColorClear);
		text_layer_set_background_color(s_yourCoc_layer, GColorClear);
		text_layer_set_text_color(s_coc_layer, GColorBlack);
		text_layer_set_text_color(s_yourCoc_layer, GColorBlack);
		text_layer_set_background_color(s_focal_layer, BG_COL_BLUE);
		text_layer_set_background_color(s_yourFocal_layer, BG_COL_BLUE);
		text_layer_set_text_color(s_focal_layer, GColorWhite);
		text_layer_set_text_color(s_yourFocal_layer, GColorWhite);
		text_layer_set_background_color(s_aperture_layer, GColorClear);
		text_layer_set_background_color(s_yourAperture_layer, GColorClear);
		text_layer_set_text_color(s_aperture_layer, GColorBlack);
		text_layer_set_text_color(s_yourAperture_layer, GColorBlack);
	}
	else if (apertureEditable == 1) {
		text_layer_set_background_color(s_coc_layer, GColorClear);
		text_layer_set_background_color(s_yourCoc_layer, GColorClear);
		text_layer_set_text_color(s_coc_layer, GColorBlack);
		text_layer_set_text_color(s_yourCoc_layer, GColorBlack);
		text_layer_set_background_color(s_focal_layer, GColorClear);
		text_layer_set_background_color(s_yourFocal_layer, GColorClear);
		text_layer_set_text_color(s_focal_layer, GColorBlack);
		text_layer_set_text_color(s_yourFocal_layer, GColorBlack);
		text_layer_set_background_color(s_aperture_layer, BG_COL_BLUE);
		text_layer_set_background_color(s_yourAperture_layer, BG_COL_BLUE);
		text_layer_set_text_color(s_aperture_layer, GColorWhite);
		text_layer_set_text_color(s_yourAperture_layer, GColorWhite);
	}
}

static void read_persistent_values() {
	if (persist_exists(key_units)) {
		// Read persisted value
		metricUnits = persist_read_bool(key_units);
	}
	else {
		metricUnits = 1;
	}
	if (persist_exists(key_cocAdvanced)) {
		// Read persisted value
		cocAdvanced = persist_read_bool(key_cocAdvanced);
	}
	else {
		cocAdvanced = 0;
	}	
	if (persist_exists(key_coc)) {
			// Read persisted value
			s_coc = persist_read_int(key_coc);
	}
	else {
			s_coc = 19;
	}
	if (persist_exists(key_cocSimple)) {
			// Read persisted value
			cocSimple = persist_read_int(key_cocSimple);
	}
	else {
			cocSimple = 1;
	}
	if (persist_exists(key_focal)) {
		// Read persisted value
		s_focal = persist_read_int(key_focal);
	}
	else {
		s_focal = 30;
	}
	if (persist_exists(key_aperture)) {
		// Read persisted value
		s_aperture = persist_read_int(key_aperture);
	}
	else {
		s_aperture = 40;
	}
}

static void update() {
	
	if (cocAdvanced == 0) {
		switch (cocSimple) {
		case 0:
			s_coc = 25;
			break;
		case 1:
			s_coc = 19;
			break;
		case 2:
			s_coc = 18;
			break;
		case 3:
			s_coc = 16;
			break;
		case 4:
			s_coc = 15;
			break;
		case 5:
			s_coc = 11;
			break;
		case 6:
			s_coc = 9;
			break;
		case 7:
			s_coc = 7;
			break;
		case 8:
			s_coc = 6;
			break;
		case 9:
			s_coc = 5;
			break;
		default:
			break;
		}
	}
	s_hyperfocal = ((s_focal * s_focal) / (((float)s_aperture / 10) * ((float)s_coc / 1000)) + s_focal) / 1000;

	text_layer_set_text(s_hyperfocal_layer, floatToString(s_hyperfocal_text, sizeof(s_hyperfocal_text), s_hyperfocal));
	text_layer_set_text(s_coc_layer, cocToString(s_coc_text, sizeof(s_coc_text), s_coc));
	text_layer_set_text(s_aperture_layer, apertureToString(s_aperture_text, sizeof (s_aperture_text), s_aperture));
	text_layer_set_text(s_focal_layer, focalToString(s_focal_text, sizeof(s_focal_text), s_focal));

	
}

static void menu_select_callback(int index, void *ctx) {

	//s_menu_items[index].subtitle = "You've hit select here!";
	switch (index) {
	case 0:
		window_stack_push(s_hyperfocal_window, true);
		update();
		break;
	case 2:
		window_stack_push(s_settings_window, true);
		break;
	default:
		break;
	}

	layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void settings_menu_select_callback(int index, void *ctx) {

	//s_menu_items[index].subtitle = "You've hit select here!";
	switch (index) {
	case 0:
		if (metricUnits == 1) {
			s_settings_menu_items[index].title = "Units: feet";
		}
		else {
			s_settings_menu_items[index].title = "Units: meters";
		}
		metricUnits = !metricUnits;
		persist_write_bool(key_units, metricUnits);
		break;
	case 1:
		if (cocAdvanced == 1) {
			s_settings_menu_items[index].title = "Simple Circle of Confusion";
		}
		else {
			s_settings_menu_items[index].title = "Manual Circle of Confusion";
		}
		cocAdvanced = !cocAdvanced;
		persist_write_bool(key_cocAdvanced, cocAdvanced);
		break;
	default:
		return;
	}

	layer_mark_dirty(simple_menu_layer_get_layer(s_settings_menu_layer));
}

static void select_click_handler_hyperfocal(ClickRecognizerRef recognizer, void *context) {
	if (cocEditable == 1) {
		cocEditable = 0;
		focalEditable = 1;
		apertureEditable = 0;
	}
	else	{
		if (focalEditable == 1) {
			cocEditable = 0;
			focalEditable = 0;
			apertureEditable = 1;
		}
		else {
			if (apertureEditable == 1) {
				cocEditable = 1;
				focalEditable = 0;
				apertureEditable = 0;

			}
		}
	}
	
	hyperfocal_layout_update();
}

static void up_click_handler_hyperfocal(ClickRecognizerRef recognizer, void *context) {
	if (cocAdvanced == 1 && cocEditable == 1 && s_coc < 1000) {
		s_coc = s_coc + 1;
		persist_write_int(key_coc, s_coc);
	}
	else if (cocAdvanced == 0 && cocEditable == 1) {
		cocSimple--;
		if (cocSimple < 0)
			cocSimple = 9;
		persist_write_int(key_cocSimple, cocSimple);
	}
	
	if (focalEditable == 1 && s_focal < 1000) {
		if (s_focal >= 300) {
			s_focal = s_focal + 10;
		}
		else if (s_focal >= 100) {
			s_focal = s_focal + 5;
		}
		else {
			s_focal = s_focal + 1;
		}
		persist_write_int(key_focal, s_focal);
	}

	if (apertureEditable == 1  && s_aperture < 640) {
		if (s_aperture >= 100) {
			s_aperture = s_aperture + 10;
		}
		else {
			s_aperture = s_aperture + 1;
		}
		persist_write_int(key_aperture, s_aperture);
	}	
	
	update();
}
static void down_click_handler_hyperfocal(ClickRecognizerRef recognizer, void *context) {

	if (cocAdvanced == 1 && cocEditable == 1 && s_coc > 1) {
		s_coc = s_coc - 1;
		persist_write_int(key_coc, s_coc);
	}
	else if (cocAdvanced == 0 && cocEditable == 1) {
		cocSimple++;
		if (cocSimple > 9)
			cocSimple = 0;
		persist_write_int(key_cocSimple, cocSimple);
	}
	
	if (focalEditable == 1 && s_focal > 1) {
		if (s_focal >= 310) {
			s_focal = s_focal - 10;
		}
		else if (s_focal >= 105) {
			s_focal = s_focal - 5;
		}
		else {
			s_focal = s_focal - 1;
		}
		persist_write_int(key_focal, s_focal);
	}

	if (apertureEditable == 1  && s_aperture > 10) {
		if (s_aperture >= 110) {
			s_aperture = s_aperture - 10;
		}
		else {
			s_aperture = s_aperture - 1;
		}
		persist_write_int(key_aperture, s_aperture);
	}
	
	update();
}
static void click_config_provider_hyperfocal(void *context) {
	// Subcribe to button click events here
	ButtonId b_select = BUTTON_ID_SELECT;
	ButtonId b_up = BUTTON_ID_UP;
	ButtonId b_down = BUTTON_ID_DOWN;

	uint16_t repeat_interval_ms = 100;

	window_single_click_subscribe(b_select, select_click_handler_hyperfocal);
	window_single_click_subscribe(b_up, up_click_handler_hyperfocal);
	window_single_click_subscribe(b_down, down_click_handler_hyperfocal);
	window_single_repeating_click_subscribe(b_up, repeat_interval_ms, up_click_handler_hyperfocal);
	window_single_repeating_click_subscribe(b_down, repeat_interval_ms, down_click_handler_hyperfocal);
}

static void hyperfocal_window_load(Window *window) {
	// Get information about the Window
	s_status_bar = status_bar_layer_create();

	Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_unobstructed_bounds(window_layer);
	GRect layer_bounds = window_bounds;
	layer_bounds.size.w /= 2;


	// Create TextLayer with specific bounds
	s_yourCoc_layer = text_layer_create (GRect(0, PBL_IF_RECT_ELSE(10, 18), window_bounds.size.w, 25));
	s_coc_layer = text_layer_create (GRect(0, PBL_IF_RECT_ELSE(35, 42), window_bounds.size.w, 26));

	s_yourFocal_layer = text_layer_create (GRect(0, PBL_IF_RECT_ELSE(60, 67), layer_bounds.size.w, 30));
	s_focal_layer = text_layer_create (GRect(0, PBL_IF_RECT_ELSE(85, 92), layer_bounds.size.w, 26));

	s_yourAperture_layer = text_layer_create (GRect(layer_bounds.size.w, PBL_IF_RECT_ELSE(60, 67), layer_bounds.size.w, 30));
	s_aperture_layer = text_layer_create (GRect(layer_bounds.size.w, PBL_IF_RECT_ELSE(85, 92), layer_bounds.size.w, 26));

	s_yourHyperfocal_layer = text_layer_create (GRect(0, 115, window_bounds.size.w, 38));
	s_hyperfocal_layer = text_layer_create (GRect(0, 137, window_bounds.size.w, 28));

	

	text_layer_set_text(s_coc_layer, cocToString(s_coc_text, sizeof (s_coc_text), s_coc));
	text_layer_set_font(s_coc_layer, s_font);
	text_layer_set_text_alignment(s_coc_layer, GTextAlignmentCenter);

	if (cocAdvanced == 1) {
		text_layer_set_text(s_yourCoc_layer, yourCoc_text);
	}
	else {
		text_layer_set_text(s_yourCoc_layer, "Sensor Size");
	}
	text_layer_set_font(s_yourCoc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_yourCoc_layer, GTextAlignmentCenter);

	text_layer_set_text(s_focal_layer, focalToString(s_focal_text, sizeof(s_focal_text), s_focal));
	text_layer_set_font(s_focal_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
	text_layer_set_text_alignment(s_focal_layer, GTextAlignmentCenter);

	text_layer_set_text(s_yourFocal_layer, yourFocal_text);
	text_layer_set_font(s_yourFocal_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_yourFocal_layer, GTextAlignmentCenter);

	text_layer_set_text(s_aperture_layer, apertureToString(s_aperture_text, sizeof(s_aperture_text), s_aperture));
	text_layer_set_font(s_aperture_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
	text_layer_set_text_alignment(s_aperture_layer, GTextAlignmentCenter);

	text_layer_set_text(s_yourAperture_layer, yourAperture_text);
	text_layer_set_font(s_yourAperture_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_yourAperture_layer, GTextAlignmentCenter);

	text_layer_set_background_color(s_hyperfocal_layer, GColorClear);
	text_layer_set_text_color(s_hyperfocal_layer, GColorBlack);
	text_layer_set_text(s_hyperfocal_layer, s_hyperfocal_text);
	text_layer_set_font(s_hyperfocal_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));
	text_layer_set_text_alignment(s_hyperfocal_layer, GTextAlignmentCenter);

	text_layer_set_background_color(s_yourHyperfocal_layer, GColorClear);
	text_layer_set_text_color(s_yourHyperfocal_layer, GColorBlack);
	if (metricUnits == 1) {
		text_layer_set_text(s_yourHyperfocal_layer, yourHyperfocal_text_meter);
	}
	else {
		text_layer_set_text(s_yourHyperfocal_layer, yourHyperfocal_text_feet);
	}
	text_layer_set_font(s_yourHyperfocal_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_yourHyperfocal_layer, GTextAlignmentCenter);



	//add it as a child layer to the window's root layer

	layer_add_child(window_layer, text_layer_get_layer(s_coc_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_yourCoc_layer));

	layer_add_child(window_layer, text_layer_get_layer(s_yourFocal_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_focal_layer));

	layer_add_child(window_layer, text_layer_get_layer(s_yourAperture_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_aperture_layer));

	layer_add_child(window_layer, text_layer_get_layer(s_hyperfocal_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_yourHyperfocal_layer));

	layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
	
	hyperfocal_layout_update();
	update();

}
static void hyperfocal_window_unload(Window *window) {
	status_bar_layer_destroy(s_status_bar);
	text_layer_destroy(s_focal_layer);
	text_layer_destroy(s_aperture_layer);
	text_layer_destroy(s_hyperfocal_layer);
	text_layer_destroy(s_coc_layer);
	text_layer_destroy(s_yourCoc_layer);
	text_layer_destroy(s_yourFocal_layer);
	text_layer_destroy(s_yourAperture_layer);
	text_layer_destroy(s_yourHyperfocal_layer);
}

static void menu_window_load(Window *window) {

	int num_a_items = 0;

	s_menu_items[num_a_items++] = (SimpleMenuItem) {
		.title = "Hyperfocal",
		 .callback = menu_select_callback,
	};
	s_menu_items[num_a_items++] = (SimpleMenuItem) {
		.title = "Depth of Field",
		 .callback = menu_select_callback,
		.subtitle = "Coming soon!"
	};
	s_menu_items[num_a_items++] = (SimpleMenuItem) {
		.title = "Settings",
		 .callback = menu_select_callback,
	};

	s_menu_sections[0] = (SimpleMenuSection) {
		.num_items = NUM_MENU_ITEMS,
		 .items = s_menu_items,
	};

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	s_simple_menu_layer = simple_menu_layer_create(bounds, window, s_menu_sections, NUM_MENU_SECTIONS, NULL);

	layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
}
static void menu_window_unload(Window *window) {
	simple_menu_layer_destroy(s_simple_menu_layer);
}

static void settings_window_load(Window *window) {
	// Although we already defined NUM_FIRST_MENU_ITEMS, you can define
	// an int as such to easily change the order of menu items later
	int num_a_items = 0;

	if (metricUnits == 1) {
		s_settings_menu_items[num_a_items++] = (SimpleMenuItem) {
			.title = "Units: meters",
			 .callback = settings_menu_select_callback,
		};
	}
	else {
		s_settings_menu_items[num_a_items++] = (SimpleMenuItem) {
			.title = "Units: feet",
			 .callback = settings_menu_select_callback,
		};
	}

	if (cocAdvanced == 1) {
		s_settings_menu_items[num_a_items++] = (SimpleMenuItem) {
			.title = "Manual Circle of Confusion",
			 .callback = settings_menu_select_callback,
		};
	}
	else {
		s_settings_menu_items[num_a_items++] = (SimpleMenuItem) {
			.title = "Simple Circle of Confusion",
			 .callback = settings_menu_select_callback,
		};
	}
	s_settings_menu_sections[0] = (SimpleMenuSection) {
		.num_items = NUM_SETTINGS_MENU_ITEMS,
		 .items = s_settings_menu_items,
	};

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	s_settings_menu_layer = simple_menu_layer_create(bounds, window, s_settings_menu_sections, NUM_MENU_SECTIONS, NULL);

	layer_add_child(window_layer, simple_menu_layer_get_layer(s_settings_menu_layer));
}
static void settings_window_unload(Window *window) {
	simple_menu_layer_destroy(s_settings_menu_layer);
}

static void init() {
	s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_18));
	
	read_persistent_values();
	
	s_hyperfocal_window = window_create();
	window_set_window_handlers(s_hyperfocal_window, (WindowHandlers) {
		.load = hyperfocal_window_load,
		 .unload = hyperfocal_window_unload
	});
	window_set_click_config_provider(s_hyperfocal_window, click_config_provider_hyperfocal);

	s_settings_window = window_create();
	window_set_window_handlers(s_settings_window, (WindowHandlers) {
		.load = settings_window_load,
		 .unload = settings_window_unload
	});

	s_menu_window = window_create();
	window_set_window_handlers(s_menu_window, (WindowHandlers) {
		.load = menu_window_load,
		 .unload = menu_window_unload
	});
	window_stack_push(s_menu_window, true);
}

static void deinit() {
	window_destroy(s_settings_window);
	window_destroy(s_menu_window);
	window_destroy(s_hyperfocal_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}