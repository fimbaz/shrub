/*
All rites reversed
extruded by Matthew Charles Baker (fimbaz@gmail.com)
in the 21st century
plant your seeds.
*/
#include <stdio.h>
#include <stdlib.h>
#include "shrub.h"
#include "genome.h"
#include "perlin.h"
#include <sys/time.h>
#include <string.h>
#include <math.h>
#define REPRO_BIAS(_bias,_n) (_bias >> ((_n-1)*2) &0x03)
int XMAP[10] = {0, 0, 1,1,1,0,-1,-1,-1,0};
int YMAP[10] = {0,-1,-1,0,1,1, 1, 0,-1,0};
Neighbor RMAP[10] = {SELF,BOTTOM,BOTTOM_LEFT,LEFT,TOP_LEFT,TOP,TOP_RIGHT,RIGHT,BOTTOM_RIGHT,NEW};
uint get_shares_by_direction(short unsigned int bias,Neighbor neighbor){
  if(neighbor > 9)
    return 0;
  bias >>= (neighbor-1)*2;
  bias &= 0x03;
  if(neighbor == SELF)
    return 1;
  return bias;
}

uint set_shares_by_direction(short unsigned int q,Neighbor n){
  if(q > 3)
    return 0;
  return q << (n-1)*2;
}


uint* sum_by_species(World*world,uint zone){
  if(zone >= world->zone_count){
    return NULL;
  }
  CA* ca = &world->zones[zone];
  uint* popcount = calloc(world->max_species_id,sizeof(uint));
  for(int i=0;i<ca->ys;i++){
    for(int j=0;j<ca->xs;j++){
      Cell* V = &ca->new[i][j];
      if(V->parent != SELF){
	popcount[V->species_id]++;
      }
    }
  }
  return popcount;
  
}

 
int get_shape(CA*ca,uint i,uint j){
  Neighbor parent = ca->new[i][j].parent;
  int shape;
  if(parent == TOP || parent == BOTTOM){
    shape = '|';
  }
  else if(parent == TOP_RIGHT || parent == BOTTOM_LEFT){
    shape = '/';
  }
  else if(parent == TOP_LEFT || parent== BOTTOM_RIGHT){
    shape = '\\';
  }
  else if(parent == LEFT || parent == RIGHT){
    shape = '_';
  }else{
    shape = 'X'; //shouldn't happen
  }
  return shape;
}  
Tissue* get_tissue(CA* ca,SpeciesId s,TissueId t){
  if(s == 0)
    return NULL;
  Species* species = &ca->world->plantae[s];
  if(species == NULL)
    return NULL;
  if(t > 7 || t == 0)
    return NULL;
  Tissue* tissue = &species->tissues[t];
  if(tissue != NULL)
    return tissue;
  return NULL;
}
 
uint in_bounds(CA*ca,int y,int x){
  if(y < ca->ys && y >= 0 && x < ca->xs && x >= 0)
    return 1;
  return 0;
}

uint is_transparent(Substrate t){
  if(t == GROUND)
    return 0;
  return 1;
  
}
uint is_porous(Substrate t){
  if(t == GROUND)
    return 1;
  return 0;
}

Cell* o_neighbor(CA* ca,Neighbor n,uint y, uint x){
  Board* b = ca->old;
  
  uint ny = y+YMAP[n];
  uint nx = x+XMAP[n];
  if(ny >= ca->ys || nx >=ca->xs){
    return NULL;
  }
  return &b[ny][nx];
}

    
Cell* neighbor(CA* ca,Neighbor n,uint y, uint x){
  Board* b = ca->new;
  
  uint ny = y+YMAP[n];
  uint nx = x+XMAP[n];
  if(ny >= ca->ys || nx >=ca->xs){
    return NULL;
  }
  return &b[ny][nx];
}



uint neighbor_count(CA*ca,uint y, uint x){
  //counts occupied neighbors of cell y,x.  A cell is its own neighbor.
  //a cell is considered occupied if it is not its own parent.
  uint occupied_cells = 0;
  for(uint n=1;n<9;n++){
    Cell* c = neighbor(ca,n,y,x);
    if(c == NULL)
      continue;
    if(c->parent)
      occupied_cells++;
  }
  return occupied_cells;
}




void find_next_member(CA*ca,int y, int x,int* y_out,int*x_out){
  Cell* V = &ca->new[y][x];      
  Cell* V1 = NULL;
  int x1 = -1; int y1= -1;
  int done = 0;
  
  for(int j=x;j<ca->xs;j++){
    for(int i=0;i<ca->ys;i++){
      V1 = &ca->new[i][j];
      if(V1->species_id == V->species_id && V1->plant_id != V->plant_id){
	y1 = i;
	x1 = j;
	done = 1;
	break;
      }      
    }
    if(done)
      break;
  }

  if(y1 >= 0 && x1 >= 0){
    for(Cell* V2=V1;!(V2->circ) && V2->parent != SELF;V2=neighbor(ca,V2->parent,y1,x1)){
      
      y1 = YMAP[V2->parent]+y1;
      x1 = XMAP[V2->parent]+x1;
    }
  }
  *y_out = y1;
  *x_out = x1;
	
  return;
}

void find_prev_member(CA*ca,int y, int x,int* y_out,int*x_out){
  Cell* V = &ca->new[y][x];      
  Cell* V1 = NULL;
  int x1 = -1; int y1= -1;
  int done = 0;
  
  for(int j=x;j>=0;j--){
    for(int i=0;i<ca->ys;i++){
      V1 = &ca->new[i][j];
      if(V1->species_id == V->species_id && V1->plant_id != V->plant_id){
	y1 = i;
	x1 = j;
	done = 1;
	break;
      }      
    }
    if(done)
      break;
  }

  if(y1 >= 0 && x1 >= 0){
    for(Cell* V2=V1;!(V2->circ) && V2->parent != SELF;V2=neighbor(ca,V2->parent,y1,x1)){
      y1 = YMAP[V2->parent]+y1;
      x1 = XMAP[V2->parent]+x1;
    }
  }
  *y_out = y1;
  *x_out = x1;
	
  return;
}


