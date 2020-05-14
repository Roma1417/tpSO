/*
 * posicion.c
 *
 *  Created on: 13 may. 2020
 *      Author: utnso
 */

#include "posicion.h"

t_posicion* posicion_create(u_int32_t x, u_int32_t y){
	t_posicion* posicion = malloc(sizeof(t_posicion));
	posicion->x = x;
	posicion->y = y;
	return posicion;
}

void mover_a_la_derecha(t_posicion* posicion){
	posicion->x++;
}

void mover_a_la_izquierda(t_posicion* posicion){
	posicion->x--;
}

void mover_hacia_arriba(t_posicion* posicion){
	posicion->y++;
}

void mover_hacia_abajo(t_posicion* posicion){
	posicion->y--;
}

bool esta_mas_a_la_derecha(t_posicion* una_posicion, t_posicion* otra_posicion){
	return una_posicion->x > otra_posicion->x;
}

bool esta_mas_arriba(t_posicion* una_posicion, t_posicion* otra_posicion){
	return una_posicion->y > otra_posicion->y;
}
