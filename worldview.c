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
#include "serialize.h"
#include "worldview.h"
#include "items.h"
extern int XMAP[10];
extern int YMAP[10];

void display_resource(WINDOW*win,CA*ca,Viewport v){

  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      mvwaddch(win,i-v.y,j-v.x,ca->new[i][j].resource+'0');
    }}
  mvprintw(0,0,"   RESC   ");
  return;
	
      
}

void display_photosynthate(WINDOW*win,CA*ca,Viewport v){
  
  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      uint p_y = i-v.y;
      uint p_x = j-v.x;
      if(ca->new[i][j].parent == SELF)
	mvwaddch(win,p_y,p_x,' ');
      else
	mvwaddch(win,p_y,p_x,ca->new[i][j].nutrients[PHOTO]+'0');
    }}
  mvprintw(0,0,"   PHOTO   ");
  return;
  
      
}




void display_water(WINDOW*win,CA*ca,Viewport v){
  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      uint p_y = i-v.y;
      uint p_x = j-v.x;
      if(ca->new[i][j].parent == SELF){
	mvwaddch(win,p_y,p_x,' ');
      }else{
	mvwaddch(win,p_y,p_x,ca->new[i][j].nutrients[WATER]+'0');
      }
      
    }}
  mvprintw(0,0,"   BOUND WATER   ");
  return;
      
}


void display_free_water(WINDOW*win,CA*ca,Viewport v){
  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  char ch;
  uint water = 0;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      water += ca->new[i][j].water + ca->new[i][j].nutrients[WATER];
    }}
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      uint p_y = i-v.y;
      uint p_x = j-v.x;
      if(ca->new[i][j].water == 0){
	ch = ' ';
      }else{
	ch = ca->new[i][j].water+'0';
      }
      
      mvwaddch(win,p_y,p_x,ch);
      
    }}
  mvprintw(0,0,"   TOTAL WATER: %d   ",water);
  return;
      
}


void display_wind(WINDOW*win,CA*ca,Viewport v){
  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  char ch;
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      uint p_y = i-v.y;
      uint p_x = j-v.x;
      Cell*V = &ca->new[i][j];
      float wind = V->wind[0] - V->wind[1];
      int nwind = (int)round(wind*10);
      if(abs(nwind) < 1){
	ch = ' ';
      }else{
	if(wind < 0)
	  nwind = abs(nwind) +10;
	ch = nwind+'0';
      }
      
      mvwaddch(win,p_y,p_x,ch);
      
    }}
  mvprintw(0,0,"   WIND   ");
  return;
      
}


void display_nutrients(WINDOW*win,CA*ca,Viewport v){
  uint max_x = v.x+v.xs > ca->xs ? ca->xs : v.x+v.xs;
  uint max_y = v.y+v.ys > ca->ys ? ca->ys : v.y+v.ys;
  for(uint i=v.y;i<max_y;i++){for(uint j=v.x;j<max_x;j++){
      uint p_y = i-v.y;
      uint p_x = j-v.x;
      if(ca->new[i][j].parent == SELF){
	mvwaddch(win,p_y,p_x,' ');
      }else{
	mvwaddch(win,p_y,p_x,ca->new[i][j].nutrients[HUMMUS]+'0');
      }
      
    }}
  mvprintw(0,0,"   HUMMUS   ");
  return;
      
}

void show_debug(WINDOW*win,CA*ca,Viewport v){
  uint free_hummus = 0;
  uint living =0;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &ca->new[i][j];
      if(V->parent != SELF)
	living++;
      if(V->substrate_type == GROUND){
	free_hummus += V->resource;
      }
    }}
  mvprintw(0,v.xs-16,"FREE: %d",free_hummus);
  mvprintw(1,v.xs-16,"LIVE: %d",living);
  mvprintw(2,v.xs-16,"TURN: %d",ca->tick);
  
	
    
}

  
  