uint longest_dist_to_edge(CA*ca,uint y,uint x){
  uint path1 = 0;
  uint path2 = 0;
  Board* new = ca->new;
  if(y>=ca->ys || x>=ca->xs)
    return 0;
      
  if((new[y][x].offspring[0] == SELF && new[y][x].offspring[1] == SELF) ||(new[y][x].offspring[0] == NEW && new[y][x].offspring[1] == NEW)){
    return 1;
  }

  
  if(new[y][x].offspring[0] != SELF && !new[y][x].circ){
    path1++;
    path1 += longest_dist_to_edge(ca,y+YMAP[new[y][x].offspring[0]],
				  x+XMAP[new[y][x].offspring[0]]);
  }
  if(new[y][x].offspring[1] != SELF){
    path2++;
    path2 += longest_dist_to_edge(ca,y+YMAP[new[y][x].offspring[1]],
				  x+XMAP[new[y][x].offspring[1]]);
  }
  return path1>path2?path1:path2;
}

uint longest_dist_to_sprout(CA*ca,uint y,uint x){
  if(y>=ca->ys || x>=ca->xs)
    return 0;
  int count = 0;
  for(Cell* V = &ca->new[y][x];V->circ == 0; V = &ca->new[y][x]){
    if(V == NULL || V->parent == SELF)
      return 0;
    y = y + YMAP[V->parent];
    x = x + XMAP[V->parent];
    count++;
  }
  return count;
}

int find_ground(CA*ca,uint x){
  if(x >= ca->xs)
    return -1;
  for(int i=0;i<ca->ys;i++){
    if(ca->old[i][x].substrate_type == GROUND){
      return i;
    }    
  }
  return -1;
}

World* create_world(Experiment* experiment){
  World* world		= calloc(1,sizeof(World));
  world->genomes	= calloc(100,sizeof(Genome));
  world->allocd_genomes = 100;
  world->plantae	= calloc(100,sizeof(Species));
  world->experiment	= calloc(1,sizeof(Experiment));
  world->item_space = 100;
  world->zones		= calloc(10,sizeof(CA));
  world->allocd_zones	= 10;
  memcpy(world->experiment,experiment,sizeof(Experiment));  

  world->max_species_id     = 1;
  gettimeofday(&world->last_tick_time,NULL);
  world->tick_speed_msec = 100;
  return world;
}

void dealloc_world(World*world){
  free(world->genomes);
  free(world->plantae);
  free(world->experiment);
  for(int i=0;i<world->zone_count;i++){
    dealloc_CA(&world->zones[i]);
  }
  free(world->zones);
  return;
}

uint new_perlin_zone(uint y,uint x,double f,double a,uint dc,uint bheight,uint tree_interval,World* world){
  uint zone = alloc_CA(world,y,x);
  CA* ca = &world->zones[zone];
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
	  ca->new[i][j].substrate_type = AIR;
    }}
  perlin_init();
  for(int j=0;j<ca->xs;j++){
    uint ground_level =dc + (uint)fabs(round(a*noise1(j*f)));
    for(int i=ground_level < ca->ys-bheight ? ground_level : ca->ys-bheight;i<ca->ys;i++){
      ca->new[i][j].substrate_type = GROUND;
      ca->old[i][j].substrate_type = GROUND;
      ca->new[i][j].resource       = 0;
      ca->new[i][j].water          = 0;
      
    }

  }
  
  for(int i=0;i<ca->ys;i++){
    for(int j=0;j<ca->xs;j++){
	if(!is_transparent(ca->new[i][j].substrate_type))
	  ca->new[i][j].resource       = 1;
	
      }
    
  }
    
  ca->rain_level = 100;
  ca->total_water = 100000;
  ca->ground_water = 0;
  return zone;
}


  


uint alloc_CA(World*world,uint ys,uint xs){
  uint zone_id =0;
  for(zone_id=0;zone_id<100;zone_id++){
    if(world->zones[zone_id].ys  == 0)
      break;
  }
  if(zone_id >= world->allocd_zones){
    world->allocd_zones *=2;
    world->zones = realloc(world->zones,sizeof(CA)*world->allocd_zones);
  }
  
      
  CA* ca = &world->zones[zone_id];
  ca->new = calloc(ys,sizeof(Cell*));
  ca->old = calloc(ys,sizeof(Cell*));
  ca->ys = ys;
  ca->xs = xs;

  for(uint i=0;i<ys;i++){
    ca->new[i] = calloc(xs,sizeof(Cell));
    ca->old[i] = calloc(xs,sizeof(Cell));
  }
  ca->world = world;
  
  ca->world->zone_count++;
  return zone_id;
}

void dealloc_CA(CA*ca){
  for(int i=0;i<ca->ys;i++){
    free(ca->new[i]);
    free(ca->old[i]);
  }
  free(ca->new);
  free(ca->old);
  return;
  
}


int in_genomes(World*w,Genome* g){
  int genome_data_size = sizeof(*g) - sizeof(g->remarks) - sizeof(g->name) - sizeof(g->genome_id);
  for(int i=0;i<w->max_species_id;i++){
    if(!memcmp(&w->genomes[i],g,genome_data_size))
      return i;
  }
  return 0;
}
  
