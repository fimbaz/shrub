#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "shrub.h"
#include "genome.h"
#include "perlin.h"
int main(int argc,char* argv[]){
  int x,y,turns,floaties;
  if(argc < 5){
    printf("Usage: ./sperf y x turns floaties\n");
    return 0;
  }
  sscanf(argv[1],"%d",&y);
  sscanf(argv[2],"%d",&x);
  sscanf(argv[3],"%d",&turns);
  sscanf(argv[4],"%d",&floaties);

  World* world = new_perlin_world(y,x,.005,250,50,5,80);

  world->plantae[1] = malloc(sizeof(Species));
  init_plantae(world->plantae);
  CA* ca = &world->zones[0];


  uint done = 0;
  int u =0;

  for(int i=0;i<ca->ys;i++){
    for(int j= 0;j<ca->xs;j++){
      Cell* V = &ca->new[i][j];
      if(V->substrate_type == AIR && V->parent == SELF){
	V->floaty = 1;
	u++;
	if(u == floaties){
	  done = 1;
	  break;
	}

      }
    }
    if(done)
      break;
  }

  
  struct timeval tm1,tm2,res;
  struct timeval *t1, *t2;
  t1 = &tm1;
  t2 = &tm2;
  for(int i=0;i<turns;i++){
    tick_CA(ca,1);

  }

  floaties = 0;
  uint living_cells = 0;
  for(int i=0;i<ca->ys;i++){
    for(int j= 0;j<ca->xs;j++){
      Cell* V = &ca->new[i][j];
      if(V->parent != SELF)
	living_cells++;
      if(V->floaty)
	floaties++;
    }}


  printf("Living cells: %d\n",living_cells);

  printf("floaties: %d\n",floaties);
  exit(0);
}
