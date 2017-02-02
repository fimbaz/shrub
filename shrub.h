#ifndef SHRUB_H
#define SHRUB_H
#define SATURATED 5
#include "genome.h"
#include "typedefs.h"
#include "items.h"
#define SS(q,n) set_shares_by_direction(q,n)
#define MAX_DEBRIS 5


typedef enum Neighbor{
  SELF = 0,
  TOP,
  TOP_RIGHT,
  RIGHT,
  BOTTOM_RIGHT,
  BOTTOM,
  BOTTOM_LEFT,
  LEFT,
  TOP_LEFT,
  NEW, /*monkey business-- a newly created cell's offspring[0] and offspring[1] slots are set to this value
	 internally in tick_reproduction, then set back to SELF when the tick is done.  This lets us mutate
	 'new' in place.
	*/
}Neighbor;

typedef enum Substrate{
  VOID,
  GROUND,
  AIR,
  SAND,
  FLOOD,
}Substrate;
    

  
typedef struct Cell{
  byte nutrients[3];
  byte escrow[3] ;
  float flux;
  short int species_id;
  byte resource       : 4;
  byte tissue_id;
  byte substrate_type : 4;
  byte damage         : 4;
  byte water          : 3;
  byte circ           : 1;
  byte floaty         : 1;
  double wind[2];
  byte wind_done : 1;
  byte reap : 1;
  byte  parent;
  byte offspring[2];
  short unsigned int plant_id;
  struct Item* items;
  //we can add props here till we're at about 64 bytes, without worrying too much.
} Cell;


typedef struct Debris{
  byte shape;
  byte tissue : 3;
  byte nutrients[3];
  char decay;
}Debris;

typedef struct Pile{
  byte count :3;
  byte free  :3;
  Debris* debris;
}Pile;


//boolean functions return non-zero if everything is cool,
//all other functions return zero, and a negative number if there's an issue
//updated fcns
World*             create_world                 (Experiment* experiment);
Cell*              neighbor			(CA*ca,Neighbor n,uint y, uint x);
uint               neighbor_count		(CA*ca,uint y, uint x);
uint               in_bounds			(CA*ca,int y, int x);
uint               longest_dist_to_edge		(CA*ca,uint y,uint x);
uint               init_hummus			(CA*ca,uint y,uint x);
uint               alloc_CA			(World*ca,uint ys,uint xs);
void               dealloc_CA			(CA*ca);
void               dealloc_world                (World*);
uint               is_transparent		(Substrate t);
uint               is_porous			(Substrate t);
void               tick_resources		(CA*ca);
void               tick_water                   (CA*ca,uint i,uint j,uint gearing);
void               tick_wind                    (CA*ca,uint i, uint j);
void               tick_decay                   (CA*ca,uint i,uint j);
void               tick_light                   (CA*ca,int i, int j);
void               tick_hummus                  (CA*ca, uint i,uint j);
void               notick_hummus                (CA*ca, uint i, uint j);
uint               set_shares_by_direction      (short unsigned int q, Neighbor n);
void               cull				(CA*ca);
void               destroy_cell			(CA*ca,uint y,uint x);
uint               _cut				(CA*ca,uint y,uint x);
uint                cut				(CA*ca,uint y,uint x);
void               transform			(CA*ca);
void               transplant			(CA*ca,uint y, uint x);       //nondestructively move existing plant cells from 'old' to 'new'.
uint               kill_branch                  (CA*ca,uint y,uint x);
void               uptake_nutrients		(CA*ca);
void               redistribute_nutrients	(CA*ca);
void               tick_nutrients		(CA*ca); //redistributes the nutrients in 'new'.
int                tick_reproduction		(CA*ca);
void               tick_CA			(CA*ca); //advance the cellular automaton.  The latest board is held in 'new'
int                plant_seed                   (CA*ca,uint sid,uint y,uint x);
int                create_species               (World*world,Genome*g1);
void               make_floaty                  (CA*ca,uint i, uint j);
uint               new_perlin_zone              (uint y,uint x,double f,double a,uint dc,uint bheight,uint tree_interval,World*world);
int                find_ground                  (CA*ca,uint x);
uint get_shares_by_direction(short unsigned int bias,Neighbor neighbor);
void kill_cell(CA*ca,uint y, uint x);
void redistribute_water(CA*ca);
void find_next_member(CA*ca,int y, int x,int* y_out,int*x_out);
void find_prev_member(CA*ca,int y, int x,int* y_out,int*x_out);
void tick_germinate(CA*ca,uint y,uint x);
Tissue* get_tissue(CA* ca,SpeciesId s,TissueId t);
int add_genome(CA*ca,Genome*g);
uint* sum_by_species(World*world,uint zone);
//in progress

//


//deprecated fcns



#endif