//don't forget to initialize hummus in init function
int create_species(World*world,Genome*g1){
  int genome_id = in_genomes(world,g1);
  if(genome_id >=1){
    return genome_id;
  }
  if(world->max_species_id >= world->allocd_genomes){
    world->allocd_genomes *= 2;
    world->genomes = realloc(world->genomes,sizeof(Genome)*world->allocd_genomes);
    world->plantae = realloc(world->plantae,sizeof(Species)*world->allocd_genomes);
  }
  Species* s = &world->plantae[world->max_species_id];
  world->plantae[world->max_species_id].tissues =  calloc(10,sizeof(Tissue));
  apply_genome(world->experiment,g1,&world->plantae[world->max_species_id]);
  memcpy(&world->genomes[world->max_species_id],g1,sizeof(Genome));
  
  s->count = 0;
  s->allocd = 10;
  s->plant_ids = calloc(10,sizeof(uint));
  s->species_id = world->max_species_id; // redundant, I s'pose.
  s->genome_id = world->max_species_id;
  return world->max_species_id++;
}

//AFTER tick_wind
void tick_flowers(CA*ca,uint y, uint x){
  Cell * V = &ca->new[y][x];
  if(V->parent == SELF)
    return;
  if(V->tissue_id == FLOWER_FEMALE && V->items != NULL && V->items->type == POLLEN &&
     V->species_id == o_neighbor(ca,V->parent,y,x)->species_id){
    Genome g1,g2;
    Genome* G = NULL;
    combine_genomes(&ca->world->genomes[V->species_id],&ca->world->genomes[V->items->item.species_id],&g1,&g2);
    if(rand()%2){
      G = &g1;
    }
    else {
      G = &g2;
    }
    int sid = create_species(ca->world,G);
    V->species_id = sid;
    delete_item(ca,y,x,0);
  }
  if(V->tissue_id == FLOWER_FEMALE && V->species_id != o_neighbor(ca,V->parent,y,x)->species_id){
    V->reap = 1;
  }

  if(V->tissue_id == FLOWER_MALE && V->items == NULL &&
    V->nutrients[HUMMUS] >=1 && V->nutrients[PHOTO] >= 1){
    Item pollen = {0};
    pollen.type = POLLEN;
    pollen.item.species_id = V->species_id;
    pollen.nutrients[WATER] = 1;
    pollen.nutrients[PHOTO] = 1;
    pollen.nutrients[HUMMUS] = 1;
    pollen.mass = .0018;
    pollen.shape = '.';
    if(create_item(ca,y,x,&pollen) != -1 && rand()%2){
      V->nutrients[HUMMUS]--;
      V->nutrients[PHOTO]--;
    }
  }
  return;
}
int has_living_neighbors(CA*ca,uint y,uint x){
  for (int i=0;i<10;i++){
    int yc = y+1+YMAP[i];
    int xc = x+XMAP[i];
    if(in_bounds(ca,yc,xc) &&
       ca->old[yc][xc].parent != SELF)
      return 1;
  }
  return 0;
}

void tick_germinate(CA*ca,uint y,uint x){
  Cell* O = &ca->new[y][x];
  int pos = 0;
  Item*items;
  for(items = O->items;items != NULL;items = items->next){
    if(items->type == SEED){
      if(!in_bounds(ca,y+1,x) || ca->old[y+1][x].substrate_type != GROUND
	 || has_living_neighbors(ca,y+1,x))
	return;
      plant_seed(ca,items->item.species_id,y+1,x);
      delete_item(ca,y,x,pos);
      return;
    }
    pos++;
  }

}

void tick_flower(CA*ca,uint y, uint x){
  return;
}
int plant_seed(CA*ca,uint sid,uint y,uint x)
{
  //we may make this more complex in the future. ex can't plant a cactus in regular soil, certain nutrient requirements.
  if(!in_bounds(ca,y,x) || ca->old[y][x].substrate_type != GROUND || ca->old[y][x].substrate_type != GROUND || sid == 0)
    return -1;
  if(sid > ca->world->max_species_id){
    printf("invalid plant");
    exit(23);
  }
  if(sid == 0){
    printf("invalid plant 0");
    exit(23);
    
  }
  
  if(sid >= ca->world->max_species_id || sid == 0){
    printf("BAD TOUCH %d,max is %d",sid,ca->world->max_species_id);
      exit(23);
  }

  Tissue* t1 = get_tissue(ca,sid,ROOT);
  Tissue* t2 = get_tissue(ca,sid,SHOOT); 
  Cell* seed1 =&ca->new[y][x];
  Cell* seed2 =&ca->new[y-1][x];
  seed1->parent = TOP;
  seed2->parent = BOTTOM;
  seed1->tissue_id = ROOT;
  seed2->tissue_id = SHOOT;
  seed1->flux = t1->initial_flux;
  seed2->flux = t2->initial_flux;
  seed1->offspring[0] = TOP;
  seed2->offspring[0] = BOTTOM;
  seed1->nutrients[HUMMUS] = 10;
  seed1->circ = 1;
  seed2->circ = 1;
  seed1->species_id = sid;


  seed2->species_id = sid;
  seed1->plant_id = ca->world->max_plant_id;
  seed2->plant_id = ca->world->max_plant_id;
  Species* s =   &ca->world->plantae[sid];
  s->plant_ids[s->count] = ca->world->max_plant_id;
  s->count++;
  if(s->count >= s->allocd){
    s->allocd *=2;
    s->plant_ids = realloc(s->plant_ids,s->allocd*sizeof(uint));
  }
  ca->world->max_plant_id++;
  
  return 0;
}

