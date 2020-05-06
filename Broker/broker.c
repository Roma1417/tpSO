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




	t_config* config = config_create("colas_mensajes.config");

	config_set_value(config, "NEW_POKEMON", cola_mensajes_create(NEW_POKEMON));

	config_save(config);







	iniciar_servidor();
	
	return EXIT_SUCCESS;
}
