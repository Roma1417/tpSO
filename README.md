# CoronaLinux 


## TO DO [Team]

- [X] Arreglar problema +1
- [ ] Hacer funcion find_first (si no encuentra devuelve el primero)
- [X] Universalizar funcion distancia
- [ ] Implementacion Deadlock (IN_PROGRESS)
- [ ] Implementacion SJF sin desalojo
- [ ] Implementacion SJF con desalojo
- [ ] Implementacion RR


### . DEADLOCK  

- [X] Chequear que la cantidad de pokemons disponibles a atrapar sea 0
- [X] Hacer lista de entrenadores con objetivo cumplido
- [X] Hacer lista de entrenadores con objetivo no cumplido
- [X] Hacer lista de los pokemons que necesita cada entrenador (After deteccion deadlock)
- [X] Hacer lista de los pokemons que no le sirven al entrenador (After deteccion deadlock)
- [ ] Crear estructura entrenador_deadlock (global)
- [ ] Inicio while (hasta que todos los entrenadores tengan los pokemons que necesitan)
- [ ] Elegir 1 entrenador e ir comparando con los pokemons_inservibles del resto
- [ ] Mandar a planificar al entrenador elegido a la posicion del entrenador con el que va a intercambiar
- [ ] Una vez en posicion, chequear si necesita alguno de los que a el no le sirven
- [ ] Realizar intercambio
- [ ] Actualizar el estado del entrenador (si ya cumplio su objetivo propio o no (si sigue en deadlock o no))

## TO DO [GameCard]

- [ ] Implementación del procedimiento al recibir mensaje NEW_POKEMON (IN_PROGRESS)
- [ ] Implementación del procedimiento al recibir mensaje CATCH_POKEMON
- [ ] Implementación del procedimiento al recibir mensaje GET_POKEMON

### . NEW_POKEMON  

- [X] Verificar existencia de archivo del pokemon. En caso contrario crearlo con su metadata.bin
- [ ] Verificar si se puede abrir el archivo
- [ ] Verificar si las posiciones ya existen en el archivo y actualizarlas. En caso contrario agregarlas en una nueva línea
- [ ] Esperar la cantidad de segundos definidas en el .config
- [ ] Enviar mensaje APPEARED_POKEMON al Broker