void just_wind(CA*ca,uint ticks){
    Board* tmp = ca->new;
    ca->new = ca->old;
    ca->old = tmp;
  for(int j=0;j<ca->xs;j++){for(int i=0;i<ca->ys;i++){
      ca->new[i][j].substrate_type = ca->old[i][j].substrate_type;
    }}
  for(int t=0;t<ticks;t++){
    Board* tmp = ca->new;
    ca->new = ca->old;
    ca->old = tmp;
    for(int j=0;j<ca->xs;j++){for(int i=0;i<ca->ys;i++){
	tick_wind(ca,i,j);
      }}
    ca->tick++;
  }
  return;
}
void unmark_cell(CA*ca,uint i,uint j){
  Cell* C = &ca->new[i][j];
  if(C->offspring[0] == NEW){
    C->offspring[0] = SELF;
  }
  if(C->offspring[1] == NEW){
    C->offspring[1] = SELF;
  }
  return;

}


void tick_CA(CA*ca){
  struct timeval start_time;
  gettimeofday(&start_time,NULL);
  Board* tmp = ca->new;
  ca->new = ca->old;
  ca->old = tmp;
  for(int j=0;j<ca->xs;j++){for(int i=0;i<ca->ys;i++){
      ca->new[i][j].substrate_type = ca->old[i][j].substrate_type;
      transplant(ca,i,j);
      unmark_cell(ca,i,j);

      tick_light(ca,i,j);
      tick_water(ca,i,j,100);
      if(ca->tick % 20)
	notick_hummus(ca,i,j);
      else
	tick_hummus(ca,i,j);
      tick_wind(ca,i,j);
      tick_germinate(ca,i,j);

      tick_flowers(ca,i,j);      
    }}
  for(int j=0;j<ca->xs;j++){for(int i=0;i<ca->ys;i++){
      tick_decay(ca,i,j);
    }}
  //  if(ca->tick%5 == 0)
  //  else
  //    notick_hummus(ca);
  //  if(ca->tick%10 == 0){
  cull(ca);
  uptake_nutrients(ca);
  redistribute_nutrients(ca);
  redistribute_water(ca);
  //  }
  transform(ca);

  tick_reproduction(ca);
 
  for(int i=0;i<ca->ys;i++){
    memset(ca->old[i],0,sizeof(Cell)*ca->xs);
  } 
  gettimeofday(&ca->world->last_tick_time,NULL);
  timersub(&ca->world->last_tick_time,&start_time,&ca->world->last_tick_duration);
  ca->tick++;
  
}

void tick_decay(CA*ca,uint i,uint j){
  Cell* V = &ca->new[i][j];
  Cell* downstairs = neighbor(ca,BOTTOM,i+1,j);
  if(downstairs == NULL || downstairs->substrate_type != GROUND)
    return;
  int pos = 0;
  for(Item* item = V->items;item != NULL;item = item->next){
    if(item->decay_rate > 0){
      if(!(rand()%item->decay_rate)){
	downstairs->resource += item->nutrients[HUMMUS];
	for(int p =0;p<item->nutrients[PHOTO];p++){
	  if(!(rand()%10)){
	    downstairs->resource++;
	  }
	}
	if(item->type != FRAGMENT){
	  printf("Kinda shouldn't happen");
	}
	delete_item(ca,i,j,pos);
      }
    }
    pos++;
  }
  return;
}

void make_floaty(CA*ca,uint i,uint j){
  if(in_bounds(ca,i,j))
     ca->new[i][j].floaty = 1;
}



void tick_wind(CA*ca,uint i,uint j){
  Cell* V = &ca->new[i][j];
  Cell* O = &ca->old[i][j];
  uint tick = ca->tick % 1000;
  if(!in_bounds(ca,i,j) || V->wind_done)
    return;
  V->wind_done = 1;
  double vec[2];
  double freq[2];
  freq[1] = .0187*sin(tick*3.14/400) + .00001;
  freq[0] = .0187*sin(tick*3.14/400) + .00001;
  if(j == 0){//was .0065
    vec[1] = i*freq[1]; vec[0] = tick*freq[1];
    double noise = noise2(vec);
    V->wind[0] = noise > 0 ? noise : noise*-1;
  }else if(j == ca->xs-1){
    vec[1] = i*freq[0]; vec[0] = tick*freq[0];
    double noise = noise2(vec);
    V->wind[1] = noise > 0 ? noise : noise*-1;
  }else {
    if(!is_transparent(V->substrate_type)){
      V->wind[0] = ca->old[i][j-1].wind[0]*.98;
      V->wind[1] = ca->old[i][j+1].wind[1]*.98;
    }else{
      double f1 = 1; double f2 = 1;
      double old1 = ca->old[i][j-1].wind[0];
      double old2 = ca->old[i][j+1].wind[1];
      if(fabs(old1) < .1)
	f1 = 1.5;
      if(fabs(old2) < .1)
	f2 = 1.5;
      V->wind[0] = old1*f1;
      V->wind[1] = old2*f2;
    }
  }
  //  uint ground = find_ground(ca,j);
  //  uint dist2ground = abs(i-ground);
  //  if(dist2ground > 5 || V->substrate_type == GROUND) dist2ground = 1000000000;
  //  V->wind[0] *= (1+ 1/(1+dist2ground*dist2ground));
  //  V->wind[1] *= (1+ 1/(1+dist2ground*dist2ground));
  if(O->items){
    Neighbor max_wind_n = 0;
    double max_wind = 0;
    Cell* new_N = NULL;
    Cell* old_N;
    for(int n=1;n<9;n++){
      old_N = o_neighbor(ca,n,i,j);
      if(old_N != NULL){
	double wind = fabs(old_N->wind[0] - old_N->wind[1]);
	if(wind >= max_wind && old_N->substrate_type == AIR){
	  max_wind_n = n;
	  max_wind  = wind;
	}
      }
    }
    if(max_wind - fabs(O->wind[0] - O->wind[1]) < O->items->mass){
      max_wind_n = BOTTOM;
    }
    uint culprit = 0;
    for(int n =1;n<9;n++){
      uint n_i = YMAP[max_wind_n]+i;
      uint n_j = XMAP[max_wind_n]+j;
      Cell* N_N = neighbor(ca,n,n_i,n_j);
      if(N_N != NULL && !N_N->wind_done){
	tick_wind(ca,n_i,n_j);
	if(V->items && culprit == 0)
	  culprit = n;


      }
    }
    new_N = neighbor(ca,max_wind_n,i,j);
    old_N = o_neighbor(ca,max_wind_n,i,j);

    if(in_bounds(ca,YMAP[max_wind_n]+i,XMAP[max_wind_n]+j))
      tick_wind(ca,YMAP[max_wind_n]+i,XMAP[max_wind_n]+j);
    
    uint new_N_y = YMAP[max_wind_n]+i;
    uint new_N_x = XMAP[max_wind_n]+j;  
    if( new_N == NULL ||
	(!(new_N->tissue_id == FLOWER_FEMALE && O->items->type == POLLEN) &&
	 (old_N->parent != SELF || old_N->substrate_type == GROUND))){
      new_N = neighbor(ca,SELF,i,j);
      new_N_y = i;
      new_N_x = j;  
      
    }
    move_item(ca,new_N_y,new_N_x,i,j);
  }
    
    
}


