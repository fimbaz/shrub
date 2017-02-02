#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <form.h>
#include "shrub.h"
#include "genome.h" 
#include "display.h"
#include "serialize.h"
#include "simulate.h"
#include "worldview.h"
#define FOCUS(panel) (((Frame*)panel_userptr(panel))->focus = 1)
#define UNFOCUS(panel) (((Frame*)panel_userptr(panel))->focus = 0)
#define USERPTR(panel) (((Frame*)panel_userptr(panel))->userptr)
#define NCUSERPTR(panel) (((Frame*)panel_userptr(panel))->nc_userptr)
#define GETFRAME(panel)  ((Frame*)panel_userptr(panel))
#define HAS_FOCUS(panel) (((Frame*)panel_userptr(panel))->focus)
char *NSTRINGS[9] = {"SELF","TOP","TOP_RIGHT","RIGHT","BOTTOM_RIGHT","BOTTOM","BOTTOM_LEFT","LEFT","TOP_LEFT"};  
extern int XMAP[10];
extern int YMAP[10];




void auto_generations(Display*D){
  Worldview_State* ws = (Worldview_State*)USERPTR(D->worldview_panel);
  if(ws->ca == NULL)
    return;
  CA* ca = ws->ca;
  World* world = ws->ca->world;
  uint   cays			= 100;
  uint   caxs			= 1000;
  float  freq			= .005;
  float  amp			= 250;
  uint   offset			= 50;
  uint   bheight		= 5;
    uint   diversity		= 10;
  uint   tree_interval		=80;
  auto_breed_plants(world,world->zone_count-1,diversity);
  dealloc_CA(ca);
  uint zone =  new_perlin_zone(cays,caxs,freq,amp,offset,bheight,tree_interval,world);
  ca = &world->zones[zone];
  world->generation++;
  auto_seed_CA(ca,50,world->max_species_id-diversity+1,world->max_species_id-1);
  ws->ca = ca;
  return;
}

PANEL* init_stats(){
  WINDOW* win	= newwin(5,20,0,0);
  WINDOW* sub	= subwin(win,3,18,0,1);
  Frame* frame	= calloc(1,sizeof(Frame));
  frame->sub	= sub;
  PANEL* panel	= new_panel(win);
  set_panel_userptr(panel,frame);
  hide_panel(panel);
  wborder(win, '|', '|', ' ', '-', '|', '|', '+', '+');
  return panel;
}


PANEL* init_main_menu(){
  int choices = 4;
  Frame* frame = calloc(1,sizeof(Frame));
  WINDOW* decor = newwin(6,16,0,0);
  WINDOW* win = subwin(decor,4,14,1,1);
  PANEL* panel = new_panel(decor);
  ITEM** items = calloc(choices+1,sizeof(ITEM*));
  items[0] = new_item("Generations (auto)","");
  items[1] = new_item("Bonsai","");
  items[2] = new_item("Load File","");
  items[3] = new_item("Save File","");
  MENU* menu = new_menu(items);
  set_menu_win(menu,decor);
  set_menu_sub(menu,win);
  wborder(decor, '|', '|', '-', '-', '+', '+', '+', '+');
  post_menu(menu);
  frame->userptr = menu;
  set_panel_userptr(panel,(void*)frame);
  return panel;
}

PANEL* init_species_menu(){
  int choices = 4;
  Frame* frame = calloc(1,sizeof(Frame));
  WINDOW* decor = newwin(6,16,0,0);
  WINDOW* win = subwin(decor,4,14,1,1);
  PANEL* panel = new_panel(decor);
  ITEM** items = calloc(choices+1,sizeof(ITEM*));
  items[0] = new_item("NULL","");
  MENU* menu = new_menu(items);
  set_menu_win(menu,decor);
  set_menu_sub(menu,win);
  wborder(decor, '|', '|', '-', '-', '+', '+', '+', '+');
  post_menu(menu);
  frame->nc_userptr = menu;
  set_panel_userptr(panel,(void*)frame);
  return panel;
}



