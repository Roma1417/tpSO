/*p
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

/**
* @NAME: iniciar_logger
* @DESC: Crea y devuelve un puntero a una estructura t_log,
* 		 que sirve como log del proceso Team.
*/
t_log* iniciar_logger(char* path) {

	t_log* logger = log_create(path,"team", true, LOG_LEVEL_INFO);
	if(logger == NULL) {
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;

}

/**
* @NAME: leer_config
* @DESC: Crea y devuelve un puntero a una estructura t_config.
*/
t_config* leer_config(void) {
	t_config* config = config_create("./team.config");
	return config;
}

/**
* @NAME: construir_config_team
* @DESC: Crea y devuelve un puntero a una estructura t_config_team,
* 		 donde sus valores internos corresponden a los guardados en
* 		 el archivo del team de tipo config.
*/
t_config_team* construir_config_team(t_config* config){

	t_config_team* config_team = malloc(sizeof(t_config_team));

	config_team -> puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");

	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	config_team -> alpha = config_get_double_value(config, "ALPHA");
	// alpha debe ser float? o double?
	// si es float -> 32bits
	// si es double -> 64bits

	config_team -> objetivos_entrenadores = pasar_a_lista_de_pokemon(config,"OBJETIVOS_ENTRENADORES");
	config_team -> pokemon_entrenadores = pasar_a_lista_de_pokemon(config, "POKEMON_ENTRENADORES");
	config_team -> posiciones_entrenadores = pasar_a_lista_de_posiciones(config, "POSICIONES_ENTRENADORES");
	return config_team;
}

//Codigo de prueba
void* ejecutar_entrenador(void* parametro){
	t_entrenador* entrenador = parametro;

	while(puede_seguir_atrapando(entrenador)){
		sem_wait(&(puede_ejecutar[entrenador->indice]));
		mover_de_posicion(entrenador->posicion, pokemon_a_atrapar->posicion, config_team);

		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);

		enviar_catch_pokemon(entrenador, pokemon_a_atrapar);
		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);
		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		sem_wait(&(llega_mensaje_caught[entrenador->indice]));

		if(entrenador->resultado_caught){
			cambiar_estado(entrenador, READY);
			queue_push(cola_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));
			atrapar(entrenador, planificado->pokemon);
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, planificado->pokemon->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		if(puede_seguir_atrapando(entrenador)) cambiar_condicion_ready(entrenador);
		sem_post(&puede_planificar);
		sem_post(&sem_entrenadores);
	}

	// wait semaforo todos

	/* Implementar un nuevo hilo

	while(!cumplio_su_objetivo(entrenador)){
		sem_wait(&(puede_ejecutar[entrenador->indice]));


	}*/

	sem_post(&(termino_de_capturar[entrenador->indice]));

	return EXIT_SUCCESS;
}


void enviar_catch_pokemon(t_entrenador* entrenador, t_appeared_pokemon* pokemon){

	char** mensaje = malloc(sizeof(char*)*5);
	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "CATCH_POKEMON");

	mensaje[2] = pokemon->pokemon;
	mensaje[3] = string_itoa(pokemon->posicion->x);
	mensaje[4] = string_itoa(pokemon->posicion->y);

	enviar_mensaje(mensaje, conexion);

	asignar_id_caught(entrenador, conexion);

	liberar_conexion(conexion);
}

/**
* @NAME: crear_entrenadores
* @DESC: Crea y devuelve un puntero a una t_list con referencias a
*        estructuras t_entrenador, inicializados con los valores guardados
*        en una estructura t_config_team pasada por parametro.
*/
t_list* crear_entrenadores(t_config_team* config_team){

	t_list* entrenadores = list_create();

	t_list* objetivos_entrenadores = config_team -> objetivos_entrenadores;
	t_list* pokemon_entrenadores = config_team -> pokemon_entrenadores;
	t_list* posiciones_entrenadores = config_team -> posiciones_entrenadores;

	for(int i = 0; i < list_size(objetivos_entrenadores); i++){
		t_list* objetivo = list_get(objetivos_entrenadores,i);
		t_list* pokemon_obtenidos = list_get(pokemon_entrenadores,i);
		t_posicion* posicion = list_get(posiciones_entrenadores, i);

		t_entrenador* entrenador = entrenador_create(posicion, pokemon_obtenidos, objetivo, i, config_team->estimacion_inicial);
		pthread_t hilo;
		pthread_create(&hilo, NULL, ejecutar_entrenador_SJF, (void*) entrenador);
		set_hilo(entrenador, hilo);
		list_add(entrenadores, entrenador);
		list_add(entrenadores_deadlock, entrenador);
	}

	return entrenadores;
}

