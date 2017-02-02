#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <roulette.h>
#include "genome.h"

#include "perlin.h"
short unsigned int crossover(short unsigned int m,short unsigned int f,short unsigned int* c1,short unsigned int*c2){
  uint entropy = rand();
  uint msk= ((1 << (entropy & 0x0F)) - 1);
  uint mt1 = msk&m;
  uint mf1 = (~msk)&f;
  uint mt2 = msk&f;
  uint mf2 = (~msk)&m;
  uint out1 = mt1 | mf1;
  for(int i=0;i<16;i++){
    if(!rand()%1024)
      out1 =  out1 ^ (1<<i);
  }
  uint out2 = mt2 | mf2;
  for(int i=0;i<16;i++){
    if(!rand()%1024)
      out1 =  out1 ^ (1<<i);
  }
  *c1 = out1;
  *c2 = out2;
  return 0;
}


void create_random_species(World*world,Genome*template,uint count){
  Genome g;
  Genome*gp = &g;
  for(int i=0;i<count;i++){
    memcpy(gp,template,sizeof(Genome));
    ALL(gp,reproduction_bias,rand());
    ALE(gp,transfer_bias,rand());
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_TIP][LEAF]  ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_TIP][SHOOT] ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][SHOOT] ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_TIP][LEAF]  ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_TIP][SHOOT] ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][SHOOT] ,rand()%16);
    ONE(gp,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][LEAF],rand()%16);
    ONE(gp,evolve_threshold[0][FLOWER_MALE],100);
    ONE(gp,evolve_threshold[1][FLOWER_FEMALE],100);
    gp->parents[0] = 0;
    gp->parents[1] = 0;
    create_species(world,gp);
  }
  return;
}

void auto_seed_CA(CA*ca,uint dist,uint min, uint max){
  uint s = 0;
  for(int j=0;j<ca->xs;j+=50){
    uint ground = find_ground(ca,j);
    plant_seed(ca,s+min,ground,j);
    s = (s+1)%(max-min);
  }
  return;
}


  
void combine_genomes(Genome*g1_in,Genome*g2_in,Genome*g1_out,Genome*g2_out){
  memcpy(g1_out,g1_in,sizeof(Genome));
  memcpy(g2_out,g2_in,sizeof(Genome));

  for(int form=0;form<3;form++){
    for(int tissue=0;tissue<TC;tissue++){
      unsigned short n1,n2;
      crossover(g1_in->evolve_threshold[form][tissue],g2_in->evolve_threshold[form][tissue],&n1,&n2);
      n1 &= 0x08;
      n2 &= 0x08;
      g1_out->evolve_threshold[form][tissue] = n1;
      g2_out->evolve_threshold[form][tissue] = n2;
    }
  }
    ONE(g1_out,evolve_threshold[0][FLOWER_MALE],100);
    ONE(g1_out,evolve_threshold[1][FLOWER_FEMALE],100);
    
  for(int t=0;t<TC;t++){
    for(int n=0;n<WATER;n++){
      crossover(g1_in->transfer_bias[n][t],g2_in->transfer_bias[n][t],&g2_out->transfer_bias[n][t],&g2_out->transfer_bias[n][t]);
    }
    crossover(g1_in->reproduction_bias[t],g2_in->reproduction_bias[t],&g2_out->reproduction_bias[t],&g2_out->reproduction_bias[t]);
  }
  return;
}
void apply_genome(Experiment* e1,Genome* g,Species* p){
  //  memset((void*)p,0,sizeof(Species));
  p->max_crowding = 4; //fix this.
  for(int t=0;t<TC;t++){
    Genome* g1 = g;
    Tissue* T = &p->tissues[t];    
    memset((void*)T,0,sizeof(Tissue));
    for(int n=0;n<NC;n++){
      T->repro_cost[n]		= e1->repro_cost[n][t];
      T->transfer_bias[n]	= g1->transfer_bias[n][t];
      T->max[n]			= g1->max[n][t];
      T->min[n]			= g1->min[n][t];
    }
    T->max_damage		= e1->max_damage[t];
    T->fg_color		 = e1->fg_color[t];
    T->bg_color		 = e1->bg_color[t];
    T->visible           = e1->visible[t];
    T->reproduction_bias = g1->reproduction_bias[t];
    T->evolve_type[0]       = g1->evolve_type[0][t];
    T->evolve_type[1]       = g1->evolve_type[1][t];
    T->evolve_threshold[0]  = g1->evolve_threshold[0][t];
    T->evolve_threshold[1]  = g1->evolve_threshold[1][t];
    T->max_offspring     = g1->max_offspring[t];
    T->flux_cost         = g1->flux_cost[t];
    T->offspring_type    = e1->offspring_type[t];
    T->initial_flux      = g1->initial_flux[t];
    T->photosynthetic    = e1->photosynthetic[t];
    T->decay_rate        = e1->decay_rate[t];
  }
  return;
}

