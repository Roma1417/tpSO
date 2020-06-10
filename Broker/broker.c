/*
 * broker.c
 *
 *  Created on: 17 abr. 2020
 *      Author: utnso
 */

#include<stdio.h>
#include "broker.h"



int main(void)
{

//probando memoria
	t_memoria* memoria = crear_memoria(100);
	agregar_stream(memoria, "asadsaf");
	agregar_stream(memoria, "boca campeon");
	agregar_stream(memoria, "chau");
	mostrar_memoria(memoria);



	for(u_int32_t i = 0; i<6; i++){
	colas_mensajes[i] = crear_cola_mensajes(i + 1);
	generador_id_suscriptor[i] = 1;
	}

	generador_id_mensaje = 1;

	iniciar_servidor();

	return EXIT_SUCCESS;
}