void display_modules(WINDOW*win,CA*ca,Viewport v,uint debug){
  int ys = v.ys;
  int xs =v.xs;
  int y = v.y;
  int x = v.x;
  
  Board* board = ca->new;
  
  for(int i=y;i<ys+y;i++){for(int j=x;j<xs+x;j++){
      char display_char = ' ';
      int pair =0;
        //these are background characters
      if(!in_bounds(ca,i,j)){
	display_char = '~';
	goto display;
      }
      Cell* V = &ca->new[i][j];
      init_pair(50,17,0);
      init_pair(51,17,17);
      if(V->substrate_type == AIR && V->parent == SELF && NULL == V->items){
	if(in_bounds(ca,i+1,j) && (ca->new[i+1][j].substrate_type == GROUND || ca->new[i+1][j].water >= SATURATED)){
	  if(V->water < 3){
	    mvwaddch(win,i-y,j-x,' ');
	  }else if(V->water < 5){
	    wattron(win,COLOR_PAIR(50));
	    mvwaddch(win,i-y,j-x,'_');
	    wattroff(win,COLOR_PAIR(50));
	  }else{
	    wattron(win,COLOR_PAIR(51));
	    mvwaddch(win,i-y,j-x,' ');
	    wattroff(win,COLOR_PAIR(51));
	  }
	}else if (V->water != 0){
	  wattron(win,COLOR_PAIR(50));
	  mvwaddch(win,i-y,j-x,'|');
	  wattroff(win,COLOR_PAIR(50));
	  
	}else{
	  mvwaddch(win,i-y,j-x,' ');
	}
	continue;	
      }
      
      if(V->items != NULL){
	pair =V->items->fg_color;
	init_pair(pair,V->items->fg_color,V->items->bg_color);
	display_char = V->items->shape;
	goto display;
      }
      if(V->substrate_type == GROUND){
	if(V->parent == SELF){
	  display_char = '.';
	}else{
	  display_char = ',';
      }
    }
      if(V->substrate_type == VOID){
	display_char = '~';
      }


      Tissue* T = get_tissue(ca,V->species_id,V->tissue_id);
      if(T == NULL || (!T->visible && !debug)){
	goto display;
      }
      pair = V->tissue_id;
      //and these are foreground characters
      init_pair(pair,T->fg_color,T->bg_color);

      Neighbor parent = board[i][j].parent;
      if(parent == TOP || parent == BOTTOM){
	display_char = '|';
      }
      else if(parent == TOP_RIGHT || parent == BOTTOM_LEFT){
	display_char = '/';
      }
      else if(parent == TOP_LEFT || parent== BOTTOM_RIGHT){
	display_char = '\\';
      }
      else if(parent == LEFT || parent == RIGHT){
	display_char = '_';
      }else{
	display_char = 'X'; //shouldn't happen
      }
    display:
      wattron(win,COLOR_PAIR(pair));
      mvwaddch(win,i-y,j-x,display_char);      
      wattroff(win,COLOR_PAIR(pair));
    }}
  mvwprintw(win,0,0,"Items: %d",total_items(ca));
}

void display_info(WINDOW*win, CA* ca,Viewport v,Cursor c){
  uint ca_y = c.y + v.y;
  uint ca_x = c.x + v.x;
  if(ca_y >= ca->ys || ca_x >= ca->xs)
    return;
  uint fields = 9;
  uint start_row = v.ys-fields-1;
  Cell* V = &ca->new[ca_y][ca_x];
  mvwprintw(win,start_row+0,0,"species\t\t=     %u",V->species_id);
  mvwprintw(win,start_row+1,0,"parent[0]\t=     %u",ca->world->genomes[ca->world->plantae[V->species_id].genome_id].parents[0]);
  mvwprintw(win,start_row+2,0,"parent[1]\t=     %u",ca->world->genomes[ca->world->plantae[V->species_id].genome_id].parents[1]);
  mvwprintw(win,start_row+2,0,"name\t=     %s",ca->world->genomes[ca->world->plantae[V->species_id].genome_id].name);

  
  
  return;
  
  
}

void display_gene_info(WINDOW*win,CA*ca,Viewport v,Cursor c){
  uint ca_y = c.y + v.y;
  uint ca_x = c.x + v.x;
  uint start_row = 10;
  uint start_col = 20;
  uint off = 0;

  if(ca_y >= ca->ys || ca_x >= ca->xs)
    return;
  Cell* V = &ca->new[ca_y][ca_x];
  Genome* G = &ca->world->genomes[V->species_id];
  for(int i=0;i<NC;i++){
    for(int j=0;j<NEW;j++){
      uint shares = get_shares_by_direction(G->transfer_bias[i][V->tissue_id],j);
      mvwprintw(win,start_row+YMAP[j],start_col+off+XMAP[j],"%u",shares);

    }
    off += 4;
  }
  for(int j =0;j<NEW;j++){
    uint shares = get_shares_by_direction(G->reproduction_bias[V->tissue_id],j);
    mvwprintw(win,start_row+YMAP[j],start_col+off+XMAP[j],"%u",shares);
  }
  
  return;
  
      
}

  

