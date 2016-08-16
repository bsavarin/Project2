#include <pebble.h>	
#include "inverter.h"
#include <ctype.h>			
#include "health.h"	
#include "ticks.h"	
#include "dithered_rects.h"	
#include <pebble-battery-bar/pebble-battery-bar.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>
#include <pebble-fctx/fpath.h>

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                           DEFINES (COLOUR/BT/BATTERY/CLOCK HANDS/HEALTH)
//----------------------------------------------------------------------------------------------------------------------------------------------

#define HOURS_RADIUS 3
#define INSET PBL_IF_ROUND_ELSE(5, 3)//5,3,4

typedef enum {
  AppModeSteps,
  AppModeDistance,
  AppModeSleep,
  AppModeCalories,
	
  AppModeMax
} AppMode;
static AppMode s_mode;

static Window *window;
#ifdef PBL_COLOR
#else
static int inverted;
InverterLayer *background_inverter_layer;
#endif
static Layer *s_canvas;
static Layer *s_tick_mark_layer;
static Layer *s2_canvas;

static GPath *hour_path, *min_path;

TextLayer *top_panel_layer;
TextLayer *bottom_panel_layer;
TextLayer *right_panel_layer;
TextLayer *no_BT_layer;
static BatteryBarLayer *battery_bar_layer;

static char date_buffer[20];
static char full_date_buffer[32];
static char day_buffer[20];
static char daydate_buffer[20];
static char custom_text_buffer[12];
static char day_week_no_buffer[20];
static char digitime_buffer[20];
static char battperc_buffer[20];
static char location_buffer[32];
static char temperature_buffer[12];
static char temperature_high_buffer[12];
static char temperature_low_buffer[12];
static char conditions_buffer[32];
static char weather_buffer[32];
static char weather1_buffer[32];
static char weather2_buffer[32];
static char steps_buffer[32];
static char distance_buffer[32];
static char sleep_buffer[32];
static char calories_buffer[32];

static int displaySeconds;
static int btVibrate;
static int stepCount, stepGoal, stepAverage, stepGoal2;
static int infoBoxTop;
static int infoBoxBottom;
static int infoBoxRight;
static int displayBattBar;
static int tickMarks;
static int lowBattMode;
static int lowBattLevel;
static int tempUnits = 0;
static int currentTemp;
static int highTemp;
static int lowTemp;

#ifdef PBL_ROUND
static GRect grect1 = {.origin={.x=105,.y=80},.size={.w=42,.h=30}};
static GRect grect2 = {.origin={.x=105,.y=70},.size={.w=42,.h=40}};
#else
static GRect grect1 = {.origin={.x=80,.y=74},.size={.w=42,.h=30}};
static GRect grect2 = {.origin={.x=80,.y=67},.size={.w=42,.h=40}};
#endif

static GFont s_font_16;
static GFont s_font_28;

#ifdef PBL_COLOR
static FFont* s_ffont_28;
#else
#endif

char *sys_locale;

const GPathInfo HOUR_POINTS = {			
  6,
  (GPoint []) {	
    { -5,  12 },
    {  0,  14 },
    {  5,  12 },
    {  3, -40 },
    {  0, -50 },
    { -3, -40 }
  }	
};
static GPath *hour_path;

const GPathInfo MIN_POINTS = {
  6,
  (GPoint []) {
    { -5,  12 },
    {  0,  14 },
    {  5,  12 },
    {  3, -54 },
    {  0, -66 },
    { -3, -54 }
  }
};
static GPath *min_path;

char *upcase(char *str) {
    char *s = str;
    while (*s)
    {	
        *s++ = toupper((int)*s);
    }	
    return str;
}
			
//Define battery and colour
static uint8_t battery_level;
static bool battery_plugged = false;
static bool battery_charging = false;

static int bground_colour = 0x000000; // Black
static int fground_colour = 0xFFFFFF; // White
static int shand_colour = 0xFFFFFF; // White
static int stepGoal_colour = 0x00FFFFFF; // Clear

static GColor background_colour = {.argb = GColorBlackARGB8};
static GColor foreground_colour = {.argb = GColorWhiteARGB8};
static GColor s_hand_colour = {.argb = GColorWhiteARGB8};
static GColor stepGoalcolour = {.argb = GColorClearARGB8};


//static int counter = 0;

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                           FUNCTIONS (COLOUR/BT/BATTERY/HEALTH)
//----------------------------------------------------------------------------------------------------------------------------------------------

void apply_colour(){   
  #ifdef PBL_COLOR
  background_colour = GColorFromHEX(bground_colour); 
  foreground_colour = GColorFromHEX(fground_colour);
  s_hand_colour = GColorFromHEX(shand_colour);
  #else
  background_colour = GColorBlack;
  foreground_colour = GColorWhite;
  s_hand_colour = GColorWhite;
  #endif
  window_set_background_color(window, background_colour);
  text_layer_set_text_color(top_panel_layer, foreground_colour);
  text_layer_set_text_color(bottom_panel_layer, foreground_colour);
  text_layer_set_text_color(right_panel_layer, foreground_colour);
};

  static void bt_handler(bool connected) {
    // Show current connection state
  if (connected) {
      apply_colour();
    #ifdef PBL_COLOR
    #else
      layer_set_hidden(text_layer_get_layer(top_panel_layer), false);
      layer_set_hidden(text_layer_get_layer(no_BT_layer), true);
    #endif
      if (btVibrate == 1 || btVibrate == 3){	
       vibes_short_pulse();
      }
  } else {
    #ifdef PBL_COLOR    
      background_colour = GColorBlack;
      foreground_colour = GColorWhite;
      s_hand_colour = GColorWhite;
      
      window_set_background_color(window, background_colour);
      text_layer_set_text_color(top_panel_layer, foreground_colour);
      text_layer_set_text_color(bottom_panel_layer, foreground_colour);
      text_layer_set_text_color(right_panel_layer, foreground_colour);
    #else    
      layer_set_hidden(text_layer_get_layer(top_panel_layer), true);
      layer_set_hidden(text_layer_get_layer(no_BT_layer), false);  
    #endif			
     if (btVibrate == 1 || btVibrate == 3){
       vibes_long_pulse();
        }
    }
  }

