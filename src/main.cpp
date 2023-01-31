/*
 * FreeRTOS Esempio 09: Mutex Race Condition demo
 *
 * Incremento concorrente di una variabile da due task
 *
 * Nota: nel file soc.h sono definiti i riferimenti ai due core della ESP32:
 *   #define PRO_CPU_NUM (0)
 *   #define APP_CPU_NUM (1)
 *
 * Qui viene adoperata la APP_CPU
 *
 */

// togliere il commento al #define sottostante
// per introdurre il controllo della sezione critica
// con mutex
#define CON_MUTEX

#include <Arduino.h>

// variabile globale condivisa
static int shared_var = 0;

#ifdef CON_MUTEX
// mutex per gestire l'accesso alla variabile globale
static SemaphoreHandle_t mutex;
#endif

//**********************************************************************
// Tasks

// Incrementa la variabile globale... nel modo corretto
// proteggendo la sezione critica tramite il mutex
void incTask(void *parameters)
{
  int local_var;

  // loop infinito
  while (1)
  {
#ifdef CON_MUTEX
    // acquisisce il mutex prima di entrare nella sezione critica
    if (xSemaphoreTake(mutex, 0) == pdTRUE)
    {
#endif
      // sezione critica
      // "approssimo" un accesso casuale alla variabile globale
      local_var = shared_var;
      local_var++;
      // introduco un ritardo casuale
      vTaskDelay(random(100, 500) / portTICK_PERIOD_MS);
      shared_var = local_var;

      // fine della sezione critica

#ifdef CON_MUTEX
      // restituisce il mutex
      xSemaphoreGive(mutex);
#endif
      // stampa il nuovo valore della variabile condivisa
      Serial.println(shared_var);

#ifdef CON_MUTEX
    }
    else
    {
      // eventuale gestione della collisione sul mutex
    }
#endif
  }
}

//**********************************************************************
// Main

// configurazione del sistema
void setup()
{
  // un po' di casualità sul generatore RND:
  randomSeed(analogRead(33));

  // Configurazione della seriale
  Serial.begin(115200);

  // breve pausa
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("FreeRTOS: race condition demo");

#ifdef CON_MUTEX
  // creazione del mutex prima di avviare i task
  mutex = xSemaphoreCreateMutex();
#endif

  // creazione e avvio del Task1
  xTaskCreatePinnedToCore(
      incTask,    // funzione da richiamare nel task
      "Inc Task", // nome del task (etichetta utile per debug)
      1500,       // dimensione in byte dello stack per le variabili locali del task (minimo 768 byte)
      NULL,       // puntatore agli eventuali parametri da passare al task
      1,          // priorità del task
      NULL,       // eventuale task handle per gestire il task da un altro task
      APP_CPU_NUM // core su cui far girare il task
  );

  // creazione e avvio del Task2
  xTaskCreatePinnedToCore(
      incTask,    // funzione da richiamare nel task
      "Inc Task", // nome del task (etichetta utile per debug)
      1500,       // dimensione in byte dello stack per le variabili locali del task (minimo 768 byte)
      NULL,       // puntatore agli eventuali parametri da passare al task
      1,          // priorità del task
      NULL,       // eventuale task handle per gestire il task da un altro task
      APP_CPU_NUM // core su cui far girare il task
  );

  // elimina il task in cui girano setup() e loop()
  vTaskDelete(NULL);
}

void loop()
{
  // put your main code here, to run repeatedly:
}