void redistribute_water(CA*ca){
  Board* new = ca->new;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &new[i][j];
      if((new[i][j].substrate_type == GROUND && new[i][j].water != 0) || ((in_bounds(ca,i+1,j) && new[i+1][j].substrate_type == GROUND) && new[i][j].water != 0)){
	uint water = V->water;
	V->water = 0;
	while(water){
	  int entropy = rand();
	  if(V->substrate_type == AIR){
	    Cell* L=neighbor(ca,BOTTOM_LEFT,i,j);
	    Cell* R=neighbor(ca,BOTTOM_RIGHT,i,j);

	    if(L!= NULL && L->substrate_type == AIR && entropy%2 && L->water < SATURATED){
	      if(water >= SATURATED && entropy%20){
		L->substrate_type = GROUND;
		Cell* E = V->substrate_type == AIR ? &new[i+1][j] : V;
		E->substrate_type = AIR;
		L->substrate_type = GROUND;
		break;
	      }
	      L->water++;
	      water--;
	      continue;
	    }
	    if(R != NULL && R->substrate_type == AIR && entropy%2 && R->water < SATURATED){
	      if(water >= SATURATED && entropy%20){
		L->substrate_type = GROUND;
		Cell* E = V->substrate_type == AIR ? &new[i+1][j] : V;
		E->substrate_type = AIR;
		L->substrate_type = GROUND;
		break;
	      }

	      R->water++;
	      water--;
	      continue;
	    }

	  }
	  Neighbor n_recipient = entropy%9;
	  if(n_recipient == 9 && V->water){
	    water--;
	    ca->ground_water--;
	    continue;
	  }

	  uint y_recipient = YMAP[n_recipient]+i;
	  uint x_recipient = XMAP[n_recipient]+j;
	  
	  Cell* recipient = neighbor(ca,n_recipient,i,j);
	  if(recipient == NULL || recipient->water > SATURATED || (V->water < SATURATED &&  recipient->substrate_type == AIR && (in_bounds(ca,y_recipient,x_recipient) && (!in_bounds(ca,y_recipient+1,x_recipient) ||  new[y_recipient+1][x_recipient].substrate_type == AIR)))){
	    if(n_recipient == TOP && new[y_recipient+1][x_recipient].substrate_type == AIR){
	      
	      water--;
	      ca->ground_water--;
	      continue;
	    }else{
	      continue;
	    }
	  }
	  if(recipient == NULL || recipient->water > new[i][j].water)
	    continue;
	  recipient->water++;
	  water--;
	}
      }
    }}
  return;
  
}
    

void tick_water(CA*ca,uint i,uint j,uint gearing){
  Board* old = ca->old;
  Cell* V = &ca->new[i][j];
  //if your BOTTOM neighbor is GROUND or SATURATED AIR and you are UNSATURATED AIR, then add your TOP neighbor's water to your own.
  if(i==0 && V->substrate_type == AIR){
    if(ca->rain_level != 0 && ca->ground_water < ca->total_water &&
       rand()%(257-ca->rain_level) == 0){
      V->water = 1;
      ca->ground_water++;
    }
  }else if(in_bounds(ca,i+1,j) && 
	   (old[i+1][j].substrate_type == GROUND     || 
  	      old[i+1][j].water          >= SATURATED) &&
	      old[i][j].substrate_type   == AIR        && 
	      old[i][j].water            < SATURATED){
    V->water = old[i][j].water + old[i-1][j].water;
  }
  //if your TOP neighbor is AIR and you are UNSATURATED AIR (and your bottom neighbor is *not* GROUND) then take on your TOP neighbor's water value.
  else if(in_bounds(ca,i-1,j) && gearing &&
	  old[i-1][j].substrate_type == AIR && 
	  old[i][j].substrate_type   == AIR && 
	  old[i][j].water            <  SATURATED){
    V->water = old[i-1][j].water;
  }
  //otherwise take on your old water value.
  else{
    V->water=old[i][j].water;
  }
  return;
}
void tick_light(CA*ca,int i, int j){
  Board* new = ca->new;
  Board* old = ca->old;
  Cell* V = &new[i][j];
  if(i == 0 && is_transparent(V->substrate_type)){
    V->resource = 1;
  }else if(in_bounds(ca,i,j) && is_transparent(V->substrate_type)){
    V->resource = old[i-1][j].resource;
  }
  return;
}

