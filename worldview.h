#ifndef WORLDVIEW_H
#define WORLDVIEW_H
#include <menu.h>
#include <panel.h>
#include <form.h>
PANEL* init_worldview();
typedef enum UI_FLAGS{
  DEBUG = 0x01,
  GENES = 0x02,
  
} UI_FLAGS;

typedef enum MAJOR_MODES{
  GENERATIONS_AUTO,
  BONSAI,
  FLOATIES,
  STORY,
}MAJOR_MODES;
typedef enum MINOR_MODES{
  SHOW_PLANTS,
  SHOW_RESOURCES,
  SHOW_PHOTO,
  SHOW_HUMMUS,
  SHOW_WATER,
  SHOW_FREE_WATER,
  SHOW_WIND
}MINOR_MODES;

  
  
      
    
typedef struct Viewport{
  int ys;
  int xs;
  int y;
  int x;
}Viewport;

typedef struct Cursor{
  int y;
  int x;
}Cursor;

typedef struct Frame{
  void* userptr;
  void* nc_userptr;
  WINDOW* sub;
  int focus;
}Frame; //have to call it *something*

  
typedef struct Worldview_State{
  Viewport v;
  Cursor c;
  int minor_mode;
  CA* ca;
  char ui_flags;
}Worldview_State;

char* pump_worldview(PANEL* panel,Worldview_State* ws);
void init_colors();
void act_worldview(Worldview_State* ws,int input);
#endif