/**
* @NAME: get_objetivo_global
* @DESC: Crea y devuelve un puntero a una t_list con los objetivos que faltan
* 		 atrapar por parte de los entrenadores.
*/
t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, (void*) get_objetivos_faltantes);

}

/**
* @NAME: destruir_config_team
* @DESC: Destruye la estructura t_config_team pasada por parametro.
*/
void destruir_config_team(t_config_team* config_team){
	list_destroy(config_team->objetivos_entrenadores);
	list_destroy(config_team->pokemon_entrenadores);
	list_destroy(config_team->posiciones_entrenadores);

	free(config_team);
}

/**
* @NAME: destruir_appeared_pokemons
* @DESC: Destruye la lista de t_appeared_pokemon.
*/
void destruir_appeared_pokemons(){
	while(!(queue_is_empty(appeared_pokemons))){
		t_appeared_pokemon* appeared_pokemon = queue_pop(appeared_pokemons);
		appeared_pokemon_destroy(appeared_pokemon);
	}

	queue_destroy(appeared_pokemons);
}

/*
* @NAME: liberar_estructuras
* @DESC: Dados un conjunto de estructuras pasados por parametro, se encarga
* 		 de destruirlas y liberar toda la memoria tomada por ellas.
* @PARAM: config_team, entrenadores, cola_ready, objetivo_global y
* 	      especies_requeridas.
*/
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

/*
* @NAME: terminar_programa
* @DESC: Destruye las estructuras t_log y t_config pasadas por parametro.
*/
void terminar_programa(t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
}

/*
* @NAME: enviar_get_pokemon
* @DESC: Establece conexion con el broker y envia un mensaje GET_POKEMON,
* 		 para un pokemon pasado por parametro.
* 		 Recordar -> Pokemon requiere casteo a (char*).
*/
void* enviar_get_pokemon(void* parametro){
	char* pokemon = parametro;
	char** mensaje = malloc(sizeof(char*)*3);
	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "GET_POKEMON");

	mensaje[2] = pokemon;
	printf("Pokemon enviado: %s\n", pokemon);
	enviar_mensaje(mensaje, conexion);
	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

/*
* @NAME: enviar_mensajes_get_pokemon
* @DESC: Envia un mensaje get_pokemon al broker por cada especie requerida.
*/
void enviar_mensajes_get_pokemon(){

	pthread_t get_pokemon[list_size(especies_requeridas)];
	t_especie* especie;
	for(int i=0; i<list_size(especies_requeridas); i++){
		especie = list_get(especies_requeridas, i);
		pthread_create(&(get_pokemon[i]),NULL, enviar_get_pokemon, (void*) especie->nombre);
		pthread_join(get_pokemon[i],NULL);
	}
}

/*
 * @NAME: enreadyar_al_mas_cercano (FUE IDEA DE JOSI)
 * @DESC: Dados una lista de entrenadores y un appeared_pokemon, pone
 * 		  en estado READY al entrenador mas cercano a ese pokemon.
 */
void enreadyar_al_mas_cercano(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon){
	t_entrenador* mas_cercano = list_find(entrenadores, puede_ser_planificado);
	int distancia_minima = distancia(mas_cercano->posicion,appeared_pokemon->posicion);

	for(int i=1; i<list_size(entrenadores); i++){
		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if(puede_ser_planificado(entrenador_actual) && (distancia(entrenador_actual->posicion,appeared_pokemon->posicion) < distancia_minima)){
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual->posicion,appeared_pokemon->posicion);
		}
	}
	cambiar_estado(mas_cercano, READY);
	cambiar_condicion_ready(mas_cercano);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	queue_push(cola_ready, planificado);
}

