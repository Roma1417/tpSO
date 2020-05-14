/*p
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

t_log* iniciar_logger(void) {

	t_log* logger = log_create("team.log","team", true, LOG_LEVEL_INFO);
	if(logger == NULL) {
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;

}

t_config* leer_config(void) {

	t_config* config = config_create("./team.config");

	return config;

}


t_config_team* construir_config_team(t_config* config){

	t_config_team* config_team = malloc(sizeof(t_config_team));

	config_team -> puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");

	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	config_team -> objetivos_entrenadores = pasar_a_lista_de_pokemon(config,"OBJETIVOS_ENTRENADORES");
	config_team -> pokemon_entrenadores = pasar_a_lista_de_pokemon(config, "POKEMON_ENTRENADORES");
	config_team -> posiciones_entrenadores = pasar_a_lista_de_posiciones(config, "POSICIONES_ENTRENADORES");

	return config_team;
}

//Codigo de prueba
void* print_message_function(){

	printf("Soy un hilo\n");

	return EXIT_SUCCESS;
}

t_list* crear_entrenadores(t_config_team* config_team){

	t_list* entrenadores = list_create();

	t_list* objetivos_entrenadores = config_team -> objetivos_entrenadores;
	t_list* pokemon_entrenadores = config_team -> pokemon_entrenadores;
	t_list* posiciones_entrenadores = config_team -> posiciones_entrenadores;

	for(int i = 0; i < list_size(objetivos_entrenadores); i++){
		t_list* objetivo = list_get(objetivos_entrenadores,i);
		t_list* pokemon_obtenidos = list_get(pokemon_entrenadores,i);
		t_posicion* posicion = list_get(posiciones_entrenadores, i);

		// Codigo en prueba
		pthread_t hilo;
	    int iret = pthread_create(&hilo, NULL, print_message_function, NULL);
	    if (iret) exit(1);

		t_entrenador* entrenador = entrenador_create(posicion, pokemon_obtenidos, objetivo, hilo);
		list_add(entrenadores, entrenador);
	}

	return entrenadores;

}

t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, (void*) get_objetivos);

}

void destruir_config_team(t_config_team* config_team){
	list_destroy(config_team->objetivos_entrenadores);
	list_destroy(config_team->pokemon_entrenadores);
	list_destroy(config_team->posiciones_entrenadores);

	free(config_team);
}

void liberar_estructuras(t_config_team* config_team, t_list* entrenadores,
		t_queue* cola_ready, t_list* objetivo_global, t_list* especies_requeridas){

	for(int i = 0; i < list_size(entrenadores); i++){
		t_entrenador* entrenador = list_get(entrenadores,i);
		destruir_entrenador(entrenador);
	}

	destruir_config_team(config_team);

	list_destroy(entrenadores);
	queue_destroy(cola_ready);
	list_destroy(objetivo_global);
	list_destroy(especies_requeridas);

}

void terminar_programa(t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
}

void enviar_mensajes_get_pokemon(int conexion, t_list* especies_requeridas){

	char** mensaje = malloc(sizeof(char*)*3);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "GET_POKEMON");

	for(int i = 0; i < list_size(especies_requeridas); i++){
		char* pokemon = list_get(especies_requeridas, i);
		mensaje[2] = pokemon;
		enviar_mensaje(mensaje, conexion);
	}

	free(mensaje[0]);
	free(mensaje[1]);
	free(mensaje);
}



void enreadyar_al_mas_cercano(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon, t_queue* cola_ready){
	t_entrenador* mas_cercano = list_get(entrenadores, 0);
	int distancia_minima = distancia(mas_cercano,appeared_pokemon);
	for(int i=1; i<list_size(entrenadores); i++){
		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if((puede_pasar_a_ready(entrenador_actual)) && (distancia(entrenador_actual,appeared_pokemon) < distancia_minima)){
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual,appeared_pokemon);
		}
	}
	cambiar_estado(mas_cercano, READY);
	queue_push(cola_ready, mas_cercano);
}

void planificar_entrenadores(t_queue* cola_ready){
	t_entrenador* entrenador_a_ejecutar = queue_pop(cola_ready);

	//Mover entrenador
}



int main (void) {
	t_list* entrenadores;
	t_log* logger = iniciar_logger();
	t_config* config = leer_config();
	t_config_team* config_team = construir_config_team(config);
	t_queue* cola_ready = queue_create();

	entrenadores = crear_entrenadores(config_team);

	t_list* objetivo_global = get_objetivo_global(entrenadores);

	t_list* especies_requeridas = eliminar_repetidos(objetivo_global);

	//int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);

	//enviar_mensajes_get_pokemon(conexion, especies_requeridas);

	t_appeared_pokemon* appeared_pokemon = iniciar_servidor(); // n

	enreadyar_al_mas_cercano(entrenadores, appeared_pokemon, cola_ready);

	planificar_entrenadores(cola_ready);

	liberar_estructuras(config_team, entrenadores, cola_ready, objetivo_global, especies_requeridas);

	terminar_programa(logger, config);

	//liberar_conexion(conexion);

	destruir_appeared_pokemon(appeared_pokemon);



	return 0;

}