void battery_state_handler(BatteryChargeState state)
{
	battery_level = state.charge_percent;
	battery_plugged = state.is_plugged;
	battery_charging = state.is_charging;
}

 static void get_goal_steps() {
    const time_t start = time_start_of_today(); 
    const time_t end = start + SECONDS_PER_DAY; 
    stepGoal = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
  };

  static void get_current_steps() {
    stepCount = (int)health_service_sum_today(HealthMetricStepCount);
  };

 static void get_avg_steps() {
    const time_t start = time_start_of_today(); 
    const time_t end = time(NULL);
    stepAverage = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily); 
  };

static void set_stepGoal_colour() {
   #ifdef PBL_COLOR   
   if(stepCount >= stepGoal) {  
     stepGoal_colour = ((bground_colour == 0x005500) ? 0x000055 : 0x005500);//GColorDarkGreen or GColorOxfordBlue
  } else if (stepCount >= (3*stepGoal)/4) {
     stepGoal_colour = ((bground_colour == 0xFF5500) ? 0xAA0000 : 0xFF5500);//GColorOrange or GColorDarkCandyAppleRed
  } else {
     stepGoal_colour = ((bground_colour == 0xFF0000) ? 0x550000 : 0xFF0000);//GColorRed or GColorBulgarianRose
  }
#else
stepGoal_colour = 0xFFFFFF; //GColorWhite
#endif
stepGoalcolour = GColorFromHEX(stepGoal_colour);
};

void getHealth(){
    if(health_is_available() && window) {
    int totalCalories = health_get_metric_sum(HealthMetricActiveKCalories) + health_get_metric_sum(HealthMetricRestingKCalories); 
      
    switch(s_mode) {
      case AppModeSteps:
       if (s_mode == 0){
        snprintf(steps_buffer, sizeof(steps_buffer), "%d steps", stepCount);//health_get_metric_sum(HealthMetricStepCount));
        }
        break;
      case AppModeDistance:
       if (s_mode == 1){
        snprintf(distance_buffer, sizeof(distance_buffer), "%d m", health_get_metric_sum(HealthMetricWalkedDistanceMeters));
        }
        break;
      case AppModeSleep:
      if (s_mode == 2){
        snprintf(sleep_buffer, sizeof(sleep_buffer), "%d sleep", health_get_metric_sum(HealthMetricSleepSeconds));
        }
        break;
      case AppModeCalories:
       if (s_mode == 3){
        snprintf(calories_buffer, sizeof(calories_buffer), "%d cal", totalCalories);
        }
        break;
      default: break;
      }
  } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
}

void temp_change() {
  snprintf(weather2_buffer, sizeof(weather2_buffer), "%s", location_buffer);
  if (tempUnits == 0)
    {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d\u00B0C", (int)currentTemp);
    snprintf(temperature_high_buffer, sizeof(temperature_high_buffer), "H/L %d/", (int)highTemp);
    snprintf(temperature_low_buffer, sizeof(temperature_low_buffer), "%d\u00B0C", (int)lowTemp);   
    } else {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d\u00B0F", (int)((currentTemp * 9/5) + 32));
    snprintf(temperature_high_buffer, sizeof(temperature_high_buffer), "H/L %d/", (int)((highTemp * 9/5) + 32));
    snprintf(temperature_low_buffer, sizeof(temperature_low_buffer), "%d\u00B0F", (int)((lowTemp * 9/5) + 32));    
    }
  snprintf(weather_buffer, sizeof(weather_buffer), "%s %s", conditions_buffer, temperature_buffer);
  snprintf(weather1_buffer, sizeof(weather1_buffer), "%s%s", temperature_high_buffer, temperature_low_buffer);
};

void displayBatteryBar(){
  if (displayBattBar == 0) {
  layer_set_hidden(battery_bar_layer, true);
  }
  else if (displayBattBar == 1) {
  layer_set_hidden(battery_bar_layer, false);
  }
};


//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     TICK MARKS
//----------------------------------------------------------------------------------------------------------------------------------------------


