#include "shrub.h"
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "serialize.h"
static short unsigned int magic_number = 0x1EAF;
static short unsigned int library_magic_number = 0xBEAF;

int load_game(World* world,char* file){
	return load_genomes(world,file);
}

int save_game(World* world,char* file){
	return save_genomes(world,file);
}


int save_default_game(World* world){
	char* home = getenv("HOME");
	char * save_file_path = malloc(FILENAME_MAX);
	snprintf(save_file_path,FILENAME_MAX,"%s/.shrub",home);
	save_game(world,save_file_path);
	free(save_file_path);
	return 0;
}
int load_default_game(World* world){
	char* home = getenv("HOME");
	char * load_file_path = malloc(FILENAME_MAX);
	snprintf(load_file_path,FILENAME_MAX,"%s/.shrub",home);
	load_game(world,load_file_path);
	free(load_file_path);
	return 0;
}

int save_named_genomes(World*world,char*file){
  int f = open(file,O_WRONLY|O_CREAT|O_TRUNC,0600);
  short unsigned int mnumber = library_magic_number;
  if(write(f,(void*)&mnumber,sizeof(short unsigned int)) != sizeof(short unsigned int))
    {
      return errno;
    }
  for(int i=1;i<world->max_species_id;i++){
    if(world->genomes[i].name[0] != '\0'){
      if(write(f,(void*)&world->genomes[i],sizeof(Genome)) == -1){
	return errno;
      }
    }
  }
  close(f);
  return errno;
}
int save_genomes(World*world,char*file){
  int f = open(file,O_WRONLY|O_CREAT|O_TRUNC,0600);
  short unsigned int mnumber = magic_number;
  if(write(f,(void*)&mnumber,sizeof(short unsigned int)) != sizeof(short unsigned int))
    {
      return errno;
    }
  int result = write(f,(void*)world->experiment,sizeof(Experiment));
  if(result == -1)
    return errno;
  for(int i=1;i<world->max_species_id;i++){
    if(write(f,(void*)&world->genomes[i],sizeof(Genome)) == -1){
      return errno;
    }
  }
  
  close(f);
  return 0;
  
}


int load_genomes(World* world,char*file){
  int f = open(file,O_RDONLY,0600);
  short unsigned int mnumber;
  int result;
  if(read(f,(void*)&mnumber,sizeof(short unsigned int)) != sizeof(short unsigned int)){
    return errno;
  }
  if(mnumber != magic_number){
    return -1;
  }   
  if((result =read(f,(void*)world->experiment,sizeof(Experiment)))){
    if(result == -1)
      return errno;
  }  
  Genome g;
  while(read(f,(void*)&g,sizeof(Genome))){
    create_species(world,&g);
  }
  close(f);
  return 0;
  
}
  

  


#ifdef SERIALIZE_IS_MAIN
int main(){
  for(int i=0;i<500;i++){    
      tick_CA(new_ex);
  }
  save_v01(".bonsai",new_ex);  
  for(int i=0;i<500;i++){    
    tick_CA(new_ex);
  }  
}
#endif