void intercambiar_pokemon(t_entrenador* entrenador, t_planificado* planificado){

	t_entrenador* donador = planificado->entrenador;
	sem_wait(&(puede_ejecutar[donador->indice]));
	mover_de_posicion(donador->posicion, entrenador->posicion, config_team);

	log_info(logger_team, "El entrenador %d esta en la posicion (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
	log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", donador->indice, donador->posicion->x, donador->posicion->y);

	char* inservible = find_first(donador->objetivos_faltantes, entrenador->pokemon_inservibles);

	intercambiar(entrenador, planificado->pokemon->pokemon, inservible);
	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", entrenador->indice, planificado->pokemon->pokemon, inservible);
	intercambiar(donador, inservible, planificado->pokemon->pokemon);
	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", donador->indice, inservible,  planificado->pokemon->pokemon);

	// Hay un error
	// No saca los inservibles bien del entrenador
	// pero si lo hace del donador
	// F <- Hay que arreglarlo

	/*printf("entrenador: %d\n", entrenador->indice);

	for(int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++){
		char* pokemon = list_get(entrenador->pokemon_obtenidos,i);
		printf("obtenido: %s\n", pokemon);
	}

	for(int i = 0; i < list_size(entrenador->objetivos_faltantes); i++){
		char* pokemon = list_get(entrenador->objetivos_faltantes,i);
		printf("objetivo faltante: %s\n", pokemon);
	}

	for(int i = 0; i < list_size(entrenador->pokemon_inservibles); i++){
		char* pokemon = list_get(entrenador->pokemon_inservibles,i);
		printf("pokemon inservibles: %s\n", pokemon);
	}

	printf("donador: %d\n", donador->indice);

	for(int i = 0; i < list_size(donador->pokemon_obtenidos); i++){
		char* pokemon = list_get(donador->pokemon_obtenidos,i);
		printf("obtenido: %s\n", pokemon);
	}

	for(int i = 0; i < list_size(donador->objetivos_faltantes); i++){
		char* pokemon = list_get(donador->objetivos_faltantes,i);
		printf("objetivo faltante: %s\n", pokemon);
	}

	for(int i = 0; i < list_size(donador->pokemon_inservibles); i++){
		char* pokemon = list_get(donador->pokemon_inservibles,i);
		printf("pokemon inservibles: %s\n", pokemon);
	}*/

	sem_post(&puede_intercambiar);

}

t_planificado* buscar_donador(t_entrenador* entrenador){
	t_planificado* planificado = NULL;
	bool encontro_donador = false;
	for(int i = 0; i < list_size(entrenadores_deadlock) && !encontro_donador;i++){
		t_entrenador* donador = list_get(entrenadores_deadlock, i);
		for(int j = 0; j < list_size(donador->pokemon_inservibles) && !encontro_donador; j++){
			char* obtenido = list_get(donador->pokemon_inservibles, j);
			encontro_donador = list_elem(obtenido, entrenador->objetivos_faltantes);
			if (encontro_donador){
				t_appeared_pokemon* pokemon = appeared_pokemon_create();
				cambiar_nombre_pokemon(pokemon, obtenido);
				cambiar_estado(donador, READY);
				cambiar_condicion_ready(donador);
				printf("pokemon a donar por el entrenador %d: %s\n", donador->indice, pokemon->pokemon);
				planificado = planificado_create(donador, pokemon);
				queue_push(cola_ready, planificado);
				sem_post(&puede_planificar);
			}
		}
	}

	return planificado;
}

void realizar_intercambios(){
	// Falta implementar en otro hilo
	// while segun ALE
	// hasta que se cumplio el objetivo posta posta (literal) segun JOSI
	// implementar con el entrenador global en principio
	// Usamos lista entrenadores_deadlock
	// Buscar dos en deadlock y mandar a uno a planificar y el
	// otro seria variable global, segun ALE

	for(int i = 0; i < list_size(entrenadores); i++){
		sem_wait(&(termino_de_capturar[i]));
	}

	log_info(logger_team, "Iniciando el algoritmo de deteccion de Deadlock...");

	// Ahora vendria lo que dijo ale
	// Agarro a un entrenador

	while(!list_is_empty(entrenadores_deadlock)){
		sem_wait(&puede_intercambiar);
		t_entrenador* entrenador = list_head(entrenadores_deadlock);
		if(!list_is_empty(entrenadores_deadlock)){
			t_planificado* planificado = buscar_donador(entrenador);
			intercambiar_pokemon(entrenador, planificado);
			if(no_cumplio_su_objetivo(entrenador))
				list_add(entrenadores_deadlock, entrenador);
		}
	}

	log_info(logger_team, "Fin del algoritmo de deteccion de Deadlock...");

}

/*
 * @NAME: planificar_entrenadores
 * @DESC: pendiente
 */
void planificar_entrenadores(){
	//Falta un switch para planificar según cada algoritmo
	if(!queue_is_empty(cola_ready)){
		sem_wait(&puede_planificar);
		t_planificado* planificado = queue_pop(cola_ready);
		pokemon_a_atrapar = planificado->pokemon;
		cambiar_estado(planificado->entrenador, EXEC);
		sem_post(&(puede_ejecutar[planificado->entrenador->indice]));
	}

	//Mover entrenador
}

// En realidad no creo que sea necesario crear una nueva funcion para esto...
// Simplemente voy a cambiar la estructura de la cola de ready por una lista
// De esa manera voy a poder elegir a cualquier entrenador en ready y no solo
// al primero
// Por ahi agregue otras funcionalidades como usar la distancia minima
// para guardarla en
void enreadyar_al_mas_cercano_SJF(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon){
	t_entrenador* mas_cercano = list_find(entrenadores, puede_ser_planificado);
	int distancia_minima = distancia(mas_cercano->posicion,appeared_pokemon->posicion);

	for(int i=1; i<list_size(entrenadores); i++){

		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if(puede_ser_planificado(entrenador_actual) && (distancia(entrenador_actual->posicion,appeared_pokemon->posicion) < distancia_minima)){
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual->posicion,appeared_pokemon->posicion);
		}
	}
	cambiar_estado(mas_cercano, READY);
	cambiar_condicion_ready(mas_cercano);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	list_add(lista_ready, planificado);
}

