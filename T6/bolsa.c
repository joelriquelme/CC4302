#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "pss.h"
#include "bolsa.h"
#include "spinlocks.h"

int mutex = OPEN;
int precio_gl = INT_MAX;
char *vendedor_gl = NULL;
char *comprador_gl = NULL;

int *pcond = NULL;
int *pestado = NULL;

int ofertado = 0;




int vendo(int precio, char *vendedor, char *comprador) {
  spinLock(&mutex);
  if (precio < precio_gl) {
    if (ofertado) {
      spinUnlock(pcond);
    }
    precio_gl = precio;
    int cond = CLOSED;
    int estado = 0;
    pcond = &cond;
    pestado = &estado; 
    comprador_gl = comprador;
    vendedor_gl = vendedor; 
    ofertado = 1;
    spinUnlock(&mutex);
    spinLock(&cond);
    spinLock(&mutex);
    if (estado == 1) {
      spinUnlock(&mutex);
      return 1;
    }
  }
  spinUnlock(&mutex);
  return 0;
}

int compro(char *comprador, char *vendedor) {
  spinLock(&mutex);
  if (!ofertado) {
    spinUnlock(&mutex);
    return 0;
  }
  else {
    *pestado = 1;
    int precio = precio_gl;
    precio_gl = INT_MAX;
    ofertado = 0;
    strcpy(comprador_gl, comprador);
    strcpy(vendedor, vendedor_gl);
    spinUnlock(pcond);
    spinUnlock(&mutex);
    return precio;
  }
}
