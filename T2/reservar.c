#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

// Defina aca las variables globales y funciones auxiliares que necesite
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int ticket_dist = 0, display = 0;
int parking[10];

void initReservar() {
  for (int i = 0; i < 10; i++) {
  parking[i] = 0;
  }
  ticket_dist = 0;
  display = 0;
}

void cleanReservar() {

}

int canPark(int k) {
  for (int i = 0; i < 10; i++) {
    if (parking[i] == 0) {
      int j = i;
      while (j < 10 && parking[j] == 0) {
        j++;
      }
      if (j - i >= k) {
        return i;
      }
    }
  }
  return -1;
}

int reservar(int k) {
  pthread_mutex_lock(&m);
  int my_num = ticket_dist++;
  //printf("Ticket: %d\n", my_num);
  //printf("Display: %d\n", display);
  //se espera a que sea el turno del vehiculo y si es que hay un lugar disponible
  while (my_num != display) {
    pthread_cond_wait(&c, &m);
  }
  while (canPark(k) == -1) {
    pthread_cond_wait(&c, &m);
  }
  int place = canPark(k);
  for (int i = place; i < place + k; i++) {
    parking[i] = 1;
  }
  pthread_mutex_unlock(&m);
  return place;
}

void liberar(int e, int k) {
  pthread_mutex_lock(&m);
  display++;
  for (int i = e; i < e + k; i++) {
    parking[i] = 0;
  }
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
} 
