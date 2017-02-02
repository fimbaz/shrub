#include "typedefs.h"
#include "roulette.h"
#include <stdlib.h>
int roulette (uint* slices,uint*winners,uint spins,uint count){
  int* wheel = calloc(count,sizeof(int));
  wheel[0] = slices[0] == 0 ? slices[0] : slices[0]-1;
  int sum =slices[0];
  for(int i=1;i<count;i++){
    wheel[i] = wheel[i-1] + slices[i];
    sum += slices[i];
  }
  for(int s=0;s<spins;s++){
    int spin = rand()%sum;
    int i;
    for(i=0;i<count;i++){
      if(wheel[i] >= spin)
	break;
    }
    winners[s] = i;
  }
  free(wheel);
  return 0;
}



  
