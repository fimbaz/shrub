#ifndef DISPLAY_H
#define DISPLAY_H
#define FOCUS(panel) (((Frame*)panel_userptr(panel))->focus = 1)
#define UNFOCUS(panel) (((Frame*)panel_userptr(panel))->focus = 0)
#define USERPTR(panel) (((Frame*)panel_userptr(panel))->userptr)
#define NCUSERPTR(panel) (((Frame*)panel_userptr(panel))->nc_userptr)
#define GETFRAME(panel)  ((Frame*)panel_userptr(panel))
#define HAS_FOCUS(panel) (((Frame*)panel_userptr(panel))->focus)

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
  char minor_mode;
  CA* ca;
  char ui_flags;
}Worldview_State;
typedef struct Display{
  char mode;
  UI_FLAGS ui_flags;
  PANEL* worldview_panel;
  PANEL* main_menu_panel;
  PANEL* species_menu;
  PANEL* stats_panel;
  PANEL* plant_info;
}Display;

  
  
void init_colors();


void display_resource(WINDOW*win,CA*ca,Viewport v);
void display_photosynthate(WINDOW*win,CA*ca,Viewport v);
void display_water(WINDOW*win,CA*ca,Viewport v);
void display_free_water(WINDOW*win,CA*ca,Viewport v);
void display_wind(WINDOW*win,CA*ca,Viewport v);
void display_nutrients(WINDOW*win,CA*ca,Viewport v);
void show_debug(WINDOW*win,CA*ca,Viewport v);
void display_modules(WINDOW*win,CA*ca,Viewport v,uint debug);
void display_info(WINDOW*win, CA* ca,Viewport v,Cursor c);
int transl_curs(const CA*ca,Viewport*v,Cursor*c,int y,int x);
int move_curs(CA*ca,uint* sely,uint*selx,int c);
char* pump_worldview(Display*D,int input);
void display_gene_info(WINDOW*win,CA*ca,Viewport v,Cursor c);
const char* pump_display(Display*D,CA*ca);
void init_display(Display*D);


//a whole new world!  watch this space for new data structure interactions
//and exciting features. 
#endif