static void tick_mark_update_proc(Layer *this_layer, GContext *ctx) {
  
  //"Bezel" background
    GRect tick_layer_bounds = layer_get_bounds(this_layer);
    draw_dithered_rect(ctx, tick_layer_bounds, background_colour, foreground_colour, DITHER_50_PERCENT);
    int radius = 0;
    radius = PBL_IF_RECT_ELSE((TRIG_MAX_ANGLE * tick_layer_bounds.size.h/2) / sin_lookup(atan2_lookup(tick_layer_bounds.size.h/2, tick_layer_bounds.size.w/2)), tick_layer_bounds.size.w/2);
    GRect circle_bounds;
    circle_bounds = GRect(((tick_layer_bounds.size.w/2)-radius)+tick_layer_bounds.origin.x,((tick_layer_bounds.size.h/2)-radius)+tick_layer_bounds.origin.y, radius*2, radius*2);

  //Defines
    GPoint rect_point = GPoint(0,0);  // GPoint to store top left co-ord of rectangle
    int orientation = 0;  // 0 for Vertical, 1 for Horizontal
    
 //Clock tick marks

  //Minutes ticks  
    if(tickMarks == 0 || tickMarks == 1) {
       for (int i=0; i<60; i++) {        
        if (i%5) {
          int iMin = i*6;      
          graphics_context_set_fill_color(ctx, foreground_colour);
          graphics_fill_radial(ctx, circle_bounds, GOvalScaleModeFitCircle, PBL_IF_RECT_ELSE(radius, MINOR_TICK_LENGTH), DEG_TO_TRIGANGLE(iMin-1), DEG_TO_TRIGANGLE(iMin));
          
       /*   graphics_context_set_fill_color(ctx, foreground_colour);
          graphics_fill_radial(ctx, circle_bounds, GOvalScaleModeFitCircle, PBL_IF_RECT_ELSE(radius, MINOR_TICK_LENGTH), DEG_TO_TRIGANGLE((i*6)-1), DEG_TO_TRIGANGLE(i*6));*/
        }
       }
    } 
 
  #ifdef PBL_RECT
    // Draw another rect in the middle using border colour to hide the inner parts of the minute ticks
    draw_dithered_rect(ctx, grect_inset(tick_layer_bounds, GEdgeInsets(MINOR_TICK_LENGTH)), background_colour, foreground_colour, DITHER_50_PERCENT);
  #endif  
  
  //Hours ticks - 3, 6, 9 & 12 o'clock
    if(tickMarks == 0) { 
       for (int i=0; i<4; i++) {     
        switch(i) {
          case 0:
            // Hour 12
            rect_point.x = (tick_layer_bounds.size.w/2) + tick_layer_bounds.origin.x;
            rect_point.y = tick_layer_bounds.origin.y;
            orientation  = 0;
            break;    
          case 1:
            // Hour 3
            rect_point.x = tick_layer_bounds.size.w - MAJOR_TICK_LENGTH + tick_layer_bounds.origin.x;
            rect_point.y = (tick_layer_bounds.size.h/2) + tick_layer_bounds.origin.x;
            orientation  = 1;
            break;   
          case 2:
            // Hour 6
            rect_point.x = tick_layer_bounds.size.w/2;
            rect_point.y = tick_layer_bounds.size.h - MAJOR_TICK_LENGTH + tick_layer_bounds.origin.y;
            orientation  = 0;
            break;   
          case 3:
            // Hour 9
            rect_point.x = tick_layer_bounds.origin.x;
            rect_point.y = (tick_layer_bounds.size.h/2) + tick_layer_bounds.origin.x;
            orientation  = 1;
            break;   
        }
          
    // Draw the double ticks using filled rectangles
        if (orientation == 0) {
          // VERTICAL
          graphics_fill_rect(ctx, GRect(rect_point.x-MAJOR_TICK_WIDTH-1, rect_point.y, MAJOR_TICK_WIDTH, MAJOR_TICK_LENGTH), 0, GCornerNone);
          graphics_fill_rect(ctx, GRect(rect_point.x, rect_point.y, MAJOR_TICK_WIDTH, MAJOR_TICK_LENGTH), 0, GCornerNone);
        } else {
          // HORIZONTAL
          graphics_fill_rect(ctx, GRect(rect_point.x, rect_point.y-MAJOR_TICK_WIDTH-1, MAJOR_TICK_LENGTH, MAJOR_TICK_WIDTH), 0, GCornerNone);
          graphics_fill_rect(ctx, GRect(rect_point.x, rect_point.y, MAJOR_TICK_LENGTH, MAJOR_TICK_WIDTH), 0, GCornerNone);
        }  
      }
    }  

  // Hours ticks - 1, 2, 4, 5, 7, 8, 10, 11 o'clock
      if(tickMarks == 0 || tickMarks == 1 || tickMarks == 2) {       
        for (int i=0; i<12; i++) {        
          if (i%3) {
            int iHour = i*30;
            graphics_context_set_fill_color(ctx, foreground_colour);
            graphics_fill_radial(ctx, circle_bounds, GOvalScaleModeFitCircle, PBL_IF_RECT_ELSE(radius, MAJOR_TICK_LENGTH), DEG_TO_TRIGANGLE(iHour-MINOR_TICK_WIDTH), DEG_TO_TRIGANGLE(iHour+MINOR_TICK_WIDTH));
            
       /*     graphics_context_set_fill_color(ctx, foreground_colour);
            graphics_fill_radial(ctx, circle_bounds, GOvalScaleModeFitCircle, PBL_IF_RECT_ELSE(radius, MAJOR_TICK_LENGTH), DEG_TO_TRIGANGLE((i*30)-MINOR_TICK_WIDTH), DEG_TO_TRIGANGLE((i*30)+MINOR_TICK_WIDTH));*/
          }
        }
      } 
  
  // Fill the center with the background colour
      graphics_context_set_fill_color(ctx, background_colour);
      #ifdef PBL_ROUND
        graphics_fill_radial(ctx, grect_inset(tick_layer_bounds, GEdgeInsets(MAJOR_TICK_LENGTH)), GOvalScaleModeFitCircle, radius-MAJOR_TICK_LENGTH, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
      #else
        graphics_fill_rect(ctx, grect_inset(tick_layer_bounds, GEdgeInsets(MAJOR_TICK_LENGTH)), 0, GCornerNone);
      #endif   
  
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "Counter %d", ++counter);
}

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     DIGITS/STEP GOAL BAR/CIRCLE
//----------------------------------------------------------------------------------------------------------------------------------------------