void tick_hummus(CA*ca,uint i,uint j){
  Board* old = ca->old;
  if(is_porous(old[i][j].substrate_type)){
    char resource = old[i][j].resource;
    while(resource){
      Neighbor n_recipient = rand()%9;
      Cell* recipient = neighbor(ca,n_recipient,i,j);
      if(recipient == NULL || !is_porous(old[i+YMAP[n_recipient]][j+XMAP[n_recipient]].substrate_type))
	continue;
      recipient->resource++;
      resource--;
    }
  }

  return;
}

void notick_hummus(CA*ca,uint i, uint j){
      Board* new = ca->new;
      Board* old = ca->old;
      if(is_porous(new[i][j].substrate_type)){
	new[i][j].resource += old[i][j].resource;
      }
  return;
}
void cull(CA*ca){
  Board* new = ca->new;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &new[i][j];
      Tissue* t = get_tissue(ca,V->species_id,V->tissue_id);
      if(V->reap)
	kill_branch(ca,i,j);
      if(t == NULL || t->flux_cost == 0)
	continue;
      if(V->damage > t->max_damage)
	kill_branch(ca,i,j);
      if(V->flux <= 0001)
	kill_branch(ca,i,j);

      if(V->flux){
	if(V->flux <= t->flux_cost){
	  V->flux = 0;
	}else{
	  V->flux -= t->flux_cost;
	}
      }
     
  }}
  return;
}

void transplant(CA*ca,uint i, uint j){
  Cell** old = ca->old;
  Cell** new = ca->new;
  /*this function moves all Module resources from 'old' for tick t-1 to 'new' for tick t*/
      Cell* o = &old[i][j];
      Cell* n = &new[i][j];
      if(o->parent == SELF)
	return;
      if(o->parent == SELF && (o->offspring[0] != SELF || o->offspring[1] != SELF)){
	exit(10);
      }
      n->parent            = o->parent;
      n->offspring[0]      = o->offspring[0];
      n->offspring[1]      = o->offspring[1];
      n->nutrients[HUMMUS] = o->nutrients[HUMMUS];
      n->nutrients[PHOTO]  = o->nutrients[PHOTO];
      n->nutrients[WATER]  = o->nutrients[WATER];
      n->tissue_id         = o->tissue_id;
      n->species_id        = o->species_id;
      n->flux              = o->flux;
      n->damage            = o->damage;
      n->circ              = o->circ;
      n->plant_id          = o->plant_id;
  return;
}

void destroy_cell(CA*ca,uint y, uint x){
  //could have odd reactions with different tissue types in new resource propagation scheme, keep an eye on this.
  Cell* c = &ca->new[y][x];
  c->nutrients[PHOTO] = 0;
  c->nutrients[HUMMUS]     = 0;
  c->parent        = SELF;
  c->offspring[0]  = SELF;
  c->offspring[1]  = SELF;
  c->tissue_id  = NONE;
  return;
}


uint _cut(CA*ca,uint y, uint x){
  uint sum = 0;
  Cell* V = &ca->new[y][x];
  if(y >= ca->ys || x >=ca->xs || V->parent == SELF){
    return sum;
  }
  Neighbor o0 = V->offspring[0];
  Neighbor o1 = V->offspring[1];
  if(o0 != SELF)
    sum += _cut(ca,y+YMAP[o0],x+XMAP[o0]);
  if(o1 != SELF)
    sum += _cut(ca,y+YMAP[o1],x+XMAP[o1]);
  destroy_cell(ca,y,x);
  sum +=1; 
  return sum;
}


void kill_cell(CA*ca,uint y, uint x){
  //could have odd reactions with different tissue types in new resource propagation scheme, keep an eye on this.
  Cell* c = &ca->new[y][x];
  Item it = {0};
  it.type = FRAGMENT;
  it.size = 25;
  //memcpy(it.nutrients,c->nutrients,3);
  Tissue* t = get_tissue(ca,c->species_id,c->tissue_id);
  it.fg_color = 226;//t->fg_color;
  it.bg_color = t->bg_color;
  it.shape = get_shape(ca,y,x);
  it.next = NULL;
  it.mass = .0156;
  it.nutrients[PHOTO] = c->nutrients[PHOTO];
  it.nutrients[HUMMUS] = c->nutrients[HUMMUS];
  it.nutrients[WATER] = c->nutrients[WATER];
  it.decay_rate = t->decay_rate;
  Cell* O_N  = o_neighbor(ca,c->parent,y,x);
  if(c->tissue_id == FLOWER_FEMALE && c->tissue_id != O_N->tissue_id && O_N->tissue_id != 0){
    it.item.species_id = c->species_id;
    it.type = SEED;
    it.decay_rate = 0;
    it.shape = '&';
  }
  create_item(ca,y,x,&it);
  c->nutrients[PHOTO]     = 0;
  c->nutrients[HUMMUS]    = 0;
  c->nutrients[WATER]     = 0;
  c->parent        = SELF;
  c->offspring[0]  = SELF;
  c->offspring[1]  = SELF;
  c->damage = 0;
  c->tissue_id  = NONE;
  c->species_id = 0;
  c->reap  = 0;
  return;
}


uint _kill(CA*ca,uint y, uint x){
  uint sum = 0;
  Cell* V = &ca->new[y][x];
  if(y >= ca->ys || x >=ca->xs || V->parent == SELF){
    return sum;
  }
  Neighbor o0 = V->offspring[0];
  Neighbor o1 = V->offspring[1];
  if(o0 != SELF && o0 != NEW)
     sum += _kill(ca,y+YMAP[o0],x+XMAP[o0]);
  if(o1 != SELF && o0 != NEW)
    sum += _kill(ca,y+YMAP[o1],x+XMAP[o1]);
  kill_cell(ca,y,x);
  sum +=1; 
  return sum;
}