int init_sample_run(Experiment* e1,Genome* gs){
  ALE(e1,repro_cost,1);
  ALL(e1,repro_cost[WATER],0);
  ALL(e1,fg_color,0);
  ALL(e1,bg_color,0);
  ONE(e1,fg_color[LEAF]  ,119);
  ONE(e1,fg_color[SHOOT] ,2);
  ONE(e1,fg_color[BRANCH],208);
  ALL(e1,offspring_type,LEAF);
  ONE(e1,offspring_type[SHOOT],LEAF);
  ONE(e1,offspring_type[ROOT],ROOT);
  ONE(e1,offspring_type[LEAF],LEAF);
  ONE(e1,offspring_type[BRANCH],LEAF);
  ONE(e1,photosynthetic[LEAF],1);
  ONE(e1,photosynthetic[SHOOT],1);


  ALL(e1,max_damage,100);  
  ALL(e1,visible,1);
  ONE(e1,visible[ROOT],0);

    ALE(gs,transfer_bias,0xFFFF);
    ALL(gs,max_offspring,2);
  
    ALE(gs,max,10);
    ALE(gs,min,0);

    ALL(gs,evolve_type[0],BRANCH);
    ALL(gs,evolve_type[1],BRANCH);
    ALL(gs,evolve_threshold[1],100);
    ALL(gs,evolve_threshold[0],100);
    ONE(gs,evolve_type[0][LEAF]                     ,SHOOT);
    ONE(gs,evolve_type[LONGEST_DISTANCE_TO_TIP][SHOOT],BRANCH);
    ONE(gs,evolve_type[LONGEST_DISTANCE_TO_ROOT][LEAF],FLOWER_MALE);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_TIP][LEAF]  ,2);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_TIP][SHOOT] ,4);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][SHOOT] ,5);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_TIP][LEAF]  ,2);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_TIP][SHOOT] ,4);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][SHOOT] ,5);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][LEAF],5);
    ONE(gs,evolve_threshold[0][FLOWER_MALE],100);
    ONE(gs,evolve_threshold[1][FLOWER_FEMALE],100);
    ONE(gs,evolve_threshold[LONGEST_DISTANCE_TO_ROOT][BRANCH],3);
    ONE(gs,evolve_type[LONGEST_DISTANCE_TO_ROOT][BRANCH],FLOWER_FEMALE);
    ONE(e1,fg_color[FLOWER_MALE],161);
    ONE(e1,fg_color[FLOWER_FEMALE],128);
    ALL(gs,reproduction_bias,0xFFFF);
    ALL(gs,flux_cost,.1);
    ALL(gs,initial_flux,100);
    ONE(gs,flux_cost[LEAF],1);
    ALL(e1,decay_rate,100);
    //ONE(gs,flux_cost[SHOOT],1);
    memset(gs->name,0,sizeof(gs->name));
    strcpy(gs->name," ");
    strcpy(gs->remarks," ");
    
    return 0;
}

int compar_species_by_pop(const void* v1,const void* v2){
  Species*sp1 = *(Species**)v1;
  Species*sp2 = *(Species**)v2;
  
  if(sp1->stats.population < sp2->stats.population)
    return 1;
  else if(sp1->stats.population > sp2->stats.population)
    return -1;
  return 0;
}

#ifdef GENOME_MAIN
int main(int argc,char* argv[]){
  uint y,x,turns,floaties;
  if(argc == 5){
    sscanf(argv[1],"%d",&y);
    sscanf(argv[2],"%d",&x);
    sscanf(argv[3],"%d",&turns);
    sscanf(argv[4],"%d",&floaties);
  }else if(argc > 1){
    printf("Usage: ./sperf y x turns floaties\n");
    exit(1);
  }else{
    y = 100;
    x = 2000;
    turns = 0;
  }
 
  srand(time(NULL));
  Experiment* e1 = calloc(sizeof(Experiment),1);
  Genome* g1     = calloc(sizeof(Genome),1);
  init_sample_run(e1,g1);
  World* world = new_perlin_zone(y,x,.005,250,50,5,80);
  CA* ca = &world->zones[0];

  uint diversity = 10;
  for(int i=0;i<diversity;i++){
    ALL(g1,reproduction_bias,rand());
    ALE(g1,transfer_bias,rand());
    create_species(ca,e1,g1);
  }

  for(int j=0;j<ca->xs/50-1;j+=1){
    uint ground = find_ground(ca,j*50);
    plant_seed(ca,(j%diversity)+1,ground,j*50);    
  }

  Species** sp = calloc(diversity,sizeof(Species));
  for(int i=0;i<diversity;i++){
    sp[i] = &world->plantae[i];
  }
      
}
#endif
int total_biomass(CA*ca){
  int biomass = 0;
  
  for(int i=0;i<ca->ys;i++)
    for(int j=0;j<ca->xs;j++)
      if(ca->new[i][j].parent != SELF)
	biomass++;
  return biomass;
  
}

      

int auto_breed_plants(World*world,uint zone,uint diversity){
  uint* popcount = sum_by_species(world,zone); 
  uint* fitness = popcount;
  uint* winner_ids = calloc(diversity,sizeof(uint));
  roulette(fitness+(world->max_species_id-diversity),winner_ids,diversity,diversity);
  Genome g1;
  Genome g2;
  for(int i=0;i<diversity;i+=2){
    Genome* m = &world->genomes[winner_ids[i]];
    Genome* p = &world->genomes[winner_ids[i+1]];
    combine_genomes(m,p,&g1,&g2);
    g1.parents[0] = winner_ids[i];
    g1.parents[1] = winner_ids[i+1];
    g2.parents[0] = winner_ids[i+1];
    g2.parents[1] = winner_ids[i];    
    create_species(world,&g1);
    create_species(world,&g2);
  }
  free(popcount);
  free(winner_ids);
  return 0;    
}
