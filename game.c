#include "typedefs.h"
#include "genome.h"
#include "shrub.h"
#include "worldview.h"
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define DIVERSITY 200
extern Item MOTE;
Item SAMPLE_SEED[DIVERSITY+1];
PANEL* init_display(){
  initscr();    
  cbreak();
  noecho();
  timeout(100);
  keypad(stdscr,TRUE);
  start_color();
  //  init_colors();
  curs_set(1);
  int y,x;
  getmaxyx(stdscr,y,x);
  WINDOW* worldview_win  =  subwin(stdscr,y,x,0,0);
  PANEL* worldview_panel = new_panel(worldview_win);
  return worldview_panel;
}
World* new_game(){
  Experiment e1;
  Genome g1;
  srand(time(NULL));
  init_sample_run(&e1,&g1);
  World* world		= create_world(&e1);
  uint   cays		= 30;
  uint   caxs		= 30;
  float  freq		= .003;
  float  amp		= 220;
  uint   offset		= 60;
  uint   bheight	= 5;
  uint   tree_interval	= 80;
 new_perlin_zone(cays,caxs,freq,amp,offset,bheight,tree_interval,world);
 create_random_species(world,&g1,DIVERSITY);
 SAMPLE_SEED[0].type  = SEED;
 SAMPLE_SEED[0].shape = '*';
 SAMPLE_SEED[0].next = NULL;
 SAMPLE_SEED[0].mass = .0087;
 SAMPLE_SEED[0].fg_color = 18;
 SAMPLE_SEED[0].size = 100;
 for(int i=0;i<=DIVERSITY;i++){
   memcpy(&SAMPLE_SEED[i],&SAMPLE_SEED[0],sizeof(Item));
 }
 for(int i=0;i<DIVERSITY;i++){
   SAMPLE_SEED[i].item.species_id  = i;
 }
  return world;
}
void act_game(Worldview_State*ws,int input){
  int static seed_id = 0;
  if(input == 'f'){
    timeout(1);
  }
  if(input == 's'){
    seed_id = (seed_id%DIVERSITY)+1;
    create_item(ws->ca,ws->c.y+ws->v.y,ws->c.x+ws->v.x,&SAMPLE_SEED[seed_id]);
  }if(input == 'd'){
    kill_branch(ws->ca,ws->c.y+ws->v.y,ws->c.x+ws->v.x);
  }if(input >= '0' && input <= '9'){
    ws->minor_mode = input - '0';
  }
  return;
}
int main(){
  PANEL* worldview = init_display();
  World* world        = new_game();
  CA* ca              = &world->zones[0];
  Worldview_State* ws = calloc(1,sizeof(Worldview_State));
  ws->ca = ca;
  ws->c.y = 10;
  ws->c.x = 10;
  while(1){
    int input = getch();
    getmaxyx(panel_window(worldview),ws->v.ys,ws->v.xs);
    act_worldview(ws,input);
    act_game(ws,input);
    tick_CA(ca);
    pump_worldview(worldview,ws);
    update_panels();
    doupdate();
    move(ws->c.y,ws->c.x);
  }
  endwin();
  return 0;
}