uint kill_branch(CA*ca,uint y,uint x){
  Cell* V = &ca->new[y][x];
  Cell* P = neighbor(ca,V->parent,y,x);
  uint parent_y = (y+YMAP[V->parent]);
  uint parent_x = (x+XMAP[V->parent]);
  //inform parent of its child's incipient demise
    if(YMAP[P->offspring[0]]+parent_y == y && XMAP[P->offspring[0]]+parent_x == x)
      P->offspring[0] = SELF;
    else if(YMAP[P->offspring[1]]+parent_y == y && XMAP[P->offspring[1]]+parent_x == x)
      P->offspring[1] = SELF;
    else
      exit(20); //inconsistency in plant, bail out.
   return _kill(ca,y,x);  
}



uint cut(CA*ca,uint y,uint x){
  Cell* V = &ca->new[y][x];
  Cell* P = neighbor(ca,V->parent,y,x);
  uint parent_y = (y+YMAP[V->parent]);
  uint parent_x = (x+XMAP[V->parent]);
  //inform parent of its child's incipient demise
    if(YMAP[P->offspring[0]]+parent_y == y && XMAP[P->offspring[0]]+parent_x == x)
      P->offspring[0] = SELF;
    else if(YMAP[P->offspring[1]]+parent_y == y && XMAP[P->offspring[1]]+parent_x == x)
      P->offspring[1] = SELF;
    else
      exit(20); //inconsistency in plant, bail out.
   return _cut(ca,y,x);
   
  
}


//these rules are likely to become more general and more complicated, as the Species struct,err, 'evolves'.
void transform(CA*ca){
  Board*new = ca->new;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell*V = &new[i][j];
      Tissue* t = get_tissue(ca,V->species_id,V->tissue_id);
      if(t == NULL)
	continue;
      uint dist = longest_dist_to_edge(ca,i,j);
      if(dist >= t->evolve_threshold[0] && t->evolve_type[0] != NONE)
	V->tissue_id = t->evolve_type[0];
      dist = longest_dist_to_sprout(ca,i,j);
      if(dist > t->evolve_threshold[1] && t->evolve_type[1] != NONE){
	V->tissue_id = t->evolve_type[1];
      }
    }}
return;
}

void uptake_nutrients(CA*ca){
  Board* new = ca->new;
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &new[i][j];
      if(V->parent == SELF)
	continue;
      Tissue* t = get_tissue(ca,V->species_id,V->tissue_id);
      if(t == NULL)
	continue;
      if(V->water && V->nutrients[WATER] < t->max[WATER]){
	V->water--;
	V->nutrients[WATER]++;
	//	V->damage++;
      }
      if(V->resource){
	if(V->substrate_type == GROUND && V->nutrients[HUMMUS] < t->max[HUMMUS]){
	  V->nutrients[HUMMUS]++;
	}else if(t->photosynthetic && V->substrate_type == AIR && V->nutrients[PHOTO] < t->max[PHOTO] && V->nutrients[WATER] > 0){
	  V->damage++;
	  V->nutrients[PHOTO]++;
	  V->nutrients[WATER]--;
	  V->flux++;
	}
	V->resource--;
      }
    }}
  return;
}

uint is_alive(Cell* c){
  if(c->parent == SELF || c->species_id == 0 || c->tissue_id == 0)
    return 0;
  return 1;
}

/*
  randomly redistribute the resources represented by 'in' to 'outs', weighted by 'shares'.
  which are not to exceed 'maxes'.
  'shares' is a number between 0 and 3.  If shares(n) is zero, then out(n) is zero.
  
 */
uint create_wheel(const uint* tickets, uint total_tickets){
  uint wheel = 0;
  for(uint i=0;i<3;i++){
    for(uint j=0;j<tickets[i];j++){
      wheel = (wheel << 2) | i;
    }
  }
  return wheel;
}
void allocate_nutrients(const uint* maxes,uint tickets[3],uint in,uint outs[3]){
  //at the end, sum(outs) == in.
  //first idea:

  //  unsigned int entropy = rand();
  uint total_tickets = 0;
  for(int i=0;i<3;i++){
    if(maxes[i] == 0)
      tickets[i] = 0;
    total_tickets += tickets[i];
  }
  if(total_tickets == 0)
    return;
  unsigned int wheel = create_wheel(tickets,total_tickets);
  for(int i=0;i<in;i++){
    //spin is a random value between 0 and total_tickets.
    uint spin = rand() % total_tickets;
    uint winner = (wheel >> (spin << 2)) & 0x03;
    outs[winner]++;    
    if(outs[winner] >= maxes[winner]){
      total_tickets -= tickets[winner];
      tickets[winner] = 0;
      wheel = create_wheel(tickets,total_tickets);
    }
  }
  return;
}

