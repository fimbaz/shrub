#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include <panel.h>
#include <form.h>
#include <menu.h>
  
  

  
typedef struct Form_Item{
  char* label;
  FIELDTYPE* type;
  char* def;
  void* place;
  char** enum_strs;

}Form_Item;

char* MAIN_MENU[] = {
  "Generations",
  "Bonsai",
  "Floaties",
  "Load file",
  "Export Genome",
  "Import Genome",
  "Exit"
};


char* WORLD_STYLES[] = {
  "Simple",
  "Sine",
  "Perlin"
};

char* MODES[] = {
  "Generations",
  "Bonsai"
};  
    
  
#define GEN(cmd) form_driver(generations_form,cmd)
FORM* render_generations_form(){
  Form_Item form_items[] = {
    {"Style",TYPE_ENUM,"Perlin",NULL,WORLD_STYLES},
    {"Height",TYPE_INTEGER,"100",NULL},
    {"Width",TYPE_INTEGER,"200",NULL},
    {"Tree Spacing",TYPE_INTEGER,"50",NULL},
    {"Turns per Gen",TYPE_INTEGER,"1000",NULL},
    {"Freq",TYPE_INTEGER,"50",NULL},
    {"Amp",TYPE_INTEGER,"50",NULL}};
  WINDOW* msgwin = newwin(1,80,20,0);
  int form_size = sizeof(form_items)/sizeof(Form_Item);

  FIELD** fields = calloc(form_size*2+3,sizeof(FIELD*));
  for(int i=0;i<form_size;i++){
    fields[2*i] = new_field(1,14,i,0,0,0);
    set_field_userptr(fields[2*i+1],form_items[i].label);
    field_opts_off(fields[2*i],O_ACTIVE);

    fields[2*i+1] = new_field(1,10,i,15,0,0);
    set_field_buffer(fields[2*i],0,form_items[i].label);
    if(form_items[i].type == TYPE_ENUM){
      set_field_type(fields[2*i+1],TYPE_ENUM,form_items[i].enum_strs,0,0);
    }else{

      set_field_type(fields[2*i+1],form_items[i].type,1,1000);

    }
      set_field_buffer(fields[2*i+1],0,form_items[i].def);
  }
  //form fields 0..(form_size-1)*2 + 1 are now occupied.
  FIELD** submit_field = &fields[form_size*2];
  *submit_field = new_field(1,10,form_size,15,0,0);
  set_field_buffer(*submit_field,0,"Submit");
  set_field_fore(*submit_field,A_UNDERLINE);
  field_opts_off(*submit_field,O_EDIT);
  FORM* generations_form = new_form(fields);
  post_form(generations_form);
  refresh();
  while(1){
    FIELD* curr = current_field(generations_form);
    FIELDTYPE* type = field_type(curr);
    int c = getch();
    switch(c){
    case KEY_DOWN:
      if(E_INVALID_FIELD == GEN(REQ_VALIDATION)){
	mvwprintw(msgwin,0,0,"INVALID");
	wrefresh(msgwin);
      }else{
	GEN(REQ_NEXT_FIELD);
      }
      break;
    case KEY_UP:
      GEN(REQ_PREV_FIELD);
      break;
    case KEY_BACKSPACE:
      GEN(REQ_LEFT_CHAR);
      GEN(REQ_DEL_CHAR);
      break;
    case KEY_LEFT:
      if(type == TYPE_ENUM){
	GEN(REQ_PREV_CHOICE);
      }else{
      	GEN(REQ_LEFT_CHAR);
      }
      break;
    case KEY_RIGHT:
      if(type == TYPE_ENUM){
	GEN(REQ_NEXT_CHOICE);
      }else{
      	GEN(REQ_RIGHT_CHAR);
      }
      break;
    case '\n':
      if(curr == *submit_field)
	return generations_form;
    default:
      if(type != TYPE_ENUM)
	GEN(c);
    }
  }
  return 0;  
}
   
int render_main_menu(Window* win){
  int choice_count = sizeof(MAIN_MENU)/sizeof(char*);
  ITEM** items = calloc(choice_count+1,sizeof(ITEM*));
  int i =0;
  for(i=0;i<choice_count;i++){
    items[i] = new_item(MAIN_MENU[i],"");
  }
  items[i] = NULL;
  MENU* menu = new_menu(items);
  post_menu(menu);
  while(1){
    int c =  getch();
    switch(c){
    case KEY_DOWN:
      menu_driver(menu,REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(menu,REQ_UP_ITEM);
    case KEY_RIGHT:
      menu_driver(menu,REQ_TOGGLE_ITEM);
      break;
    }
  }
  
  return 0;
}

char* field_value(FORM*form,char* name){
  FIELD** fields = form_fields(form);
  for(int i=0;fields[i] != NULL;i++){
    char* fieldname = field_userptr(fields[i]);
    if(fieldname == NULL) continue;
    if(!strcmp(name,fieldname)){
      return field_buffer(fields[i],0);
    }
  }
  return NULL;      
}


int main()
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr,TRUE);
  FORM* form = render_generations_form();

  int y			= atoi(field_value(form,"Height"));
  int x			= atoi(field_value(form,"Width"));
  int tree_interval	= atoi(field_value(form,"Turns per Gen"));
  float f		= atof(field_value(form,"Freq"));
  float a		= atof(field_value(form,"Amp"));
  char* style = field_value(form,"Style");
  printf("%d",y);


}




  
