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

int q = 0;




int vendo(int precio, char *vendedor, char *comprador) {
  spinLock(&mutex);
  if (precio < precio_gl) {
    if (q) {
      spinUnlock(pcond);
    }
    precio_gl = precio;
    int cond = CLOSED;
    int estado = 0;
    pcond = &cond;
    pestado = &estado; 
    comprador_gl = comprador;
    vendedor_gl = vendedor; 
    q = 1;
    spinUnlock(&mutex);
    spinLock(&cond);
    spinLock(&mutex);
    if (*pestado == 1) {
      spinUnlock(&mutex);
      return 1;
    }
  }
  spinUnlock(&mutex);
  return 0;
}

int compro(char *comprador, char *vendedor) {
  spinLock(&mutex);
  if (!q) {
    spinUnlock(&mutex);
    return 0;
  }
  else {
    int estado = 1;
    pestado = &estado;
    int precio = precio_gl;
    precio_gl = INT_MAX;
    q = 0;
    strcpy(comprador_gl, comprador);
    strcpy(vendedor, vendedor_gl);
    spinUnlock(pcond);
    spinUnlock(&mutex);
    return precio;
  }
}
