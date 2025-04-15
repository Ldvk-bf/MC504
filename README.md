# **Simulação de Sincronização de Montanha Russa 🎢**

Um programa C multithread que simula o embarque e desembarque de passageiros em uma montanha-russa, demonstrando conceitos de sincronização usando pthreads e semáforos.

## **Tabela de Conteúdos**

* Conceito  
* Mecanismos de Sincronização  
* Características  
* Execução  
* Configuração  
* Estrutura do código  
* Boas Práticas

## **Conceito**

Este programa implementa:

* Uma montanha-russa com capacidade fixa  
* Vários passageiros tentando embarcar  
* Requisitos de sincronização:  
  * A montanha-russa só sai quando está cheia  
  * Os passageiros só podem embarcar quando a montanha-russa estiver disponível  
  * Os passageiros só podem desembarcar após a conclusão da viagem  
  * Não é permitido embarcar durante o passeio ou desembarcar

## **Mecanismos de Sincronização**

Esta implementação usa:

* **Mutex** (`pthread_mutex_t`):  
  * Protege o balcão de embarque durante o embarque/desembarque  
* **Semaphores**:  
  * `board_queue`: Controla o embarque de passageiros (inicialmente \= capacidade)  
  * `unboard_queue`: Controla o desembarque de passageiros (inicialmente 0\)  
  * `all_aboard`: Sinal quando a montanha-russa está cheia  
  * `all_ashore`: Sinal quando todos os passageiros saíram

## **Características**

* Representação visual   
* Atrasos aleatórios simulam a imprevisibilidade do mundo real  
* Gestão de recursos limpos  
* Parâmetros configuráveis:  
  * Capacidade da montanha-russa  
  * Número de passageiros  
  * Duração do passeio

## **Execução**

```
bash

# Compilar
gcc roller_coaster.c -o roller_coaster -lpthread

# Run 
./roller_coaster
```

## **Configuração**

Modifique os  `#define`s  no código-fonte:

```c

#define CAPACITY 4   // Numero de assentos na montanha russa
#define NUM_PASSENGERS 12   // Total passageiros
```

## **Estrutura do código** 

Funções chaves:

* `init_roller_coaster()`: Inicializa primitivas de sincronização  
* `passenger()`: Rotina de thread de passageiro  
* `roller_coaster()`: control thread da montanha-russa  
* `board()`/`unboard()`: Atomic embarque/desembarque operações

## **Boas Práticas**

* Dados compartilhados protegidos por mutex  
* Uso adequado do semáforo para sinalização  
* Limpeza de recursos na saída  
* Feedback visual para demonstração

---