static void layer_update_proc(Layer *layer, GContext *ctx) {
   graphics_context_set_fill_color(ctx, foreground_colour);
  
    // Digits
     if(tickMarks == 1 || tickMarks == 2 || tickMarks == 3) {      
        #ifdef PBL_COLOR
        FContext fctx;
        fctx_init_context(&fctx, ctx);
        fctx_set_fill_color(&fctx, foreground_colour);
        fctx_set_text_size(&fctx, s_ffont_28, 28);
        fctx_begin_fill(&fctx);
       
        FPoint no_03F = FPointI(PBL_IF_ROUND_ELSE(164, 132),PBL_IF_ROUND_ELSE(90, 84));
        fctx_set_offset(&fctx, no_03F);
        fctx_draw_string(&fctx,"3", s_ffont_28, GTextAlignmentCenter, FTextAnchorMiddle);
 
        FPoint no_06F = FPointI(PBL_IF_ROUND_ELSE(90, 72),PBL_IF_ROUND_ELSE(163, 155));
        fctx_set_offset(&fctx, no_06F);
        fctx_draw_string(&fctx,"6", s_ffont_28, GTextAlignmentCenter, FTextAnchorMiddle);      
       
        FPoint no_09F = FPointI(PBL_IF_ROUND_ELSE(15, 12),PBL_IF_ROUND_ELSE(90, 84));
        fctx_set_offset(&fctx, no_09F);
        fctx_draw_string(&fctx,"9", s_ffont_28, GTextAlignmentCenter, FTextAnchorMiddle);      
       
        FPoint no_12F = FPointI(PBL_IF_ROUND_ELSE(90, 72),PBL_IF_ROUND_ELSE(15, 10));
        fctx_set_offset(&fctx, no_12F);
        fctx_draw_string(&fctx,"12", s_ffont_28, GTextAlignmentCenter, FTextAnchorMiddle);
       
        fctx_end_fill(&fctx);      
        fctx_deinit_context(&fctx);
        #else
     /*   GRect no_03 = PBL_IF_ROUND_ELSE(GRect(159,73,17,17),GRect(125,67,17,17)); //GRect(159,73,17,17) Round
        GRect no_06 = PBL_IF_ROUND_ELSE(GRect(82,146,17,17),GRect(64,137,17,17)); //GRect(82,146,17,17) Round
        GRect no_09 = PBL_IF_ROUND_ELSE(GRect(5,73,17,17),GRect(3,67,17,17));   //GRect(5,73,17,17) Round
        GRect no_12 = PBL_IF_ROUND_ELSE(GRect(74,-2,35,27),GRect(55,-7,35,27));  //GRect(74,-2,35,27) Round*/
       
        GRect no_03 = GRect(125,67,17,17); //GRect(159,73,17,17) Round
        GRect no_06 = GRect(64,137,17,17); //GRect(82,146,17,17) Round
        GRect no_09 = GRect(3,67,17,17);   //GRect(5,73,17,17) Round
        GRect no_12 = GRect(55,-7,35,27);  //GRect(74,-2,35,27) Round
       
        graphics_draw_text(ctx,"3",s_font_28,no_03,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
        graphics_draw_text(ctx,"6",s_font_28,no_06,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
        graphics_draw_text(ctx,"9",s_font_28,no_09,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
        graphics_draw_text(ctx,"12",s_font_28,no_12,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter,NULL);
        #endif
    } 
  

  //Circle around clock
    #ifdef PBL_ROUND
    GRect bounds = layer_get_bounds(layer);	
    GRect frame3 = grect_inset(bounds, GEdgeInsets(-(INSET * 15)/100));
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_fill_color(ctx, foreground_colour);
    graphics_fill_radial(ctx, frame3, GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
    #else
    #endif
  
  //Step goal bar (Basalt, Diorite, Emery), step goal circle (Chalk)
    BatteryChargeState state = battery_state_service_peek();
    if((infoBoxTop == 4 || infoBoxBottom == 4) && (lowBattMode == 0 || state.charge_percent > lowBattLevel)) { 
      graphics_context_set_fill_color(ctx, stepGoalcolour);
      graphics_context_set_antialiased(ctx, true);
    /*  stepCount = 6500; 
      stepGoal = 8000;
      stepAverage = 5000;*/
    #ifdef PBL_ROUND
      GRect frame4 = grect_inset(bounds, GEdgeInsets((INSET * 11)/2));
      stepGoal2 = (360 * stepCount / stepGoal) <= 360 ? (360 * stepCount / stepGoal) : 360;
      graphics_fill_radial(ctx, frame4, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(stepGoal2));
    #else
      stepGoal2 = (98 * stepCount / stepGoal) <= 98 ? (98 * stepCount / stepGoal) : 98;//98 - bar width
      graphics_fill_rect(ctx, GRect(23, 23, stepGoal2, 4), 0, GCornerNone);
    #endif
    }
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "Counter %d", ++counter);
}

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     CLOCK HANDS
//----------------------------------------------------------------------------------------------------------------------------------------------

static void layer_update_proc2(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  int16_t second_hand_length = PBL_IF_ROUND_ELSE(70, 60);

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  //Vibrate every hour
  if (btVibrate == 2 || btVibrate == 3){
        if(tick_time->tm_min == 0 && tick_time->tm_sec == 0) {
          vibes_double_pulse();
        }
      }

  // Clock hands
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, foreground_colour);
  graphics_context_set_stroke_color(ctx, background_colour);
  
  // Hour hand
  int32_t hour_angle = (TRIG_MAX_ANGLE * (((tick_time->tm_hour % 12) * 6) + (tick_time->tm_min / 10))) / (12 * 6);
  gpath_rotate_to(hour_path, hour_angle);
  gpath_draw_filled(ctx, hour_path);
  gpath_draw_outline(ctx, hour_path);

  BatteryChargeState state = battery_state_service_peek();
  if(displaySeconds == 0 && (lowBattMode == 0 || state.charge_percent > lowBattLevel)) {
    int32_t minute_angle = TRIG_MAX_ANGLE * (tick_time->tm_min * 60 + tick_time->tm_sec) / 3600;
    int32_t second_angle = TRIG_MAX_ANGLE * tick_time->tm_sec / 60;
      GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
     };

   // Minute hand
    gpath_rotate_to(min_path, minute_angle);
    gpath_draw_filled(ctx, min_path);
    gpath_draw_outline(ctx, min_path);
    
     // Second hand
    graphics_context_set_stroke_color(ctx, s_hand_colour);
    // Only allowed 1px (too thin) or 3px (too thick).  So three lines very close to each other for greater visibility:-
    graphics_draw_line(ctx, second_hand, PBL_IF_ROUND_ELSE(GPoint(89, 89),GPoint(71, 83)));
    graphics_draw_line(ctx, second_hand, PBL_IF_ROUND_ELSE(GPoint(90, 90),GPoint(72, 84)));//actual centre position
    graphics_draw_line(ctx, second_hand, PBL_IF_ROUND_ELSE(GPoint(89, 90),GPoint(71, 84)));
  
  } else {
    int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
    gpath_rotate_to(min_path, minute_angle);
    gpath_draw_filled(ctx, min_path);
    gpath_draw_outline(ctx, min_path);
  
    s_hand_colour = GColorClear;
  } 

    // Adjust geometry variables for inner ring
      GRect frame = grect_inset(bounds, GEdgeInsets(INSET));
      frame = grect_inset(frame, GEdgeInsets(2 * HOURS_RADIUS));
	
    // Dots in the middle
      GRect frame2 = grect_inset(bounds, GEdgeInsets(PBL_IF_ROUND_ELSE(18,24) * INSET));
  			
      GPoint p = gpoint_from_polar(frame2, GOvalScaleModeFitCircle, 0);
      graphics_context_set_fill_color(ctx, foreground_colour);
      graphics_fill_circle(ctx, p, 4);
  
      GPoint p1 = gpoint_from_polar(frame2, GOvalScaleModeFitCircle, 0);
      graphics_context_set_fill_color(ctx, background_colour);
      graphics_fill_circle(ctx, p1, 2);
  
 // APP_LOG(APP_LOG_LEVEL_DEBUG, "Counter %d", ++counter);
}	

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                             TOP-RIGHT-BOTTOM LAYER
//----------------------------------------------------------------------------------------------------------------------------------------------

void displayRightPanel(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    text_layer_set_text(right_panel_layer, "");

      if(infoBoxRight == 0)//None
      {
        text_layer_set_text(right_panel_layer, "");
      }
        else if(infoBoxRight == 10)//Date
        {
        strftime(date_buffer,sizeof(date_buffer),"%d", tick_time);
        text_layer_set_text(right_panel_layer,date_buffer);
        } 
        else if(infoBoxRight == 11)//Day
        {
        strftime(day_buffer,sizeof(day_buffer),"%a", tick_time);
        upcase(day_buffer);
        text_layer_set_text(right_panel_layer,day_buffer);
        } 
        else if(infoBoxRight == 12)//Day & Date
        {
        strftime(daydate_buffer,sizeof(daydate_buffer),"%a""\n""%d", tick_time);
        upcase(daydate_buffer);
        text_layer_set_text(right_panel_layer,daydate_buffer);
        } 
        else if(infoBoxRight == 13)// Date & Month (UK)
        {
        strftime(daydate_buffer,sizeof(daydate_buffer),"%d""\n""%b", tick_time);
        upcase(daydate_buffer);
        text_layer_set_text(right_panel_layer,daydate_buffer);  
        } 
        else if(infoBoxRight == 14)// Month & Date (US)
        {
        strftime(daydate_buffer,sizeof(daydate_buffer),"%b""\n""%d", tick_time);
        upcase(daydate_buffer);
        text_layer_set_text(right_panel_layer,daydate_buffer);
       }
}


void displayTopPanel(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    text_layer_set_text(top_panel_layer, "");
        
        if(infoBoxTop == 4 || infoBoxTop == 5 || infoBoxTop == 6 || infoBoxTop == 7){
             BatteryChargeState state = battery_state_service_peek();
              if (lowBattMode == 0 || state.charge_percent > lowBattLevel) {
              #ifdef PBL_HEALTH
              getHealth();
              #else
              #endif
              }
        }

      if(infoBoxTop == 0)//None
       {
        text_layer_set_text(top_panel_layer, "");
       } 
      else if(infoBoxTop == 1)//Location
        {	
          if(strlen(weather2_buffer) == 0) {
            text_layer_set_text(top_panel_layer, "Pebble");
          } else {
            text_layer_set_text(top_panel_layer,weather2_buffer);
          }
        }
      else if(infoBoxTop == 2)//Conditions
       {
          if(strlen(weather2_buffer) == 0) {
            text_layer_set_text(top_panel_layer, "Pebble");
          } else {
            text_layer_set_text(top_panel_layer,weather_buffer);
          }
       }
      else if(infoBoxTop == 3)//Day Number & Week Number
       {
         text_layer_set_text(top_panel_layer,day_week_no_buffer);
       }
      else if(infoBoxTop == 4)//Steps
       {
        s_mode = 0;
        text_layer_set_text(top_panel_layer,steps_buffer);
       }
      else if(infoBoxTop == 5)//Distance
       {
        s_mode = 1;
        text_layer_set_text(top_panel_layer,distance_buffer);
       }
      else if(infoBoxTop == 6)//Sleep
       {
        s_mode = 2;
        text_layer_set_text(top_panel_layer,sleep_buffer);
       }
      else if(infoBoxTop == 7)//Calories
       {
        s_mode = 3;
        text_layer_set_text(top_panel_layer,calories_buffer);
       }
      else if(infoBoxTop == 8)//Digital time
      {
        text_layer_set_text(top_panel_layer,digitime_buffer);
      }
      else if(infoBoxTop == 9)//Full UK date format
      {
        strftime(full_date_buffer, sizeof(full_date_buffer), "%a %d %b", tick_time);
        text_layer_set_text(top_panel_layer,full_date_buffer);
      }
      else if(infoBoxTop == 10)//Full US date format
      {
        strftime(full_date_buffer, sizeof(full_date_buffer), "%a %b %d", tick_time);
        text_layer_set_text(top_panel_layer,full_date_buffer);
      }
      else if(infoBoxTop == 11)//High/Low Temp
      {
        if(strlen(weather2_buffer) == 0) {
        text_layer_set_text(top_panel_layer, "Hi--/Lo--");
        } else {
        text_layer_set_text(top_panel_layer,weather1_buffer);    
        }
      }
        else if(infoBoxTop == 12)//Battery %
      {
       text_layer_set_text(top_panel_layer,battperc_buffer);
      }

  temp_change();
}


void displayBottomPanel(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    text_layer_set_text(bottom_panel_layer, "");
  
       if(infoBoxBottom == 4 || infoBoxBottom == 5 || infoBoxBottom == 6 || infoBoxBottom == 7){
            BatteryChargeState state = battery_state_service_peek();
            if (lowBattMode == 0 || state.charge_percent > lowBattLevel) {
              #ifdef PBL_HEALTH
              getHealth();
              #else
              #endif
            }
          }

      if(infoBoxBottom == 0)//None
       {
        text_layer_set_text(bottom_panel_layer, "");
       } 
      else if(infoBoxBottom == 1)//Location
        {	
          if(strlen(weather2_buffer) == 0) {
            text_layer_set_text(bottom_panel_layer, "Pebble");
          } else {
            text_layer_set_text(bottom_panel_layer,weather2_buffer);
          }
        }
      else if(infoBoxBottom == 2)//Conditions
       {
          if(strlen(weather2_buffer) == 0) {
            text_layer_set_text(bottom_panel_layer, "Pebble");
          } else {
            text_layer_set_text(bottom_panel_layer,weather_buffer);
          }
       }
      else if(infoBoxBottom == 3)//Day Number & Week Number
       {
         text_layer_set_text(bottom_panel_layer,day_week_no_buffer);
       }
      else if(infoBoxBottom == 4)//Steps
       {
        s_mode = 0;
        text_layer_set_text(bottom_panel_layer,steps_buffer);
       }
      else if(infoBoxBottom == 5)//Distance
       {
        s_mode = 1;
        text_layer_set_text(bottom_panel_layer,distance_buffer);
       }
      else if(infoBoxBottom == 6)//Sleep
       {
        s_mode = 2;
        text_layer_set_text(bottom_panel_layer,sleep_buffer);
       }
      else if(infoBoxBottom == 7)//Calories
       {
        s_mode = 3;
        text_layer_set_text(bottom_panel_layer,calories_buffer);
       }
      else if(infoBoxBottom == 8)//Digital time
      {
        text_layer_set_text(bottom_panel_layer,digitime_buffer);
      }
      else if(infoBoxBottom == 9)//Full UK date format
      {
        strftime(full_date_buffer, sizeof(full_date_buffer), "%a %d %b", tick_time);
        text_layer_set_text(bottom_panel_layer,full_date_buffer);
      }
      else if(infoBoxBottom == 10)//Full US date format
      {
        strftime(full_date_buffer, sizeof(full_date_buffer), "%a %b %d", tick_time);
        text_layer_set_text(bottom_panel_layer,full_date_buffer);
      }
      else if(infoBoxBottom == 11)//High/Low Temp
      {
        if(strlen(weather2_buffer) == 0) {
        text_layer_set_text(bottom_panel_layer, "Hi--/Lo--");
        } else {
        text_layer_set_text(bottom_panel_layer,weather1_buffer);    
        }
      }
      else if(infoBoxBottom == 12)//Battery %
      {
       text_layer_set_text(bottom_panel_layer,battperc_buffer);
      }
      else if(infoBoxBottom == 13)//Custom Text
      {
        persist_read_string(MESSAGE_KEY_CUSTOM_TEXT, custom_text_buffer, sizeof(custom_text_buffer));
        text_layer_set_text(bottom_panel_layer, custom_text_buffer);
      }
  
  temp_change();
}


//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     UPDATE TIME/TICK HANDLER/INVERTER LAYER
//----------------------------------------------------------------------------------------------------------------------------------------------


static void update_time() {
  // Get a tm structure
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  displayBatteryBar();
  displayRightPanel();
  displayTopPanel();
  displayBottomPanel();
  
  layer_set_frame(text_layer_get_layer(right_panel_layer), infoBoxRight == 10 || infoBoxRight == 11 ? grect1 : grect2);
  
   if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(digitime_buffer, sizeof(digitime_buffer), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(digitime_buffer, sizeof(digitime_buffer), "%I:%M %P", tick_time);
  }	
  strftime(day_week_no_buffer, sizeof(day_week_no_buffer), "D%j W%U", tick_time);  
  
  
   if(tick_time->tm_min == 0 && tick_time->tm_sec == 0) {
       health_init();
            get_avg_steps();
            get_goal_steps();
            get_current_steps();
            set_stepGoal_colour();
            getHealth();
            displayRightPanel();
    
        if(infoBoxTop != 0 && infoBoxTop != 3 && infoBoxTop != 8 && infoBoxTop != 9 && infoBoxTop != 10) {
              displayTopPanel();
        }           
        if (infoBoxBottom != 0 && infoBoxBottom != 3 && infoBoxBottom != 8 && infoBoxBottom != 9 && infoBoxBottom != 10 && infoBoxBottom != 13) {
              displayBottomPanel();
        }
       health_service_events_unsubscribe();
    }
  
 // APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap used: %d, free: %d", (int)heap_bytes_used(), (int)heap_bytes_free());
}	

			
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
    if (changed & SECOND_UNIT) {
      update_time();
    }  

  BatteryChargeState state = battery_state_service_peek();	
  snprintf(battperc_buffer,sizeof(battperc_buffer),"%d%%", state.charge_percent);
  
  if (lowBattMode == 1 && state.charge_percent <= lowBattLevel) {
    health_service_events_unsubscribe();
        if(infoBoxTop != 0 && infoBoxTop != 3 && infoBoxTop != 9 && infoBoxTop != 10) {
          layer_set_hidden(text_layer_get_layer(top_panel_layer), true);
        }   
        if(infoBoxBottom != 0 && infoBoxBottom != 3 && infoBoxBottom != 9 && infoBoxBottom != 10 && infoBoxBottom != 13) {
          layer_set_hidden(text_layer_get_layer(bottom_panel_layer), true);
        }
    } 
}

