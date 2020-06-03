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
	    pthread_create(&hilo, NULL, print_message_function, NULL);

		t_entrenador* entrenador = entrenador_create(posicion, pokemon_obtenidos, objetivo, hilo);
		list_add(entrenadores, entrenador);
	}

	return entrenadores;

}

t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, (void*) get_objetivos_faltantes);

}

void destruir_config_team(t_config_team* config_team){
	list_destroy(config_team->objetivos_entrenadores);
	list_destroy(config_team->pokemon_entrenadores);
	list_destroy(config_team->posiciones_entrenadores);

	free(config_team);
}

void destruir_appeared_pokemons(){
	for (int i = 0; i<list_size(appeared_pokemons); i++){
		t_appeared_pokemon* appeared_pokemon = list_get(appeared_pokemons, i);
		appeared_pokemon_destroy(appeared_pokemon);
	}

	list_destroy(appeared_pokemons);
}

void liberar_estructuras(t_config_team* config_team, t_list* entrenadores,
		t_queue* cola_ready, t_list* objetivo_global, t_list* especies_requeridas){

	for(int i = 0; i < list_size(entrenadores); i++){
		t_entrenador* entrenador = list_get(entrenadores,i);
		entrenador_destroy(entrenador);
	}

	destruir_config_team(config_team);
	destruir_appeared_pokemons();

	list_destroy(entrenadores);
	queue_destroy(cola_ready);
	list_destroy(objetivo_global);

	list_iterate(especies_requeridas, free);
	list_destroy(especies_requeridas);

}

void terminar_programa(t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
}

void* enviar_get_pokemon(void* pokemon){
	printf("Ward1\n");
	char** mensaje = malloc(sizeof(char*)*3);
	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "GET_POKEMON");

	mensaje[2] = (char*)pokemon;
	printf("Pokemon: %s\n", mensaje[2]);
	enviar_mensaje(mensaje, conexion);
	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

void enviar_mensajes_get_pokemon(){

	pthread_t get_pokemon[list_size(especies_requeridas)];
	t_especie* especie;
	for(int i=0; i<list_size(especies_requeridas); i++){
		especie = list_get(especies_requeridas, i);
		pthread_create(&(get_pokemon[i]),NULL, enviar_get_pokemon, (void*) especie->nombre);
		pthread_join(get_pokemon[i],NULL);
	}

	/*int conexion[list_size(especies_requeridas)];
	t_especie* pokemon;

	char** mensaje = malloc(sizeof(char*)*3);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "GET_POKEMON");

	for(int i = 0; i < list_size(especies_requeridas); i++){
		conexion[i] = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
		pokemon = list_get(especies_requeridas, i);
		mensaje[2] = pokemon->nombre;
		enviar_mensaje(mensaje, conexion[i]);
		liberar_conexion(conexion[i]);
	}

	free(mensaje[0]);
	free(mensaje[1]);
	free(mensaje);*/
}

void enreadyar_al_mas_cercano(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon){
	t_entrenador* mas_cercano = list_find(entrenadores, puede_pasar_a_ready);
	int distancia_minima = distancia(mas_cercano,appeared_pokemon);
	for(int i=1; i<list_size(entrenadores); i++){
		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if(puede_pasar_a_ready(entrenador_actual) && (distancia(entrenador_actual,appeared_pokemon) < distancia_minima)){
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual,appeared_pokemon);
		}
	}
	cambiar_estado(mas_cercano, READY);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	queue_push(cola_ready, planificado);
}

void planificar_entrenadores(){
	t_planificado* planificado = queue_pop(cola_ready);

	//Mover entrenador

}

void* mantener_servidor(){

	printf("Soy un servidor\n");

	iniciar_servidor();

	return EXIT_SUCCESS;
}

void* iniciar_planificador(){

	while(1)
		planificar_entrenadores();

	return EXIT_SUCCESS;
}

void* iniciar_planificador_largo_plazo(void* parametro){
	t_list* entrenadores = parametro;

	while(1){
		//Poner semáforos
		sem_wait(&sem_appeared_pokemon);
		sem_wait(&sem_entrenadores); //Falta el signal cuando el entrenador se bloquea
		t_appeared_pokemon* appeared_pokemon = list_get(appeared_pokemons, 0);
		enreadyar_al_mas_cercano(entrenadores, appeared_pokemon);
	}


	return EXIT_SUCCESS;
}