// Codigo de prueba
// Pensar donde calcular la proxima estimacion :O
t_planificado* elegir_proximo_a_ejecutar_SJF(){
	int j = 0;
	t_planificado* planificado = list_get(lista_ready,j);
	t_entrenador* entrenador = planificado->entrenador;
	float estimacion_a_ejecutar = entrenador->estimacion;

	for(int i = 0; i < list_size(lista_ready); i++){
		t_planificado* planificado_auxiliar = list_get(lista_ready,i);
		t_entrenador* auxiliar = planificado_auxiliar->entrenador;
		float estimacion_actual = auxiliar->estimacion;
		if (estimacion_a_ejecutar > estimacion_actual){
			estimacion_a_ejecutar = estimacion_actual;
			j = i;
		}
	}

	planificado = list_remove(lista_ready, j);

	return planificado;
}

void planificar_entrenadores_SJF(){
	if(!list_is_empty(lista_ready)){
		sem_wait(&puede_planificar);
		t_planificado* planificado = elegir_proximo_a_ejecutar_SJF();
		pokemon_a_atrapar = planificado->pokemon;
		cambiar_estado(planificado->entrenador, EXEC);
		sem_post(&(puede_ejecutar[planificado->entrenador->indice]));
	}
}

void* ejecutar_entrenador_SJF(void* parametro){
	t_entrenador* entrenador = parametro;

	while(puede_seguir_atrapando(entrenador)){
		sem_wait(&(puede_ejecutar[entrenador->indice]));

		u_int32_t rafaga = distancia(entrenador->posicion, pokemon_a_atrapar->posicion);
		mover_de_posicion(entrenador->posicion, pokemon_a_atrapar->posicion, config_team);
		float estimacion = calcular_estimado_de_la_proxima_rafaga(entrenador->estimacion, rafaga, config_team->alpha);

		printf("rafaga: %d\n", rafaga);
		printf("estimacion: %f\n", estimacion);
		printf("estimacion anterior: %f\n", entrenador->estimacion);

		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);

		enviar_catch_pokemon(entrenador, pokemon_a_atrapar);
		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);
		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		sem_wait(&(llega_mensaje_caught[entrenador->indice]));

		if(entrenador->resultado_caught){
			cambiar_estado(entrenador, READY);
			list_add(lista_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));

			// Comentar si en esta parte hay que calcular la estimacion para
			// rafaga CPU por atrapar = 1

			atrapar(entrenador, planificado->pokemon);
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, planificado->pokemon->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		if(puede_seguir_atrapando(entrenador)) cambiar_condicion_ready(entrenador);
		sem_post(&puede_planificar);
		sem_post(&sem_entrenadores);
	}

	sem_post(&(termino_de_capturar[entrenador->indice]));

	return EXIT_SUCCESS;
}