//Define several TextLayers with the same settings
  static TextLayer * create_initialized_text_layer(GRect bounds) {
    TextLayer * result = text_layer_create(bounds);
    text_layer_set_text_alignment(result, GTextAlignmentCenter); 
    text_layer_set_background_color(result, GColorClear);
    text_layer_set_text_color(result, foreground_colour);
    text_layer_set_font(result, s_font_16);
    layer_add_child(window_get_root_layer(window),text_layer_get_layer(result));
    return result;
}

void display_inverter_layer() {
  //Inverter options
    #ifdef PBL_COLOR
    #else
      if(inverted == 1)//Set and save as inverted
      {
        layer_set_hidden(inverter_layer_get_layer(background_inverter_layer), false);
      } else {//Set and save as not inverted
        layer_set_hidden(inverter_layer_get_layer(background_inverter_layer), true);
      }
    #endif
  //End of inverter options
}

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     WINDOW LOAD/UNLOAD
//----------------------------------------------------------------------------------------------------------------------------------------------

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, background_colour);
    
  s_font_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LIBERATION_SANS_BOLD_16));
  s_font_28 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LIBERATION_SANS_BOLD_28));
  
  #ifdef PBL_COLOR
  s_ffont_28 = ffont_create_from_resource(RESOURCE_ID_LIBERATION_SANS_FFONT);
  #else
  #endif
  
  s_tick_mark_layer = layer_create(bounds);
  layer_set_update_proc(s_tick_mark_layer, &tick_mark_update_proc);
  layer_add_child(window_layer, s_tick_mark_layer);
  
  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, &layer_update_proc);
  layer_add_child(window_layer, s_canvas); 
  
  battery_bar_layer = battery_bar_layer_create();
  battery_bar_set_position(PBL_IF_ROUND_ELSE(GPoint(60, 36),GPoint(41, 29)));
    #ifdef PBL_COLOR
    battery_bar_set_colors(GColorDarkGreen, GColorOrange, GColorRed, GColorBlue);
    #else	
    battery_bar_set_colors(GColorWhite, GColorWhite, GColorWhite, GColorWhite);
    #endif
  battery_bar_set_percent_hidden(1);
  layer_add_child(window_get_root_layer(window),battery_bar_layer);
   
	right_panel_layer = create_initialized_text_layer(infoBoxRight == 10 || infoBoxRight == 11 ? grect1 : grect2);
  top_panel_layer = create_initialized_text_layer(PBL_IF_ROUND_ELSE(GRect(35,43,113,21),GRect(16,36,113,21)));
  bottom_panel_layer = create_initialized_text_layer(PBL_IF_ROUND_ELSE(GRect(35,118,113,21),GRect(16,118,113,21)));
  no_BT_layer = create_initialized_text_layer(PBL_IF_ROUND_ELSE(GRect(35,43,113,21),GRect(16,36,113,21)));// NO BT label, Aplite only
  text_layer_set_text(no_BT_layer, "BT OFF"); 
  layer_set_hidden(text_layer_get_layer(no_BT_layer), true);
  
  get_avg_steps();
  get_goal_steps();
  get_current_steps();
  set_stepGoal_colour(); 
  getHealth();
  displayRightPanel();
  displayTopPanel();
  displayBottomPanel();
  
  s2_canvas = layer_create(bounds);
  layer_set_update_proc(s2_canvas, &layer_update_proc2);
  layer_add_child(window_layer, s2_canvas);
  
  
  #ifdef PBL_COLOR
  #else
  background_inverter_layer = inverter_layer_create(GRect(0,0,144,168));
  display_inverter_layer();
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(background_inverter_layer));
  #endif
 
  bluetooth_connection_service_subscribe(bt_handler);
  
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap used: %d, free: %d", (int)heap_bytes_used(), (int)heap_bytes_free());  
}	