bool elem_especies(t_list* especies, char* pokemon){
	bool encontrado = false;
	for (int i = 0; i < list_size(especies) && !encontrado; i++){
		t_especie* especie = list_get(especies, i);
		encontrado = string_equals_ignore_case(especie->nombre,pokemon);
		if (encontrado) especie->cantidad++;
	}
	return encontrado;
}

t_list* obtener_especies(t_list* objetivo_global){

	t_list* especies = list_create();

	for (int i = 0; i < list_size(objetivo_global); i++){
		// if esta en la lista -> Aumento la cantidad
		// else -> lo agrego con cantidad 1

		char* pokemon = list_get(objetivo_global, i);

		if (elem_especies(especies, pokemon)){}
		else {
			t_especie* especie = malloc(sizeof(t_especie));
			especie->nombre = pokemon;
			especie->cantidad = 1;
			list_add(especies, especie);
		}

	}

	return especies;
}

void imprimir(int n){
	printf("Eclipse no funciona igual -> F\n");
}

void liberar_todo(int n){
	printf("\n Intento terminar el programa pero... \n"); // Karen ayuda pls
	list_clean(objetivo_global);
}

void* suscribirse(void* cola){
	char* msg = (char *)cola;
	// No acepta el config team global -> no se puede establecer conexion

	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);

	char** mensaje = malloc(sizeof(char*)*4);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "SUSCRIPTOR");

	mensaje[2] = string_new();
	string_append(&(mensaje[2]), msg);

	mensaje[3] = string_itoa(id_team);

	// Y si establece conexion, no se envian mensajes bien
	enviar_mensaje(mensaje, conexion);

	//Falta esperar la respuesta

	// Problema para ale -> no liberar conexion
	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

void suscribirse_a_colas(){

	pthread_t hilo_appeared;
	pthread_t hilo_localized;
	pthread_t hilo_caught;

	// REVISAR

	char* mensaje = string_new();
	string_append(&mensaje, "APPEARED_POKEMON");
	pthread_create(&hilo_appeared, NULL, suscribirse,(void*) mensaje);
	pthread_join(hilo_appeared, NULL); //Vamos a tener que mover el join
	free(mensaje);

	mensaje = string_new();
	string_append(&mensaje, "LOCALIZED_POKEMON");
	pthread_create(&hilo_localized, NULL, suscribirse,(void*) mensaje);
	pthread_join(hilo_appeared, NULL);
	free(mensaje);

	mensaje = string_new();
	string_append(&mensaje, "CAUGHT_POKEMON");
	pthread_create(&hilo_caught, NULL, suscribirse,(void*) mensaje);
	pthread_join(hilo_appeared, NULL);
	free(mensaje);

}

int main (void) {

	signal(SIGTERM, imprimir); // Mostrar
	signal(SIGINT,liberar_todo); // Mostrar

	t_list* entrenadores;
	t_log* logger = iniciar_logger();
	t_config* config = leer_config();

	// REVISAR SI ES NECESARIO QUE SEA GLOBAL
	config_team = construir_config_team(config);

	cola_ready = queue_create();
	appeared_pokemons = list_create();
	id_team = 0;

	sem_init(&sem_appeared_pokemon, 0, 0);

	entrenadores = crear_entrenadores(config_team);

	sem_init(&sem_entrenadores, 0, list_size(entrenadores));

	//suscribirse_a_colas(); // REVISAR

	t_list* auxiliar = get_objetivo_global(entrenadores);
	objetivo_global = list_flatten(auxiliar);
	list_destroy(auxiliar);

	especies_requeridas = obtener_especies(objetivo_global);

	enviar_mensajes_get_pokemon(); // REVISAR

	pthread_t hilo_servidor;
	pthread_create(&hilo_servidor, NULL, mantener_servidor, NULL);

	pthread_t hilo_planificador_largo_plazo;
	pthread_create(&hilo_planificador_largo_plazo, NULL, iniciar_planificador_largo_plazo, (void*) entrenadores);

	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador, NULL, iniciar_planificador, NULL);

    //enreadyar_al_mas_cercano(entrenadores, appeared_pokemon, cola_ready);
	//planificar_entrenadores(cola_ready);

	pthread_join(hilo_planificador, NULL);
	pthread_join(hilo_servidor, NULL);

	liberar_estructuras(config_team, entrenadores, cola_ready, objetivo_global, especies_requeridas);

	terminar_programa(logger, config);

	//liberar_conexion(conexion);

	return 0;

}
