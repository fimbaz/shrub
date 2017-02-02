#include "items.h"
#include <stdlib.h>
#include <string.h>
Item MOTE = {0,10,{0,0,0},'*',FLOATY,0,0,.00056};
uint total_items(CA*ca){
  uint total = 0;
  for(int i=0;i<ca->ys;i++){
    for(int j=0;j<ca->xs;j++){
      Item* items = ca->new[i][j].items;
      for(Item* item = items;item != NULL;item = item->next){
	total++;
      }
    }
  }
  return total;
}
int occupied_space(CA*ca,uint y,uint x){
  uint occupied = 0;
  if(!in_bounds(ca,y,x))
    return ca->world->item_space;
  for(Item* item = ca->new[y][x].items;item !=NULL;item = item->next){
    occupied += item->size;
  }
  return occupied;
}
int occupied_space_o(CA*ca,uint y,uint x){
  uint occupied = 0;
  if(!in_bounds(ca,y,x))
    return ca->world->item_space;
  for(Item* item = ca->old[y][x].items;item !=NULL;item = item->next){
    occupied += item->size;
  }
  return occupied;
}

int create_item(CA*ca,uint y,uint x,Item*new){
  int counter = 0;
   if(occupied_space(ca,y,x) > ca->world->item_space + new->size){
     return -1;
   }
  Item* item = calloc(1,sizeof(Item));
  memcpy(item,new,sizeof(Item));
  if(ca->new[y][x].items == NULL)
    ca->new[y][x].items = item;
  else{
    Item* old;
    for(old = ca->new[y][x].items;old->next != NULL;old = old->next) counter++;
    old->next = item;
  }
  return counter;
}
void delete_item(CA*ca,uint y,uint x,uint pos){
  Item*  prev = ca->new[y][x].items;
  if(prev == NULL)
    return;
  else if(pos == 0){
    ca->new[y][x].items = prev->next;
    free(prev);
    return;
  }
  Item* curr =   prev->next;
  for(int i=0;i<pos;i++){
    if(curr == NULL)
      return;
    prev = curr;
    curr = curr->next;
  }
  if(curr != NULL){
    prev->next = curr->next;
    free(curr);
  }
  return;
}
int move_item(CA*ca,uint y,uint x,uint old_y,uint old_x){
  Item* item = ca->old[old_y][old_x].items;
  if(item == NULL || item == (void*)0x01){
    return -1;
  }
  uint occupied = 0;
  for(Item* item = ca->old[y][x].items;item !=NULL;item = item->next){
    occupied += item->size;
  }
  for(Item* item = ca->new[y][x].items;item !=NULL;item = item->next){
    occupied += item->size;
  }

  if(occupied >= ca->world->item_space + item->size){
    y = old_y;
    x = old_x;
  }
  //  if(y == old_y && x == old_x){
  //    ca->new[y][x].items = item;
  //    return 0;
  //  }
  Item* i2;
  if(ca->new[old_y][old_x].items == NULL)
    ca->new[old_y][old_x].items = item->next;
  else{
    for(i2 = ca->new[old_y][old_x].items;i2->next != NULL;i2 = i2->next);
    i2->next = item->next;
  }
  item->next = NULL;
  if(ca->new[y][x].items == NULL)
    ca->new[y][x].items = item;
  else{
    Item* i2;
    for(i2 = ca->new[y][x].items;i2->next != NULL;i2 = i2->next);
    i2->next = item;
  }
  return 0;
}
