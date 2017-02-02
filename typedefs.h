#ifndef TYPEDEFS_H
#define TYPEDEFS_H
#include <sys/time.h>
#define TC 9 //number of tissues
#define NC 3
#define THC 2
typedef enum EVOLVE_THRESHOLD_TYPE{
  LONGEST_DISTANCE_TO_TIP,
  LONGEST_DISTANCE_TO_ROOT,
}EVOLVE_THRESHOLD_TYPE;
typedef unsigned int uint;
typedef const short unsigned int cuint;
typedef short unsigned int Cprop;
typedef unsigned char byte;
typedef byte Pprop;
typedef short unsigned int SpeciesId;
typedef  struct Cell* Board;

typedef struct World{
  uint allocd_genomes;
  struct Genome* genomes;
  struct Species* plantae;
  struct Animalia* animalia;
  struct CA* zones;
  uint zone_count;
  struct timeval last_tick_duration;
  struct timeval last_tick_time;
  int tick_speed_msec;
  struct Experiment* experiment;
  uint max_plant_id;
  uint max_species_id;
  uint allocd_zones;
  uint generation;
  uint item_space;
}World;
//if we were programming in C++, we'd make World a class, wouldn't we?
/*
  
  
*/
typedef struct CA {
  uint ys;
  uint xs;
  uint rain_level;
  struct Cell** new;
  struct Cell** old;
  struct World* world;
  uint tick;
  uint total_water;
  uint ground_water;
} CA;

typedef struct Genome{
  short unsigned transfer_bias[NC][TC];
  short unsigned reproduction_bias[TC];
  byte min[NC][TC];
  byte max[NC][TC];
  byte evolve_type[THC][TC];
  byte evolve_threshold[THC][TC];
  byte max_offspring[TC];
  float flux_cost[TC];
  float initial_flux[TC];
  unsigned int  parents[2];
  uint genome_id;
  char min_seed_nutrients[NC];
  char name[32]; // optional
  char remarks[128];
}Genome;

typedef enum TissueId{
  NONE,
  LEAF,
  SHOOT,
  BRANCH,
  TRUNK,
  FLOWER_MALE,
  FLOWER_FEMALE,
  ROOT,
}TissueId;

typedef enum EvolveThresholdType{
  DIST_TO_ROOT,
  DIST_TO_LEAF,
}EvolveThresholdType;
typedef struct Tissue{
  byte                max[3];   //maximum Currency tissue can hold.
  byte                repro_cost[3];
  TissueId            offspring_type;
  
  short unsigned int  transfer_bias[3];
  /*
    each short is an array of 8 2 bit quantities, each expressing the 'extra shares' of the appropriate currency
    delivered to cooresponding neighbor.
   */
  short unsigned int reproduction_bias;
  byte                maint_cost[2];  //probabilistic chance of consuming a currency, in chances/255
  TissueId            evolve_type[2];
  byte                evolve_threshold[2];
  
  byte                max_offspring; // must be 0, 1, or 2.
  byte                min[3];   //minimum of Currency to hoard
  float flux_cost;
  float initial_flux;
  byte fg_color;
  byte bg_color;
  byte visible;
  byte photosynthetic;
  unsigned int decay_rate;
  short unsigned int max_damage;
} Tissue;

typedef struct Experiment{
  byte repro_cost[NC][TC];
  byte max_damage[TC];
  int fg_color[TC];
  int bg_color[TC];
  byte visible[TC];
  byte offspring_type[TC];
  byte photosynthetic[TC];
  unsigned int decay_rate[TC]; // chance/65535 of tissue decaying after death.
}Experiment;
#endif
