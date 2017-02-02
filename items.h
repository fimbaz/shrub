#ifndef INVENTORY_H
#define INVENTORY_H
#include "typedefs.h"
#include "genome.h"
typedef enum Item_Type{
  SEED,
  FLOATY,
  FRAGMENT,
  POLLEN,
}Item_Type;
typedef union It{
  char floaty;
  int species_id;
}It;

typedef struct Item{
  It item;
  uint size;
  byte nutrients[3];
  char shape;
  char reap :1;
  Item_Type type;
  struct Item* next;
  int fg_color;
  int bg_color;
  double mass;
  int decay_rate;
}Item;

uint total_items(CA*ca);
int create_item(CA*ca,uint y,uint x,Item*new);
int move_item(CA*ca,uint y,uint x,uint old_y,uint old_x);
int occupied_space(CA*ca,uint y,uint x);
void delete_item(CA*ca,uint y,uint x,uint pos);
#endif
