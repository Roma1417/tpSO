/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "utils.h"



void iniciar_servidor(void)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char* ip = consultar_config_por_string("./broker.config", "IP_BROKER");
    char* puerto = consultar_config_por_string("./broker.config", "PUERTO_BROKER");

    getaddrinfo(ip, puerto, &hints, &servinfo);

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
	t_cola_mensajes* cola_mensajes;

		switch (cod_op) {
		case NEW_POKEMON:
		case APPEARED_POKEMON:
		case CATCH_POKEMON:
		case CAUGHT_POKEMON:
		case GET_POKEMON:
		case LOCALIZED_POKEMON:
			printf("Recibí un mensaje de tipo %s\n", obtener_tipo_mensaje_string(cod_op));

			int size;
			void* stream = (void*)recibir_cadena(cliente_fd, &size);

			t_mensaje* mensaje = crear_mensaje(size, stream);
			cola_mensajes = get_cola_mensajes(obtener_tipo_mensaje_string(cod_op));

			agregar_mensaje(mensaje, cola_mensajes);
			
			printf("El tamaño de la cola de mensajes ahora es %d\n", queue_size(cola_mensajes->mensajes));
			t_mensaje* primer_mensaje = queue_peek(cola_mensajes->mensajes);
			printf("El primer parametro de la cola es %s\n", primer_mensaje->stream + 4);

			break;

		case SUSCRIPTOR:

			printf("Recibí un mensaje de tipo SUSCRIPTOR\n");

			size = recibir_entero(cliente_fd);
			printf("La size recibida fue: %d\n", size);

			int cola_size;
			char* cola_nombre = recibir_cadena(cliente_fd, &cola_size);
			printf("La cola recibida fue: %s\n", cola_nombre);

			int segundos = recibir_entero(cliente_fd);
			printf("La VERDADERA segundos recibida fue: %d\n", segundos);

			cola_mensajes = get_cola_mensajes(cola_nombre);

			agregar_suscriptor(&cliente_fd,cola_mensajes); //
			time_t tiempo_inicial;
			time(&tiempo_inicial);

			int* primer_sub = list_get(cola_mensajes->suscriptores, 0);

			sleep(segundos);

			eliminar_suscriptor(&cliente_fd, cola_mensajes);

			printf("El socket %d se ha desuscripto de la cola.\n", *primer_sub);

			break;
		case EXIT:
			finalizar_servidor();
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


void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);

	return cadena;
}


void* serializar_paquete(t_paquete* paquete, int bytes){
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


tipo_mensaje obtener_tipo_mensaje(char* tipo){
	if(string_equals_ignore_case(tipo,"NEW_POKEMON")) return NEW_POKEMON;
	if(string_equals_ignore_case(tipo,"APPEARED_POKEMON")) return APPEARED_POKEMON;
	if(string_equals_ignore_case(tipo,"CATCH_POKEMON")) return CATCH_POKEMON;
	if(string_equals_ignore_case(tipo,"CAUGHT_POKEMON")) return CAUGHT_POKEMON;
	if(string_equals_ignore_case(tipo,"GET_POKEMON")) return GET_POKEMON;
	if(string_equals_ignore_case(tipo,"EXIT")) return EXIT;
	return -1;
}


char* obtener_tipo_mensaje_string(tipo_mensaje tipo){
	if(tipo == NEW_POKEMON) return "NEW_POKEMON";
	if(tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	if(tipo == CATCH_POKEMON) return "CATCH_POKEMON";
	if(tipo == CAUGHT_POKEMON) return "CAUGHT_POKEMON";
	if(tipo == GET_POKEMON) return "GET_POKEMON";
	if(tipo == SUSCRIPTOR) return "SUSCRIPTOR";
	if(tipo == EXIT) return "EXIT";
	return "DESCONOCIDO";
}

char* consultar_config_por_string(char* path, char* key){

	t_config* config = config_create(path);

	char* cadena = string_from_format("%s",config_get_string_value(config, key));

	config_destroy(config);

	return cadena;
}


void finalizar_servidor(){

	for (int i=1; i<=6; i++){
		t_cola_mensajes* cola_mensajes = get_cola_mensajes(obtener_tipo_mensaje_string(i));
		cola_mensajes_destroy(cola_mensajes);
	}
	exit(2);
}




