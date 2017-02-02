#ifndef GENOME_H
#define GENOME_H
#include "shrub.h"
#include "typedefs.h"
#define ALL(_ex,_prop,val) for(byte _i=0;_i<TC;_i++){_ex->_prop[_i] = val;}
#define ALE(_ex,_prop,val) for(byte _i=0;_i<TC;_i++){for(byte _j=0;_j<NC;_j++){_ex->_prop[_j][_i] = val;}}
#define ONE(_ex,_prop,val) _ex->_prop = val


typedef enum Nutrient{
  PHOTO,
  HUMMUS,
  WATER,
}Nutrient;

  
typedef struct Animalia{
  SpeciesId species_id;
}Animalia;


typedef struct Statistics{
  uint population;
  uint pop_by_tissue[TC];
} Statistics;

typedef struct Species{
  //a species is a group of organisms in a CA that share a genome.
  unsigned int  species_id;
  unsigned int  genome_id;
  unsigned int  count;
  unsigned int  allocd;
  unsigned int* plant_ids;
  Tissue* tissues;
  Pprop max_crowding;
  Statistics stats;
}Species;

typedef Species* Plantae;
void init_plantae(Plantae* plantae);
int auto_breed_plants(World* world,uint zone,uint diversity);

int init_sample_run(Experiment* e1,Genome* g1);
void apply_genome(Experiment* e1,Genome* g,Species* sp);
void create_random_species(World*world,Genome*template,uint count);
void auto_seed_CA(CA*ca,uint dist,uint min, uint max);
int  total_biomass(CA*ca);
void combine_genomes(Genome*g1_in,Genome*g2_in,Genome*g1_out,Genome*g2_out);
#endif
