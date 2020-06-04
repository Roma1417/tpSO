/*
 * posicion.h
 *
 *  Created on: 9 may. 2020
 *      Author: utnso
 */

#ifndef POSICION_H_
#define POSICION_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t u_int32_t;

typedef struct {

	u_int32_t x;
	u_int32_t y;

} t_posicion;

t_posicion* posicion_create(u_int32_t x, u_int32_t y);
void mover_a_la_derecha(t_posicion* posicion);
void mover_a_la_izquierda(t_posicion* posicion);
void mover_hacia_arriba(t_posicion* posicion);
void mover_hacia_abajo(t_posicion* posicion);
bool esta_mas_a_la_derecha(t_posicion* una_posicion, t_posicion* otra_posicion);
bool esta_mas_arriba(t_posicion* una_posicion, t_posicion* otra_posicion);
u_int32_t distancia_en_x(t_posicion* una_posicion, t_posicion* otra_posicion);
u_int32_t distancia_en_y(t_posicion* una_posicion, t_posicion* otra_posicion);

#endif /* POSICION_H_ */
