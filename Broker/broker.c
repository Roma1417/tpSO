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


	set_cola_mensajes("NEW_POKEMON");
	set_cola_mensajes("APPEARED_POKEMON");
	set_cola_mensajes("CATCH_POKEMON");
	set_cola_mensajes("CAUGHT_POKEMON");
	set_cola_mensajes("GET_POKEMON");
	set_cola_mensajes("LOCALIZED_POKEMON");



	/*printf("La direccion es %d\n", direccion);
	printf("La direccion es %d\n", cola);
	printf("La direccion es %d\n", config_get_int_value(config, "NEW_POKEMON"));
	printf("El puntero de la lista de subs es %p\n", cola->suscriptores);*/









	iniciar_servidor();
	
	return EXIT_SUCCESS;
}