/*
 * @NAME: mantener_servidor
 * @DESC: Inicia y mantiene el servidor abierto para escuchar los mensajes
 * 		  que le manden al proceso Team.
 */
void* mantener_servidor(){

	iniciar_servidor();

	return EXIT_SUCCESS;
}

void* iniciar_intercambiador(){
	//while(1)
		realizar_intercambios();

	return EXIT_SUCCESS;
}

/*
 * @NAME: iniciar_planificador
 * @DESC: pendiente
 */
void* iniciar_planificador(){

	while(!pokemons_objetivo_fueron_atrapados()){ // hasta que todos terminen
		//planificar_entrenadores();
		planificar_entrenadores_SJF();
	}

	return EXIT_SUCCESS;
}

/*
 * @NAME: iniciar_planificador_largo_plazo
 * @DESC: Si aparece un pokemon, se encarga de poner en ready al
 * 		  entrenador mas cercano.
 */
void* iniciar_planificador_largo_plazo(void* parametro){
	t_list* entrenadores = parametro;

	while(1){
		//Poner semáforos
		sem_wait(&sem_appeared_pokemon);
		sem_wait(&sem_entrenadores); //Falta el signal cuando el entrenador se bloquea
		t_appeared_pokemon* appeared_pokemon = queue_pop(appeared_pokemons);
		//enreadyar_al_mas_cercano(entrenadores, appeared_pokemon);
		enreadyar_al_mas_cercano_SJF(entrenadores, appeared_pokemon);
	}

	return EXIT_SUCCESS;
}

/*
 * @NAME: elem_especies
 * @DESC: Dados una lista de especies y el nombre de una especie,
 * 		  se fija si ese nombre esta entre las especies.
 */
bool elem_especies(t_list* especies, char* pokemon){
	bool encontrado = false;
	for (int i = 0; i < list_size(especies) && !encontrado; i++){
		t_especie* especie = list_get(especies, i);
		encontrado = string_equals_ignore_case(especie->nombre,pokemon);
		if (encontrado) especie->cantidad++;
	}
	return encontrado;
}

/*
 * @NAME: obtener_especies
 * @DESC: Dada una lista con el objetivo global,
 * 		  crea una lista con referencias a estructuras t_especies.
 * 		  Estas especies tienen el nombre de la especie y la cantidad
 * 		  de apariciones en el objetivo global.
 */