void redistribute_nutrients(CA*ca){
  Board* new = ca->new;
  //this needs to suck less.
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &new[i][j];
      for(uint n=PHOTO;n<=WATER;n++){
	uint maxes[3]    = {0};
	uint shares[3]   = {0};
	uint outs[3]     = {0};
	Cell* cells[3]   = {0};
	Neighbor neighbors[3] = {0};
	if(V->parent == SELF)
	  continue;

	cells[0] = neighbor(ca,V->offspring[0],i,j);
	neighbors[0] = V->offspring[0];

	cells[1] = neighbor(ca,V->offspring[1],i,j);
	neighbors[1] = V->offspring[1];

	cells[2] = V->offspring[0] != V->parent ? neighbor(ca,V->parent,i,j) : V;
	neighbors[2] = V->parent;
	Tissue* t = get_tissue(ca,V->species_id,V->tissue_id);

	if(V->nutrients[n] <= t->min[n])	  
	  continue;

	if(V->parent == NEW || V->offspring[0] == NEW || V->offspring[1] == NEW){
	  printf("COMPUTER ALOERT!");
	}
	uint holes = 0;
	for(int c=0;c<3;c++){
	  if(cells[c] == NULL || cells[c] == V){
	    maxes[c] = 0;
	    shares[c] = 0;

	  }else{
	    Tissue* tissue = get_tissue(ca,cells[c]->species_id,cells[c]->tissue_id);
	    if(tissue == NULL){
	      	      printf("This should never happen...");
	      continue;
	    }
	    maxes[c] = tissue->max[n] - cells[c]->nutrients[n] - cells[c]->escrow[n];
	    shares[c] = get_shares_by_direction(tissue->transfer_bias[n],neighbors[c]);
	  }
	if(shares[c] != 0)
	    holes += maxes[c];
	}

	uint available_nutrients = V->nutrients[n] - t->min[n];
	uint allocatable_nutrients = available_nutrients < holes ? available_nutrients : holes;
	allocate_nutrients(maxes,shares,allocatable_nutrients,outs);
	uint allocd = 0;
	for(int c=0;c<3;c++){
	  cells[c]->flux += outs[c];
	  cells[c]->escrow[n] += outs[c];
	  allocd += outs[c];
	}
	V->nutrients[n] -= allocd;
      }
    }}
  return;
}



int tick_reproduction(CA*ca){
  Board*new = ca->new;
  
  for(int i=0;i<ca->ys;i++){for(int j=0;j<ca->xs;j++){
      Cell* V = &new[i][j];
      Species* species = &ca->world->plantae[V->species_id];
      Tissue* t = get_tissue(ca,V->species_id,V->tissue_id);
      if(t == NULL)
	continue;
      
      for(int n=0;n<=WATER;n++){
	V->nutrients[n] += V->escrow[n];
      }
      Tissue* offspring_t = get_tissue(ca,V->species_id,t->offspring_type);
      if(offspring_t == NULL)
	continue;

      if((t->max_offspring == 1 && (V->offspring[0] != SELF || V->offspring[1] != SELF)) || t->max_offspring == 0){
	continue;
      }
      byte* offspring_slot	       = V->offspring[0] == SELF ? &V->offspring[0] : &V->offspring[1];
      if(*offspring_slot != SELF){
	continue;
      }
      
      //    If a unit of nutrient and photosynthate available
      if(V->nutrients[PHOTO] < offspring_t->repro_cost[PHOTO] || V->nutrients[HUMMUS] < offspring_t->repro_cost[HUMMUS]){
	continue;
      }
      
      //If more than one cell neighbouring the occupying cell is empty
      if(neighbor_count(ca,i,j) > species->max_crowding){
	continue;
      }
      
      //choose most resource rich unoccupied cell
      Neighbor candidates[8]	= {0};
      uint win_rng[8]           = {0};
      uint cc			=  0 ;
      uint max	        	=  0 ;
      Neighbor chosen_neighbor  =  0 ;

      Cell* C;
      uint bias = t->reproduction_bias;      
      for(int n=1;n<9;n++){
	uint candidate_y = YMAP[n]+i;
	uint candidate_x = XMAP[n]+j;
	if(!in_bounds(ca,candidate_y,candidate_x))
	  continue;

	C = &new[candidate_y][candidate_x];
	if(C->parent != SELF){
	  continue;
	}
	if(C->substrate_type != V->substrate_type){
	  //we're not interested in creeper/rhizome behavior at this time (too ugly)
	  continue;
	}

	if(REPRO_BIAS(bias,n)){
	  if(C->resource == max){
	    candidates[cc++] = n;
	  }else if(C->resource > max){
	    cc = 0;
	    candidates[cc++] = n;
	    max = C->resource;
	  }
	}

      }

      if(cc == 0){
	//no candidates for reproduction.
	continue;
      }      


      uint rng = 0;
      uint o_bias = REPRO_BIAS(bias,candidates[0])-1;
      win_rng[0] = o_bias;
      rng = o_bias;
      for(int k=1;k<cc;k++){
	win_rng[k] = rng+REPRO_BIAS(bias,candidates[k]);
	rng += REPRO_BIAS(bias,candidates[k]);
      }
      win_rng[cc] = 1000;

      
      uint result = rng == 0 ?rng:rand()%rng;
      uint winner = 0;
      for(int k=0;k<cc;k++){
	if(result <= win_rng[k]){
	  winner = k;
	  break;
	}
      }
      //todo: add reproduction_bias code

      chosen_neighbor = candidates[winner];  
      //create new plant module
      Cell* chosen_cell = neighbor(ca,chosen_neighbor,i,j);
      if(chosen_cell->floaty){
	//temporary floaty fix.
	continue;
      }
      //set the offspring node to newly created module
      *offspring_slot = chosen_neighbor;
      if(chosen_cell->parent != SELF || chosen_cell->offspring[0] != SELF || chosen_cell->offspring[1] != SELF || V->parent == SELF || V->offspring[0] == SELF){
	printf("THSIO NEVER HAPPEN!IN");
      }

      //set parent node of newly created module to current module
      chosen_cell->parent = RMAP[chosen_neighbor];
      chosen_cell->offspring[0] = NEW;
      chosen_cell->offspring[1] = NEW;
      chosen_cell->tissue_id = t->offspring_type;
      chosen_cell->species_id = V->species_id;
      chosen_cell->flux = offspring_t->initial_flux;
      chosen_cell->plant_id = V->plant_id;
      //      ca->population[chosen_cell->species_id][chosen_cell->plant_id][chosen_cell->tissue_id]++;
      //reduce nutrient and photosynthate.
      V->nutrients[PHOTO] -= t->repro_cost[PHOTO];
      V->nutrients[HUMMUS]     -= t->repro_cost[HUMMUS];
    }}
  return 0;
}