static void window_unload(Window *window) {
  #ifdef PBL_COLOR
  #else
  inverter_layer_destroy(background_inverter_layer);
  #endif
  
  text_layer_destroy(top_panel_layer);
  text_layer_destroy(bottom_panel_layer);
  text_layer_destroy(right_panel_layer);
  text_layer_destroy(no_BT_layer);
  battery_bar_layer_destroy(battery_bar_layer);
  
  fonts_unload_custom_font(s_font_16);
  fonts_unload_custom_font(s_font_28);
  
  #ifdef PBL_COLOR
  ffont_destroy(s_ffont_28);  
  #else
  #endif
  
  layer_destroy(s_tick_mark_layer);
  layer_destroy(s_canvas);
  layer_destroy(s2_canvas);
}	
static void tick_handler(struct tm *tick_time, TimeUnits changed);

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                    MESSAGE KEY PERSISTS/TUPLES/INBOX-OUTBOX
//----------------------------------------------------------------------------------------------------------------------------------------------

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  // Store incoming information

   // Read first item
  Tuple *loc = dict_find(iter, MESSAGE_KEY_LOCATION);
  Tuple *cond = dict_find(iter, MESSAGE_KEY_CONDITIONS);
  Tuple *temp = dict_find(iter, MESSAGE_KEY_TEMPERATURE);
  Tuple *tempunit = dict_find(iter, MESSAGE_KEY_TEMPERATURE_UNIT);
  Tuple *temphigh = dict_find(iter, MESSAGE_KEY_TEMPERATURE_HIGH);
  Tuple *templow = dict_find(iter, MESSAGE_KEY_TEMPERATURE_LOW);
  Tuple *invert = dict_find(iter, MESSAGE_KEY_INVERT);
  Tuple *seconds = dict_find(iter, MESSAGE_KEY_SECONDS);
  Tuple *vib = dict_find(iter, MESSAGE_KEY_BT_VIBRATE);
  Tuple *batt = dict_find(iter, MESSAGE_KEY_BATTERY_BAR);
  Tuple *bg = dict_find(iter, MESSAGE_KEY_BACKGROUND_COLOUR);
  Tuple *fg = dict_find(iter, MESSAGE_KEY_FOREGROUND_COLOUR);
  Tuple *tick = dict_find(iter, MESSAGE_KEY_TICK_MARKS);
  Tuple *lowbatt = dict_find(iter, MESSAGE_KEY_LOW_BATTERY_MODE);
  Tuple *lowlevel = dict_find(iter, MESSAGE_KEY_LOW_BATTERY_LEVEL);
  Tuple *rinfo = dict_find(iter, MESSAGE_KEY_RIGHT_INFORMATION_BOX);
  Tuple *tinfo = dict_find(iter, MESSAGE_KEY_TOP_INFORMATION_BOX);
  Tuple *binfo = dict_find(iter, MESSAGE_KEY_BOTTOM_INFORMATION_BOX);
  Tuple *custom = dict_find(iter, MESSAGE_KEY_CUSTOM_TEXT);
  
  if(loc) {
    snprintf(location_buffer, sizeof(location_buffer), "%s", loc->value->cstring);
  } 
  if(temp) {
    currentTemp = temp->value->int32;
   }
  if(temphigh) {
    highTemp = temphigh->value->int32;
  }	
  if(templow) {
    lowTemp = templow->value->int32;
  }	    
  if(cond) {
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", cond->value->cstring);
  } 
  if(tempunit) {
    tempUnits = atoi(tempunit->value->cstring);
    }
  
  if(invert) {
    #ifdef PBL_COLOR
    #else
    inverted = invert->value->int32; display_inverter_layer();
    #endif
  } 
  
  if(seconds) {
      //It's the KEY_SECONDS key
    tick_timer_service_unsubscribe();
      if(seconds->value->int32 == 1)//Set and display second hand
      {
        displaySeconds = 0;        
          #ifdef PBL_COLOR
          apply_colour();
          #else
          s_hand_colour = GColorWhite;
          #endif
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      }
      else if(seconds->value->int32 == 0) //Set and hide second hand
      {
        displaySeconds = 1;       
          #ifdef PBL_COLOR
          s_hand_colour = GColorClear;
          #else
          s_hand_colour = GColorClear;
          #endif
        tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      }
  }
  
  if(vib) {
    btVibrate = vib->value->int32;
  }
  
  if(batt) {
    displayBattBar = batt->value->int32;
  }
    
  if(bg) {
      #ifdef PBL_COLOR
      bground_colour = bg->value->uint32; apply_colour();
      #else
      apply_colour();
      #endif
  }
  
    if(fg) {
      #ifdef PBL_COLOR
      fground_colour = fg->value->uint32; apply_colour();
      shand_colour = fg->value->uint32; apply_colour();
      #else
      apply_colour();
      #endif
  } 
  
  if(tick) {
    tickMarks = tick->value->int32;
  }
  
  if(lowbatt) {
    lowBattMode = lowbatt->value->int32;
  }
  
  if(lowlevel) {
    lowBattLevel = lowlevel->value->int32;
  }
	
  if(rinfo) {
    infoBoxRight = atoi(rinfo->value->cstring); displayRightPanel();
  } 
	
  if(tinfo) {
    infoBoxTop = atoi(tinfo->value->cstring); displayTopPanel();
  }
  
  if(binfo) {
    infoBoxBottom = atoi(binfo->value->cstring);
      if(infoBoxBottom == 13) {	  
          if(custom) {
            char *customText = strncpy(custom_text_buffer, custom->value->cstring, sizeof(custom_text_buffer));
          }
      }
    displayBottomPanel();
  } 	
  temp_change();
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

