/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "utils.h"


/*cola_mensajes *colas_de_mensajes[6] =
		[cola_mensajes_create(NEW_POKEMON),
		cola_mensajes_create(APPEARED_POKEMON),
		cola_mensajes_create(CATCH_POKEMON),
		cola_mensajes_create(CAUGHT_POKEMON),
		cola_mensajes_create(GET_POKEMON),
		cola_mensajes_create(LOCALIZED_POKEMON)];*/

void* get_cola_mensajes(){


	t_config* config = config_create("colas_mensajes.config");

	printf("Ward2.1\n");

	char* direccion_cola = config_get_string_value(config, "NEW_POKEMON");

	printf("Ward2.2\n");

	printf("La direccion es %s\n", direccion_cola);

	return (void*) direccion_cola;
}

void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);

	return cadena;
}

void iniciar_servidor(void)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente; //¿Puedo sacar la ip y el ?

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);


	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_detach(thread);

}

void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}



void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
		switch (cod_op) {
		case NEW_POKEMON:
			printf("Recibí un mensaje de tipo NEW_POKEMON\n");

			size = recibir_entero(cliente_fd);

			int pokemon_size;
			char* pokemon_nombre = recibir_cadena(cliente_fd, &pokemon_size);
			printf("El pokémon recibido fue: %s\n", pokemon_nombre);

			int pos_x = recibir_entero(cliente_fd);
			printf("La pos_x del pokémon recibido fue: %d\n", pos_x);
			
			free(pokemon_nombre);


			suscriptor* nuevo_suscriptor = crear_suscriptor("ip_generica","puerto_generico");

			printf("Ward2\n");
			
			cola_mensajes* cola = get_cola_mensajes();

			printf("Ward3\n");

			printf("El codigo de la cola es %d\n", cola->id);
			printf("Ward4\n");

			cola_mensajes* cola_mensajes = cola;
			printf("Ward5\n");

			agregar_suscriptor(nuevo_suscriptor,cola);
			printf("Ward6\n");

			suscriptor* primer_sub = list_get(cola_mensajes->suscriptores, 0);

			printf("La ip es: %s\n",  primer_sub->ip );

			break;
		case APPEARED_POKEMON:
			printf("Recibí un mensaje de tipo APPEARED_POKEMON\n");
			break;
		case CATCH_POKEMON:
			printf("Recibí un mensaje de tipo CATCH_POKEMON\n");
			break;
		case CAUGHT_POKEMON:
			printf("Recibí un mensaje de tipo CAUGHT_POKEMON\n");
			break;
		case GET_POKEMON:
			printf("Recibí un mensaje de tipo GET_POKEMON\n");
			break;
		case SUSCRIPTOR:
			printf("Recibí un mensaje de tipo SUSCRIPTOR\n");

			size = recibir_entero(cliente_fd);
			printf("La size recibida fue: %d\n", size);

			int cola_size;
			char* cola_nombre = recibir_cadena(cliente_fd, &cola_size);
			printf("La cola recibida fue: %s\n", cola_nombre);

			int ip_size;
			char* ip_nombre = recibir_cadena(cliente_fd, &ip_size);
			printf("La VERDADERA ip recibida fue: %s\n", ip_nombre);


			int puerto_size;
			char* puerto_nombre = recibir_cadena(cliente_fd, &puerto_size);

			printf("La VERDADERA puerto recibida fue: %s\n", puerto_nombre);




			break;



		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}




int recibir_entero(int socket_cliente){
	int entero;

	recv(socket_cliente, &entero, sizeof(int), MSG_WAITALL);

	return entero;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(u_int32_t));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(u_int32_t));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void devolver_mensaje(void* payload, int size, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = NEW_POKEMON;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

cola_mensajes* cola_mensajes_create(tipo_mensaje id){

	cola_mensajes* cola_mensajes = malloc(sizeof(cola_mensajes));
	t_queue* mensajes = queue_create();
	t_list* suscriptores = list_create();
	cola_mensajes -> id = id;
	cola_mensajes -> mensajes = mensajes;
	cola_mensajes -> suscriptores = suscriptores;

		return cola_mensajes;
}

void cola_mensajes_destroy(cola_mensajes* self){

	queue_destroy(self -> mensajes);
	list_destroy(self -> suscriptores);
	free(self);
}






void agregar_suscriptor(suscriptor* suscriptor, cola_mensajes* cola_mensajes){
	printf("Ward5.1\n");
	list_add(cola_mensajes->suscriptores, suscriptor);
	printf("Ward5.2\n");
}


suscriptor* crear_suscriptor(char* ip, char* puerto){

	suscriptor* suscriptor = malloc(sizeof(suscriptor));
	suscriptor -> ip = ip;
	suscriptor -> puerto = puerto;

	return suscriptor;
}

tipo_mensaje obtener_tipo_mensaje(char* tipo){
	if(string_equals_ignore_case(tipo,"NEW_POKEMON")) return NEW_POKEMON;
	if(string_equals_ignore_case(tipo,"APPEARED_POKEMON")) return APPEARED_POKEMON;
	if(string_equals_ignore_case(tipo,"CATCH_POKEMON")) return CATCH_POKEMON;
	if(string_equals_ignore_case(tipo,"CAUGHT_POKEMON")) return CAUGHT_POKEMON;
	if(string_equals_ignore_case(tipo,"GET_POKEMON")) return GET_POKEMON;
	if(string_equals_ignore_case(tipo,"SUSCRIPTOR")) return SUSCRIPTOR;
	return -1;
}




