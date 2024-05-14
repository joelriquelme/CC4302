#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

typedef struct {
  int ready;
  pthread_cond_t w;
} Request;


pthread_mutex_t n = PTHREAD_MUTEX_INITIALIZER;
int busy = 0;
int index = 0;
PriQueue *q;
PriQueue *p;
PriQueue *trash;

//Función para solicitar acceso exclusivo al disco.
//Debe esperar si el disco está ocupado.La identificación numérica de
//pista, track, es linealmente proporcional a la distancia de la pista al
//centro del disco. La pista 0 es la más cercana al centro. El thread
//accederá a la pista track después del retorno de requestDisk.
//invocará releaseDisk notificando el término del uso del disco.
//Si en ese momento hay varias
//solicitudes de acceso en espera, y se acaba de acceder a la pista t, entre
//todos los requestDisk(t') pendientes Ud. debe autorizar el acceso que
//requiera el cabezal en la pista t' más cercana a t sujeto a que t' ≥ t.
//Autorice haciendo que ese requestDisk(t') retorne. Si no hay ninguna
//solicitud con esas características, autorice la solicitud que lleve el
//cabezal a la pista más cercana al centro del disco. Por ejemplo si el
//cabezal está en la pista 4 y hay solicitudes en espera para las pistas 2, 2,
//3, 4, 6 y 10, el orden de autorización debe ser 4, 6, 10, 2, 2 y 3.
void requestDisk(int track) {
  pthread_mutex_lock(&n);
  if (!busy) {
    busy = 1; 
  } else {
    Request req = { 0, 
    PTHREAD_COND_INITIALIZER };
    if (track < index) {
      priPut(p, &req, track);
    } else {
      priPut(q, &req, track);
    }
    while (!req.ready){
      pthread_cond_wait(&req.w, &n);
    }
  pthread_cond_destroy(&req.w);
  }
  index = track;
  pthread_mutex_unlock(&n);
  return;
}

void releaseDisk(void) {
  pthread_mutex_lock(&n);
  if (emptyPriQueue(q) && emptyPriQueue(p)) {
    busy = 0;
  }
  else if (emptyPriQueue(q)) {
    trash = q;
    q = p;
    p = makePriQueue();
    destroyPriQueue(trash);
    Request *req = priGet(q);
    req->ready = 1;
    pthread_cond_signal(&req->w);
  }
  else {
    Request *req = priGet(q);
    req->ready = 1;
    pthread_cond_signal(&req->w);
  }
  pthread_mutex_unlock(&n);
}

void iniDisk(void) {
  q = makePriQueue();
  p = makePriQueue();
}

void cleanDisk(void) {
  destroyPriQueue(q);
  destroyPriQueue(p);
}