t_list* obtener_especies(){

	t_list* especies = list_create();

	for (int i = 0; i < list_size(objetivo_global); i++){
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

// Codigo de prueba
void imprimir(int n){
	printf("Eclipse no funciona igual -> F\n");
}

// Codigo de prueba
void liberar_todo(int n){
	printf("\n Intento terminar el programa pero... \n"); // Karen ayuda pls
	list_clean(objetivo_global);
	exit(1);
}

/*
 * @NAME: suscribirse
 * @DESC: Dado el nombre de una cola, se suscribe a esa cola de mensajes del
 * 		  broker.
 */
void* suscribirse(void* cola){
	char* msg = (char *)cola;

	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);

	char** mensaje = malloc(sizeof(char*)*4);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "SUSCRIPTOR");

	mensaje[2] = string_new();
	string_append(&(mensaje[2]), msg);

	mensaje[3] = string_itoa(id_team);

	enviar_mensaje(mensaje, conexion);

	for(int i = 0; i < 4; i++) free(mensaje[i]);
	free(mensaje);

	pthread_t thread_suscriptor;
	while (1){ // Falta condicion
		pthread_create(&thread_suscriptor,NULL,(void*)recibir_mensaje,&conexion);
		pthread_join(thread_suscriptor, NULL);
	}

	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

/*
 * @NAME: suscribirse_a_colas
 * @DESC: Suscribe al proceso Team a las colas de mensajes del
 * 		  broker.
 */
void suscribirse_a_colas(){

	// REVISAR

	char* mensaje = string_new();
	string_append(&mensaje, "APPEARED_POKEMON");
	pthread_create(&hilo_appeared, NULL, suscribirse,(void*) mensaje);

	mensaje = string_new();
	string_append(&mensaje, "LOCALIZED_POKEMON");
	pthread_create(&hilo_localized, NULL, suscribirse,(void*) mensaje);

	mensaje = string_new();
	string_append(&mensaje, "CAUGHT_POKEMON");
	pthread_create(&hilo_caught, NULL, suscribirse,(void*) mensaje);

}

/*
 * @NAME: inicializar_vector_de_semaforos
 * @DESC: Dada una longitud devuelve un vector de semáforos inicializados en 0
 *
 */
sem_t* inicializar_vector_de_semaforos(u_int32_t longitud){

	sem_t* vector = malloc(sizeof(sem_t)*longitud);
	for(int i=0; i<longitud; i++){
		sem_init(&(vector[i]), 0, 0);
	}
	return vector;
}

void actualizar_objetivo_global(){
	t_list* auxiliar = get_objetivo_global(entrenadores);
	objetivo_global = list_flatten(auxiliar);
	list_destroy(auxiliar);
}



// Funcion main
int main (void) {

	//signal(SIGTERM, imprimir); // Mostrar
	//signal(SIGINT,liberar_todo); // Mostrar

	t_config* config = leer_config();

	// REVISAR SI ES NECESARIO QUE SEA GLOBAL
	config_team = construir_config_team(config);
	logger_team = iniciar_logger(config_team->log_file);

	printf("alpha: %f\n", config_team->alpha);

	cola_ready = queue_create();
	lista_ready = list_create(); // Lista para SJF sin desalojo
	appeared_pokemons = queue_create();
	entrenadores_deadlock = list_create();
	id_team = 0;

	sem_init(&sem_appeared_pokemon, 0, 0);
	sem_init(&puede_planificar, 0, 1);
	sem_init(&puede_intercambiar, 0, 1);
	puede_ejecutar = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));
	llega_mensaje_caught = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));

	termino_de_capturar = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));

	entrenadores = crear_entrenadores(config_team);

	sem_init(&sem_entrenadores, 0, list_size(entrenadores));

	suscribirse_a_colas(); // REVISAR

	actualizar_objetivo_global();

	especies_requeridas = obtener_especies(objetivo_global);

	enviar_mensajes_get_pokemon(); // REVISAR
	pthread_t hilo_servidor;
	pthread_create(&hilo_servidor, NULL, mantener_servidor, NULL);
	pthread_t hilo_planificador_largo_plazo;
	pthread_create(&hilo_planificador_largo_plazo, NULL, iniciar_planificador_largo_plazo, (void*) entrenadores);
	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador, NULL, iniciar_planificador, NULL);
	pthread_t hilo_intercambiador;
	pthread_create(&hilo_intercambiador, NULL, iniciar_intercambiador, NULL);

    //enreadyar_al_mas_cercano(entrenadores, appeared_pokemon, cola_ready);
	//planificar_entrenadores(cola_ready);

	pthread_join(hilo_planificador, NULL);
	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_appeared, NULL);
	pthread_join(hilo_caught, NULL);
	pthread_join(hilo_localized, NULL);


	liberar_estructuras(config_team, entrenadores, cola_ready, objetivo_global, especies_requeridas);

	terminar_programa(logger_team, config);

	//liberar_conexion(conexion);

	return 0;

}
