/*
 * posicion.c
 *
 *  Created on: 13 may. 2020
 *      Author: utnso
 */

#include "posicion.h"

/*
 * @NAME: posicion_create
 * @DESC: Dados dos valores enteros, crea y devuelve un puntero
 *        a una estructura t_posicion.
 */
t_posicion* posicion_create(u_int32_t x, u_int32_t y){
	t_posicion* posicion = malloc(sizeof(t_posicion));
	posicion->x = x;
	posicion->y = y;
	return posicion;
}

/*
 * @NAME: mover_a_la_derecha
 * @DESC: Dada una posicion, aumenta su posicion en x en 1.
 */
void mover_a_la_derecha(t_posicion* posicion){
	posicion->x++;
}

/*
 * @NAME: mover_a_la_izquierda
 * @DESC: Dada una posicion, decrementa su posicion en x en 1.
 */
void mover_a_la_izquierda(t_posicion* posicion){
	posicion->x--;
}

/*
 * @NAME: mover_hacia_arriba
 * @DESC: Dada una posicion, aumenta su posicion en y en 1.
 */
void mover_hacia_arriba(t_posicion* posicion){
	posicion->y++;
}

/*
 * @NAME: mover_hacia_la_derecha
 * @DESC: Dada una posicion, decrementa su posicion en y en 1.
 */
void mover_hacia_abajo(t_posicion* posicion){
	posicion->y--;
}

/*
 * @NAME: esta_mas_a_la_derecha
 * @DESC: Dadas dos posiciones, nos dice si la primera tiene una mayor
 * 		  posicion en x que la segunda.
 */
bool esta_mas_a_la_derecha(t_posicion* una_posicion, t_posicion* otra_posicion){
	return una_posicion->x > otra_posicion->x;
}

/*
 * @NAME: esta_mas_arriba
 * @DESC: Dadas dos posiciones, nos dice si la primera tiene una mayor
 * 		  posicion en y que la segunda.
 */
bool esta_mas_arriba(t_posicion* una_posicion, t_posicion* otra_posicion){
	return una_posicion->y > otra_posicion->y;
}

/*
 * @NAME: distancia_en_x
 * @DESC: Devuelve el valor absoluto de la diferencia en x
 */
u_int32_t distancia_en_x(t_posicion* una_posicion, t_posicion* otra_posicion){
	return abs(una_posicion->x - otra_posicion->x);
}

/*
 * @NAME: distancia_en_y
 * @DESC: Devuelve el valor absoluto de la diferencia en y
 */
u_int32_t distancia_en_y(t_posicion* una_posicion, t_posicion* otra_posicion){
	return abs(una_posicion->y - otra_posicion->y);
}

void mover_de_posicion(t_posicion* posicion1, t_posicion* posicion2, t_config_team* config_team){
	u_int32_t distancia_x = distancia_en_x(posicion1, posicion2);
	for(int i=0; i<distancia_x;i++){
		if(esta_mas_a_la_derecha(posicion2, posicion1))
			mover_a_la_derecha(posicion1);
		else mover_a_la_izquierda(posicion1);
		sleep(config_team->retardo_ciclo_cpu);
	}
	u_int32_t distancia_y = distancia_en_y(posicion1, posicion2);
	for(int j=0; j<distancia_y;j++){
		if(esta_mas_arriba(posicion2, posicion1))
			mover_hacia_arriba(posicion1);
		else mover_hacia_abajo(posicion1);
		sleep(config_team->retardo_ciclo_cpu);
	}
}