char* pump_species_menu(Display*D,int input){
  PANEL* species_menu = D->species_menu;  
  MENU* menu =  (MENU*)NCUSERPTR(species_menu);
  Worldview_State* ws = (Worldview_State*)USERPTR(species_menu);
  if(HAS_FOCUS(D->worldview_panel) && input == 's'){
    World* world = ws->ca->world;
    ITEM** items = menu_items(menu);
    int count = item_count(menu);
    for(int i=0;i<count;i++) free_item(items[i]);
    free(items);
    items = calloc(world->max_species_id,sizeof(ITEM*)+1);
    for(int i=0;i<world->max_species_id-1;i++){
      items[i] = new_item(world->genomes[world->plantae[i+1].genome_id].name,"");
    }
    unpost_menu(menu);
    set_menu_items(menu,items);
    int y,x;
    scale_menu(menu,&y,&x);
    wresize(menu_win(menu),y+2,x+2);
    wresize(menu_sub(menu),y,x);
    wborder(menu_win(menu), '|', '|', '-', '-', '+', '+', '+', '+');
    FOCUS(D->species_menu);
    UNFOCUS(D->worldview_panel);
    show_panel(D->species_menu);
  }
  if(!HAS_FOCUS(species_menu))
    input = ERR;
  ITEM* item = NULL;
  const char* name = NULL;
    switch(input){
    case KEY_DOWN:
      menu_driver(menu,REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(menu,REQ_UP_ITEM);
      break;
    case '\n':
      item = current_item(menu);
      name =  item_name(item);
      menu_driver(menu,REQ_TOGGLE_ITEM);
      UNFOCUS(D->species_menu);
      FOCUS(D->worldview_panel);
      hide_panel(D->species_menu);
      break;
     default:
       break;
     }
     post_menu(menu);

     if(HAS_FOCUS(D->species_menu)){
       WINDOW* win = panel_window(D->species_menu);
       int yc,xc,xs,ys,smys,smxs;
       getmaxyx(panel_window(D->worldview_panel),ys,xs);
       getmaxyx(panel_window(D->species_menu),smys,smxs);
       int y = (ys-smys)/2; int x = (xs-smxs)/2;
       move_panel(D->species_menu,y,x);
       getyx(win,yc,xc);
       
       move(yc+y,xc+x);
     }

     return (char*)name;

}
PANEL* init_plant_info(){
  Frame* frame = calloc(1,sizeof(Frame));
  FIELD** field = (FIELD**)calloc(5,sizeof(FIELD*));
  FORM* form;
  field[0] = new_field(1,10,0,0,0,0);
  field[1] = new_field(1,30,0,11,0,0);
  field[2] = new_field(1,10,1,0,0,0);;
  field[3] = new_field(1,30,1,11,0,0);;
  field[4] = NULL;
  set_field_buffer(field[0],0,"Name   ");
  set_field_buffer(field[2],0,"Remarks:   ");
  field_opts_off(field[0],O_EDIT);
  field_opts_off(field[2],O_EDIT);
  field_opts_off(field[2],O_ACTIVE);
  field_opts_off(field[0],O_ACTIVE);
  field_opts_on(field[1],O_BLANK);
  field_opts_on(field[1],O_ACTIVE);
  field_opts_on(field[3],O_BLANK);
  field_opts_on(field[3],O_ACTIVE);

  form = new_form(field);
  set_current_field(form,field[1]);
  int y,x;
  scale_form(form,&y,&x);
  WINDOW* win = newwin(y+3,x+2,0,0);
  WINDOW* sub = subwin(win,y,x,0,1);
  PANEL* panel = new_panel(win);
  set_form_win(form,win);
  set_form_sub(form,sub);
  set_panel_userptr(panel,frame);
  frame->sub = sub;
  wborder(win, '|', '|', ' ', '-', '|', '|', '+', '+');
  frame->nc_userptr = form;
  post_form(form);
  clear();
  update_panels();
  doupdate();

  return panel;
}

char* pump_plant_info(Display*D,int input){
  PANEL* panel = D->plant_info;
  FORM* form = NCUSERPTR(panel);
  Worldview_State* ws = ((Worldview_State*)USERPTR(D->plant_info));
  uint y = ws->c.y + ws->v.y;
  uint x = ws->c.x + ws->v.x;
  FIELD** fields = form_fields(form);
  Genome* g = NULL;

  if(ws->ca != NULL &&in_bounds(ws->ca,y,x) && ws->ca->new[y][x].parent != SELF){
    g  = &ws->ca->world->genomes[ws->ca->new[y][x].species_id];
    show_panel(D->plant_info);
    if(!HAS_FOCUS(panel) && input == 'n'){
      input = ERR;
      FOCUS(panel);
      top_panel(panel);
      set_current_field(form,fields[1]);
      set_field_buffer(fields[1],0,"");
      set_field_buffer(fields[3],0,"");
      UNFOCUS(D->worldview_panel);
    }
    if(!HAS_FOCUS(panel)){
      char* name = g->name;
      char* print_me;
      if(strlen(name) == 0){
	print_me = "(Unnamed Species)";
      }else{
	print_me = name;
      }
      set_field_buffer(fields[1],0,print_me);      
      set_field_buffer(fields[3],0,g->remarks);      
    }
  }else{
      UNFOCUS(D->plant_info);
      hide_panel(D->plant_info);
  }
    if(HAS_FOCUS(panel)){
      switch(input){
      case KEY_DOWN:
	form_driver(form,REQ_NEXT_FIELD);
	break;
      case KEY_UP:
	form_driver(form,REQ_PREV_FIELD);
	break;

      case KEY_BACKSPACE:
	form_driver(form,REQ_LEFT_CHAR);
	form_driver(form,REQ_DEL_CHAR);
	break;
      case '\n':
	if(field_index(current_field(form)) == 1){
	  form_driver(form,REQ_NEXT_FIELD);
	  break;
	}
	pos_form_cursor(form);
	form_driver(form,REQ_VALIDATION);
	if(g != NULL){
	  strncpy(g->name,field_buffer(fields[1],0),sizeof(g->name));
	  strncpy(g->remarks,field_buffer(fields[3],0),sizeof(g->remarks));
	}
	FOCUS(D->worldview_panel);
	UNFOCUS(D->plant_info);
	break;
      default:
	form_driver(form,input);
	break;
      }
    }

    post_form(form);
    int ya,xa;
    if(HAS_FOCUS(panel)){
      getyx(panel_window(D->plant_info),ya,xa);
      move(ya,xa);
    }
     return NULL;
}

const char* pump_stats(Display*D,int c){
  Worldview_State* ws = ((Worldview_State*)USERPTR(D->worldview_panel));
  if(ws->ca == NULL){
    return NULL;
  }
  struct timeval* ltt = &ws->ca->world->last_tick_duration;
  int tick_time = ltt->tv_sec*1000 + ltt->tv_usec/1000;
  mvwprintw(GETFRAME(D->stats_panel)->sub,0,0,"Tick %d      ",ws->ca->tick);
  mvwprintw(GETFRAME(D->stats_panel)->sub,1,0,"Generation %d      ",ws->ca->world->generation);
  mvwprintw(GETFRAME(D->stats_panel)->sub,1,0,"Last Tick (ms) %d      ",tick_time);
  return NULL;
}


int act_main_menu(Display*D,const char*name){
  Worldview_State* ws = (Worldview_State*)USERPTR(D->worldview_panel);
  if(!strcmp(name,"Load File")){
    if(ws->ca == NULL)
      return -1;
    load_genomes(ws->ca->world,"genomes.bin");
  }
  else if(!strcmp(name,"Save File")){
    if(ws->ca == NULL)
      return -1;
    save_genomes(ws->ca->world,"genomes.bin");
  }
  else if(!strcmp(name,"Generations (auto)")){
    if(ws->ca != NULL){
      dealloc_world(ws->ca->world);
    }
    D->mode = GENERATIONS_AUTO;
    World* w = init_simulation();
    ws->ca = &w->zones[0];
  }
  return 0;
}
const char* pump_main_menu(Display*D,int c){
  PANEL* main_menu = D->main_menu_panel;  
  MENU* menu =  (MENU*)USERPTR(main_menu);
  if(!HAS_FOCUS(main_menu))
    c = ERR;

  ITEM* item = NULL;
  const char* name = NULL;
    switch(c){
    case KEY_DOWN:
      menu_driver(menu,REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(menu,REQ_UP_ITEM);
      break;
    case '\n':
      item = current_item(menu);
      name =  item_name(item);
      menu_driver(menu,REQ_TOGGLE_ITEM);
      act_main_menu(D,name);
       break;
     default:
       break;
     }
     post_menu(menu);
     return name;
 }

 void init_display(Display*D){
     initscr();    
     cbreak();
     noecho();
     D->plant_info        = init_plant_info();
     D->main_menu_panel   = init_main_menu();
     D->worldview_panel   = init_worldview();
     D->stats_panel       = init_stats();
     D->species_menu      = init_species_menu();
     timeout(100);
     Frame* mmwin = (Frame*)panel_userptr(D->main_menu_panel);
     mmwin->focus = 1;
     keypad(stdscr,TRUE);
     start_color();
     init_colors();
     curs_set(2);

     Worldview_State* ws = USERPTR(D->worldview_panel);
     ((Frame*)(panel_userptr(D->plant_info)))->userptr = ws;
     ((Frame*)(panel_userptr(D->species_menu)))->userptr = ws;
     top_panel(D->worldview_panel);
     top_panel(D->main_menu_panel);
     top_panel(D->plant_info);
     hide_panel(D->species_menu);
     UNFOCUS(D->plant_info);
     FOCUS(D->worldview_panel);
 }



 const char* pump_display(Display*D,CA*ca){
   WINDOW* mw = panel_window(D->main_menu_panel);
   WINDOW* sw = panel_window(D->stats_panel);
   uint ys,xs,mwys,mwxs,stys,stxs; getmaxyx(stdscr,ys,xs); getmaxyx(mw,mwys,mwxs); getmaxyx(sw,stys,stxs);
   const char* choice =  NULL;
   int input = wgetch(stdscr);
   static int menu_on = 1;
    const char* menuopt = pump_main_menu(D,input);
   if(input == '~' || menuopt != NULL){
     if(menu_on){
       menu_on = 0;
       UNFOCUS(D->main_menu_panel);
       FOCUS(D->worldview_panel);
       hide_panel(D->main_menu_panel);
     }else{
       menu_on = 1;
       UNFOCUS(D->worldview_panel);
       FOCUS(D->main_menu_panel);
       show_panel(D->main_menu_panel);
    }
  }
   pump_species_menu(D,input);
   pump_plant_info(D,input);
   pump_worldview(D,input);
   pump_stats(D,input);

  if(menuopt != NULL){
     choice = menuopt;
     show_panel(D->stats_panel);
  }  
  pump_stats(D,input);
  move_panel(D->main_menu_panel,(ys-mwys)/2,(xs-mwxs)/2);
  move_panel(D->stats_panel,stys-stys,xs-stxs);
  Cursor c = ((Worldview_State*)USERPTR(D->worldview_panel))->c;

  update_panels();
  doupdate();
  if(HAS_FOCUS(D->worldview_panel))
    move(c.y,c.x);
  refresh();
  return choice;
  
}

  

  
