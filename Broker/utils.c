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
	struct sockaddr_in dir_cliente;

	u_int32_t tam_direccion = sizeof(struct sockaddr_in); //capaz no sea unsigned

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_join(thread, NULL);

}


void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}


void process_request(int cod_op, int cliente_fd) {
	t_cola_mensajes* cola_mensajes;

	//u_int32_t id_mensaje = recibir_entero(cliente_fd);


		switch (cod_op) {
		case NEW_POKEMON:
		case APPEARED_POKEMON:
		case CATCH_POKEMON:
		case CAUGHT_POKEMON:
		case GET_POKEMON:
		case LOCALIZED_POKEMON:
			printf("Recibí un mensaje de tipo %s\n", (char*) obtener_tipo_mensaje_string(cod_op));

			int32_t size;
			void* stream = (void*)recibir_cadena(cliente_fd, &size);
			printf("Parametro cadena %s\n",(char*) (stream+4));

			t_buffer* buffer = crear_buffer(size, stream);
			t_paquete* paquete = crear_paquete(generar_id_mensaje(), cod_op, buffer);
			t_mensaje* mensaje = crear_mensaje(paquete);
			cola_mensajes = get_cola_mensajes(cod_op);

			queue_push(cola_mensajes->mensajes, mensaje);

			printf("Asignado el mensaje de ID %d a la cola %s\n", mensaje->paquete->id_mensaje, obtener_tipo_mensaje_string(cola_mensajes->id));

			printf("El tamaño de la cola de mensajes ahora es %d\n", queue_size(cola_mensajes->mensajes));

			//enviar_a_suscriptores(mensaje, cola_mensajes->suscriptores);
			/*t_suscriptor* sub = list_get(cola_mensajes->suscriptores, 0);
			printf("ID suscriptor %d\n", sub->id);
			enviar_mensaje(paquete, cliente_fd);
			printf("Socket suscriptor %d\n", sub->numero_socket);
			printf("ID mensaje %d\n", paquete->id_mensaje);
			enviar_mensaje(paquete, sub->numero_socket);
			printf("El tamaño de la cola de mensajes ahora es %d\n", queue_size(cola_mensajes->mensajes));
			*/
			break;

		case SUSCRIPTOR:

			printf("Recibí un mensaje de tipo SUSCRIPTOR\n");

			size = recibir_entero(cliente_fd);
			printf("La size recibida fue: %d\n", size);

			int32_t cola_size;
			char* cola_nombre = recibir_cadena(cliente_fd, &cola_size);
			printf("La cola recibida fue: %s\n", cola_nombre);

			u_int32_t id_suscriptor = recibir_entero(cliente_fd);
			printf("La id recibida fue: %d\n", id_suscriptor);

			cola_mensajes = get_cola_mensajes(obtener_tipo_mensaje(cola_nombre));

			t_suscriptor* suscriptor;

			if(id_suscriptor == 0){

				 suscriptor = agregar_suscriptor(cliente_fd,cola_mensajes);

				printf("Asignada ID a nuevo suscriptor:%d\n", suscriptor->id);
				notificar_suscriptor(suscriptor, cola_mensajes->id);
			} else {
				suscriptor = actualizar_suscriptor(cliente_fd, cola_mensajes, id_suscriptor);
				printf("Actualizado el socket del suscriptor %d: %d\n", suscriptor->id, suscriptor->numero_socket);
			}

			printf("Cantidad de subs en la cola %s: %d\n", cola_nombre, list_size(cola_mensajes->suscriptores));



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


tipo_mensaje obtener_tipo_mensaje(char* tipo){
	if(string_equals_ignore_case(tipo,"NEW_POKEMON")) return NEW_POKEMON;
	if(string_equals_ignore_case(tipo,"APPEARED_POKEMON")) return APPEARED_POKEMON;
	if(string_equals_ignore_case(tipo,"CATCH_POKEMON")) return CATCH_POKEMON;
	if(string_equals_ignore_case(tipo,"CAUGHT_POKEMON")) return CAUGHT_POKEMON;
	if(string_equals_ignore_case(tipo,"GET_POKEMON")) return GET_POKEMON;
	if(string_equals_ignore_case(tipo, "LOCALIZED_POKEMON")) return LOCALIZED_POKEMON;
	if(string_equals_ignore_case(tipo,"EXIT")) return EXIT;
	return -1;
}


char* obtener_tipo_mensaje_string(tipo_mensaje tipo){
	if(tipo == NEW_POKEMON) return "NEW_POKEMON";
	if(tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	if(tipo == CATCH_POKEMON) return "CATCH_POKEMON";
	if(tipo == CAUGHT_POKEMON) return "CAUGHT_POKEMON";
	if(tipo == GET_POKEMON) return "GET_POKEMON";
	if(tipo == LOCALIZED_POKEMON) return "LOCALIZED";
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
		t_cola_mensajes* cola_mensajes = get_cola_mensajes(i);
		printf("Tipo msj %s\n", obtener_tipo_mensaje_string(i));
		destruir_cola_mensajes(cola_mensajes);
	}
	exit(2);
}


void* serializar_paquete(t_paquete* paquete, size_t bytes){
	void* magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->tipo_mensaje), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, &(paquete->id_mensaje), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}


void notificar_suscriptor(t_suscriptor* suscriptor, tipo_mensaje tipo_mensaje){ //envio al suscriptor un mensaje de argumentos (id, SUSCRIPTOR, cola)
	void* stream = malloc(sizeof(tipo_mensaje));
	memcpy(stream, &tipo_mensaje, sizeof(tipo_mensaje));
	t_buffer* buffer = crear_buffer(sizeof(tipo_mensaje), stream);
	t_paquete* paquete = crear_paquete(suscriptor->id, SUSCRIPTOR, buffer);

	enviar_mensaje(paquete, suscriptor->numero_socket);

	destruir_paquete(paquete);
}


void enviar_mensaje(t_paquete* paquete, int socket){
	printf("Ward 3.-1\n");

	size_t bytes = paquete->buffer->size + 3*sizeof(int32_t);
	printf("Ward 3\n");
	void* a_enviar = serializar_paquete(paquete, bytes);
	printf("%s\n",(char*) (a_enviar +12));
	printf("Ward 3.1\n");
	send(socket, a_enviar, bytes, MSG_NOSIGNAL);
	printf("Ward 3.2\n");
	free(a_enviar);
}

void enviar_a_suscriptor(t_mensaje* mensaje, t_suscriptor* suscriptor){
	printf("Ward 2\n");
	enviar_mensaje(mensaje->paquete, suscriptor->numero_socket);
	printf("Ward 2.1\n");
	list_add(mensaje->suscriptores_enviados, suscriptor);
}

void enviar_a_suscriptores(t_mensaje* mensaje, t_list* suscriptores){
	printf("Ward 1\n");
	t_suscriptor* suscriptor = list_get(suscriptores, 0);
	enviar_mensaje(mensaje->paquete, suscriptor->numero_socket);
	for (u_int32_t i = 0; i < list_size(suscriptores); i++){
		t_suscriptor* suscriptor = (void*)list_get(suscriptores, i);
		printf("Ward 1.1\n");
		enviar_a_suscriptor(mensaje, suscriptor);
		printf("Se envio el mensaje de ID %d al suscriptor %d\n", mensaje->paquete->id_mensaje, suscriptor->id);
	}

}