int transl_curs(const CA*ca,Viewport*v,Cursor*c,int y,int x){
  c->y +=      y;
  c->x += x;  
  return 0;
}

int move_curs(CA*ca,uint* sely,uint*selx,int c){
  switch(c)
    {
    case KEY_UP:
      if(*sely>0)
	(*sely)--;
      break;
    case KEY_DOWN:
      if(*sely<=ca->ys-1)
	(*sely)++;
      break;
    case KEY_LEFT:
      if(*selx>0)
	(*selx)--;
      break;
    case KEY_RIGHT:
      if(*selx<=ca->xs-1)
	(*selx)++;
      break;
    default:
      return 1;
      break;
    }
  return 0;
}

void act_worldview(Worldview_State* ws,int input){
  CA* ca = ws->ca;
  if(input == 'h')
    transl_curs(ca,&ws->v,&ws->c,0,-1);
  if(input == 'H')
    transl_curs(ca,&ws->v,&ws->c,0,-5); 
  if(input == 'j')
    transl_curs(ca,&ws->v,&ws->c,1,0);
  if(input == 'J')
    transl_curs(ca,&ws->v,&ws->c,5,0);
  if(input == 'k')
    transl_curs(ca,&ws->v,&ws->c,-1,0);
  if(input == 'K')
    transl_curs(ca,&ws->v,&ws->c,-5,0);
  if(input == 'l')
    transl_curs(ca,&ws->v,&ws->c,0,1);
  if(input == 'L')
    transl_curs(ca,&ws->v,&ws->c,0,5);
  //  if(input == 'F')
  //    make_floaty(ca,ws->c.y+ws->v.y,ws->c.x+ws->v.x);
  if(input == 'D')
      kill(ca,ws->c.y+ws->v.y,ws->c.x+ws->v.x);
}
char* pump_worldview(PANEL* panel,Worldview_State* ws){
  CA* ca = ws->ca;
  if(ca == NULL){
    return NULL;
  }
  WINDOW* win = panel_window(panel);
  uint ys,xs;
  getmaxyx(win,ys,xs);
  ws->v.ys = ys;
  ws->v.xs = xs;

  Cursor c		= ws->c;
  Viewport v		= ws->v;
  char minor_mode = ws->minor_mode;
  if(c.y > v.ys){	
    v.y += c.y-v.ys;
    c.y = v.ys-1;
    clear();
  }else if(c.y < 0){
    v.y += c.y;
    c.y = 0;
    clear();
  }
  
  if(c.x > v.xs){
    v.x += c.x-v.xs;
    c.x = v.xs-1;
    clear();
  }else if(c.x < 0){
    v.x += c.x;
    c.x = 0;
    clear();
  }
  if(minor_mode == SHOW_PLANTS){
    display_modules(win,ca,v,ws->minor_mode&DEBUG);
  }if(minor_mode == SHOW_RESOURCES){
    display_resource(win,ca,v);
  }
  if(minor_mode == SHOW_PHOTO){
    display_photosynthate(win,ca,v);
  }if(minor_mode == SHOW_HUMMUS){
    display_nutrients(win,ca,v);
  }if(minor_mode  == SHOW_WATER){
    display_water(win,ca,v);
  }if(minor_mode  == SHOW_FREE_WATER){
    display_free_water(win,ca,v);
    }if(minor_mode == SHOW_WIND){
    display_wind(win,ca,v);
  }  
  ws->c = c;
  ws->v = v;
  return NULL;
}

PANEL* init_worldview(){
  Worldview_State* ws = calloc(1,sizeof(Worldview_State));
  Frame* frame = calloc(1,sizeof(Frame));
  frame->userptr = ws;
  ws->minor_mode = SHOW_PLANTS;
  WINDOW* win = newwin(0,0,0,0);
  PANEL*panel = new_panel(win);
  frame->userptr = (void*)ws;
  set_panel_userptr(panel,(void*)frame);
  mvwprintw(win,20,20,"No world loaded...");
  return panel;
}
