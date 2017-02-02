#ifndef SERIALIZE_H
#define SERIALIZE_H
#include <stddef.h>
#include "shrub.h"
#define EXP_PROPCOUNT_V01 15
#define CELL_PROPCOUNT_V01 7

#define PROP(property,obj) {#property,offsetof(obj,property)}
typedef struct prop_pair {
  char*s;
  uint o;
}prop_pair;


int save_genomes(World*world,char*file);
int load_genomes(World* world,char*file);
#endif
