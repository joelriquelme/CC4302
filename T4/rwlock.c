#define _XOPEN_SOURCE 500

#include "nthread-impl.h"

#include "rwlock.h"

struct rwlock {
  NthQueue* writerQueue;
  NthQueue* readerQueue;
  int writerCount;
  int readerCount;
};

nRWLock *nMakeRWLock() {
  nRWLock *rwl= (nRWLock *)nMalloc(sizeof(nRWLock)); 
  rwl->writerQueue= nth_makeQueue();
  rwl->readerQueue= nth_makeQueue();
  rwl->writerCount= 0;
  rwl->readerCount= 0; 
  return rwl;
}

void nDestroyRWLock(nRWLock *rwl) {
  nth_destroyQueue(rwl->writerQueue);
  nth_destroyQueue(rwl->readerQueue);
  nFree(rwl);
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  //Una solicitud de ingreso de un escritor (función nEnterWrite) se
  //acepta de inmediato si no hay lectores o un escritor trabajando.
  //De otro modo la solicitud queda pendiente
  START_CRITICAL
  if (rwl->readerCount == 0 && rwl->writerCount == 0) {
    rwl->writerCount = 1;
  }
  else {
    nThread thisTh = nSelf();
    nth_putBack(rwl->writerQueue, thisTh);
    suspend(WAIT_RWLOCK);
    schedule();
  }
  END_CRITICAL
  return 1;
}

int nEnterRead(nRWLock *rwl, int timeout) {
  //Una solicitud de ingreso de un lector (función nEnterRead) se
  //acepta de inmediato si no hay un escritor trabajando y no hay
  //solicitudes de escritores pendientes. De lo contrario la solicitud
  //queda pendiente.
  START_CRITICAL
  if (rwl->writerCount == 0 && nth_emptyQueue(rwl->writerQueue)) {
    rwl->readerCount++;
  }
  else {
    nThread thisTh= nSelf();
    nth_putBack(rwl->readerQueue, thisTh);
    suspend(WAIT_RWLOCK);
    schedule();
  }
  END_CRITICAL
  return 1;
}

void nExitWrite(nRWLock *rwl) {
  //Cuando termina de trabajar un escritor (función nExitWrite) y
  //hay solicitudes de lectores pendientes, se acepta el ingreso de
  //todos los lectores pendientes. Si no hay lectores pendientes,
  //pero sí hay solicitudes de escritores pendientes, se acepta el
  //ingreso del escritor que lleva más tiempo esperando.
  START_CRITICAL
  rwl->writerCount = 0;

  if (!nth_emptyQueue(rwl->readerQueue)) {
    while (!nth_emptyQueue(rwl->readerQueue)) {
      nThread w = nth_getFront(rwl->readerQueue);
      setReady(w);
      rwl->readerCount++;
    }
    schedule();
  }
  else if (!nth_emptyQueue(rwl->writerQueue)) {
    nThread w = nth_getFront(rwl->writerQueue);
    setReady(w);
    rwl->writerCount = 1;
    schedule();
  }
  END_CRITICAL 
}

void nExitRead(nRWLock *rwl) {
  //Cuando termina de trabajar un lector (función nExitRead) y ya
  //no quedan otros lectores trabajando, si hay solicitudes de
  //escritores pendientes, se acepta el ingreso del escritor que lleva
  //más tiempo esperando.
  START_CRITICAL
  rwl->readerCount--;

  if (rwl->readerCount == 0 && !nth_emptyQueue(rwl->writerQueue)) {
    nThread w = nth_getFront(rwl->writerQueue);
    setReady(w);
    rwl->writerCount = 1;
    schedule();
  }
  END_CRITICAL 
}


