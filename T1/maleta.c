// Plantilla para maleta.c

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>

#include "maleta.h"
#define P 8

// Defina aca las estructuras y funciones adicionales que necesite

typedef struct {
  int n;
  double maxW;
  int k;
  double res;
  double *w;
  double *v;
  int *z; 
} Args;

void *thread(void *p) {
  Args *args = (Args *)p;
  int n = args->n;
  double *w, *v;
  int *z;
  w = args->w;
  v = args->v;
  z = args->z; 
  double maxW = args->maxW;
  int k = args->k;

  args->res = llenarMaletaSec(w, v, z, n, maxW, k);
  return NULL;
}

double llenarMaletaPar(double w[], double v[], int z[], int n,
                       double maxW, int k) {
  pthread_t pid[P];
  Args args[P];

  for (int i = 0; i < 8; i++) {
    int *newz;
    newz = malloc(n*sizeof(int));
    args[i].w = w;
    args[i].v = v;
    args[i].z = newz;
    args[i].n = n;
    args[i].maxW = maxW;
    args[i].k = k/P;

    pthread_create(&pid[i], NULL, thread, &args[i]);
  }
  
  double best = 0;
  for (int i = 0; i < 8; i++) {
    pthread_join(pid[i], NULL);
    if (args[i].res > best) {
      best = args[i].res;
      for (int j = 0; j < n; j++) {
        z[j] = args[i].z[j];
      }
    }
    free(args[i].z);
  }
  return best;
}