static void app_focus_changing(bool focusing) {
    if (focusing) {
      Layer *window_layer = window_get_root_layer(window);
      layer_set_hidden(window_layer, true);
    }
}
static void app_focus_changed(bool focused) {
    if (focused) {
      Layer *window_layer = window_get_root_layer(window);
      layer_set_hidden(window_layer, false);
      layer_mark_dirty(s_tick_mark_layer);
      layer_mark_dirty(s_canvas);
      layer_mark_dirty(s2_canvas);
      layer_mark_dirty(window_layer);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------
//                                                     WINDOW UPDATE/INIT/DEINIT
//----------------------------------------------------------------------------------------------------------------------------------------------

void main_window_update() {
  layer_mark_dirty(s_tick_mark_layer);
  layer_mark_dirty(s_canvas);
  layer_mark_dirty(s2_canvas);
}

static void init() {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
  
  health_init();
  
  sys_locale = setlocale(LC_ALL, "");
  
  hour_path = gpath_create(&HOUR_POINTS);
  min_path = gpath_create(&MIN_POINTS);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
 	
  gpath_move_to(hour_path, center);
  gpath_move_to(min_path, center);
  			
  bluetooth_connection_service_subscribe(bt_handler);
  BatteryChargeState state = battery_state_service_peek();
  
  if(displaySeconds == 0 && (lowBattMode = 0 || state.charge_percent > lowBattLevel)) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }	
  
app_focus_service_subscribe_handlers((AppFocusHandlers){ 
  .did_focus = app_focus_changed, 
  .will_focus = app_focus_changing
});
  
   // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);	
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_open(256, 256);//128,128
  
  bground_colour = persist_exists(MESSAGE_KEY_BACKGROUND_COLOUR) ? persist_read_int(MESSAGE_KEY_BACKGROUND_COLOUR): 0x000000; apply_colour();
  fground_colour = persist_exists(MESSAGE_KEY_FOREGROUND_COLOUR) ? persist_read_int(MESSAGE_KEY_FOREGROUND_COLOUR): 0xFFFFFF;	apply_colour();
  shand_colour = persist_exists(MESSAGE_KEY_SECOND_HAND_COLOUR) ? persist_read_int(MESSAGE_KEY_SECOND_HAND_COLOUR): 0xFFFFFF; apply_colour();
  btVibrate = persist_exists(MESSAGE_KEY_BT_VIBRATE) ? persist_read_int(MESSAGE_KEY_BT_VIBRATE): 0;
  displayBattBar = persist_exists(MESSAGE_KEY_BATTERY_BAR) ? persist_read_int(MESSAGE_KEY_BATTERY_BAR): 0;
  tempUnits = persist_exists(MESSAGE_KEY_TEMPERATURE_UNIT) ? persist_read_int(MESSAGE_KEY_TEMPERATURE_UNIT) : 0;
  displaySeconds = persist_exists(MESSAGE_KEY_SECONDS) ? persist_read_int(MESSAGE_KEY_SECONDS): 0;
  infoBoxRight = persist_exists(MESSAGE_KEY_RIGHT_INFORMATION_BOX) ? persist_read_int(MESSAGE_KEY_RIGHT_INFORMATION_BOX): 0;
  infoBoxTop = persist_exists(MESSAGE_KEY_TOP_INFORMATION_BOX) ? persist_read_int(MESSAGE_KEY_TOP_INFORMATION_BOX): 0;
  infoBoxBottom = persist_exists(MESSAGE_KEY_BOTTOM_INFORMATION_BOX) ? persist_read_int(MESSAGE_KEY_BOTTOM_INFORMATION_BOX): 0;
  tickMarks = persist_exists(MESSAGE_KEY_TICK_MARKS) ? persist_read_int(MESSAGE_KEY_TICK_MARKS): 0;
  lowBattMode = persist_exists(MESSAGE_KEY_LOW_BATTERY_MODE) ? persist_read_int(MESSAGE_KEY_LOW_BATTERY_MODE): 0;
  lowBattLevel = persist_exists(MESSAGE_KEY_LOW_BATTERY_LEVEL) ? persist_read_int(MESSAGE_KEY_LOW_BATTERY_LEVEL): 0;
  s_mode = persist_exists(MESSAGE_KEY_HEALTH) ? persist_read_int(MESSAGE_KEY_HEALTH): 0;
     
    if(persist_exists(MESSAGE_KEY_CUSTOM_TEXT)){
    persist_read_string(MESSAGE_KEY_CUSTOM_TEXT, custom_text_buffer, sizeof(custom_text_buffer));
  }
  
  #ifdef PBL_COLOR
  #else
  inverted = persist_exists(MESSAGE_KEY_INVERT) ? persist_read_int(MESSAGE_KEY_INVERT): 0;
  #endif 
  
}

static void deinit() {

  gpath_destroy(min_path);
  gpath_destroy(hour_path);
  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  health_service_events_unsubscribe(); 
  
  persist_write_int(MESSAGE_KEY_BACKGROUND_COLOUR, bground_colour);
  persist_write_int(MESSAGE_KEY_FOREGROUND_COLOUR, fground_colour);
  persist_write_int(MESSAGE_KEY_SECOND_HAND_COLOUR, shand_colour); 
  persist_write_int(MESSAGE_KEY_BT_VIBRATE, btVibrate);
  persist_write_int(MESSAGE_KEY_SECONDS, displaySeconds);
  persist_write_int(MESSAGE_KEY_RIGHT_INFORMATION_BOX, infoBoxRight);
  persist_write_int(MESSAGE_KEY_TOP_INFORMATION_BOX, infoBoxTop);
  persist_write_int(MESSAGE_KEY_BOTTOM_INFORMATION_BOX, infoBoxBottom);
  persist_write_int(MESSAGE_KEY_BATTERY_BAR, displayBattBar);
  persist_write_int(MESSAGE_KEY_TEMPERATURE_UNIT, tempUnits);
  persist_write_int(MESSAGE_KEY_TICK_MARKS, tickMarks);
  persist_write_int(MESSAGE_KEY_LOW_BATTERY_MODE, lowBattMode);
  persist_write_int(MESSAGE_KEY_LOW_BATTERY_LEVEL, lowBattLevel);
  persist_write_int(MESSAGE_KEY_HEALTH, s_mode);
  persist_write_string(MESSAGE_KEY_CUSTOM_TEXT, custom_text_buffer);
  
  #ifdef PBL_COLOR
  #else
  persist_write_int(MESSAGE_KEY_INVERT, inverted);
  #endif
    
  window_destroy(window); 
}	

int main() {
  init();
  app_event_loop();
  deinit();
}	
