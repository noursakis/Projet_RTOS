//On commence par ajouter les bibliothéques nécessaires
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

//Partie Déclaration//
//déclaration de La structure
struct valeurCapteurs {
    int analogique;
    int numerique;
    double tempsEnMillisecondes;
};
//Déclaration

//Sémaphore
SemaphoreHandle_t xSerialSemaphore = NULL; // Déclarez unmutex Semaphore Handle que nous utiliserons pour gérer le port série, Il sera utilisé pour s'assurer qu'une seule tâche accède à cette ressource à tout moment
//Les Queues
QueueHandle_t qPT; //analog reading of the potentiometer
QueueHandle_t qBT; //digital sum of 2 push buttons
QueueHandle_t qStruct; //structure "valeurCapteurs" 
QueueHandle_t qStruct_new; //structure "valeurCapteurs" used by task 4 in order to send data

//Les Ports
int PT = A0; //Potentiométre sur l'entrée A0
int BT1 = 2; //Bouton poussoir1 
int BT2 = 3; //Boutton poussoir2


const TickType_t xDelay = 100 / portTICK_PERIOD_MS; //Retarder une tâche de 100ms



void setup() {
// put your setup code here, to run once:  
  Serial.begin(9600); // Définit le débit de données à 9600 bps
  while (!Serial) {
    ; // Attendre la connexion du port série for serial port to connect.
  }

  if ( xSerialSemaphore == NULL ) //condition1: Vérifiez  que la sémaphore en série n'a pas déjà été créé.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Crée un sémaphore mutex que nous utiliserons pour gérer le port série
    if ( ( xSerialSemaphore ) != NULL )//condition2: Vérifiez  que la sémaphore en série a déjà été créé.
      xSemaphoreGive( ( xSerialSemaphore ) );  // Rendre le port série disponible pour utilisation, en "donnant" le sémaphore.
  }
  
// Initialisation des ports

  pinMode(PT, INPUT);//définir le Potentimétre comme entrée
  pinMode(BT1, INPUT); //définir le BT1 comme entrée
  pinMode(BT2, INPUT);//définir le BT2 comme entrée
  
//Creation des taches

  xTaskCreate(Task1,"lecture_ANALOG",128,NULL,1,NULL);
  xTaskCreate(Task2,"lecture_BTN",128,NULL,1,NULL);
  xTaskCreate(Task3,"Structure",128,NULL,1,NULL);
  xTaskCreate(Task4,"Affiche la structure sur le port série",1000,NULL,1,NULL);
  xTaskCreate(Task5,"SetDataTime",1000,NULL,1,NULL);
  
//Creation et Initialisation des Queues
  
  qPT = xQueueCreate(5,sizeof(uint32_t));//Crée une queue capable de contenir 5 valeurs de type uint32_t
  qBT = xQueueCreate(5,sizeof(uint32_t));
  qStruct = xQueueCreate(5,sizeof(valeurCapteurs)); 
  qStruct_new = xQueueCreate(5,sizeof(valeurCapteurs));
  
}
void loop() {
  // no code is loop, all is done in tasks

}
//Premiére Tache
void Task1(void *pvParameters)
{
  int Pot_Data = 0;
  while(1)
  {
    Pot_Data= analogRead(PT);//Récupèrer une valeur analogique sur l’entrée A0 
    xQueueSend(qPT, &Pot_Data, 0);//Attribuer la valeur à la Queue qPT
    vTaskDelay( xDelay );//arreter la tache pendant 500ms
  }
}
//Deuxiéme Tache
void Task2(void *pvParameters)  
{
  int Bt_Data;
  while(1){
    Bt_Data = digitalRead(BT1)+digitalRead(BT2);//faire l'addition des deux valeurs des deux entrées numérique 3 et 4
    xQueueSend(qBT,&Bt_Data,0);//Attribuer la valeur à la Queue qBT
    vTaskDelay( xDelay  );// Delay the Task for 500ms before resume another cycle.
  }
  
}
//Troisiéme Tache
void Task3(void *pvParameters)
{
  int PTRead;
  int BTRead;
  valeurCapteurs s;
  while(1)
  {
    
    xQueueReceive(qPT,&PTRead,0);//reçoit les  valeurs de la tâche 1
    xQueueReceive(qBT,&BTRead,0);//reçoit les  valeurs de la tâche2

    s.analogique = PTRead;//affecter la valeur de la tâche 1 à la structure 
    s.numerique = BTRead;//affecter la valeur de la tâche2 à la structure 
    s.tempsEnMillisecondes = millis();
    xQueueSend(qStruct,&s,0);//Attribuer la valeur sensor à la Queue qStruct
    vTaskDelay( xDelay  );

  }
}

//Quatriéme Tache
void Task4(void *pvParameters)
{
  valeurCapteurs S;
  while(1)
  {
    xQueueReceive(qStruct,&S,0);//reçois la valeur dans la queue qStruct
    
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) //On peut accéder à la ressource partagée
    {
      //Afficher le contenu de la structure
      Serial.print("La valeur de la structure est: { ");
      Serial.print(S.analogique);
      Serial.print(" ; ");
      Serial.print(S.numerique);
      Serial.print(" ; ");
      Serial.print(S.tempsEnMillisecondes);
      Serial.println(" }");
      Serial.print("\n");
      xSemaphoreGive( xSerialSemaphore ); // Libérer le port série
    }
    xQueueSend(qStruct_new,&S,0); //Attribuer la valeur sensor à la Queue qStruct_new
    vTaskDelay( xDelay  );
  }
}

//cinquiéme Tache

void Task5(void *pvParameters)
{
  valeurCapteurs S;
  valeurCapteurs S2;
  while(1)
  {
    xQueueReceive(qStruct_new,&S,0); //reçoit les  valeurs de la tâche 4
    S2.analogique = S.analogique;//garder la meme valeur analogique 
    S2.numerique = S.numerique;//garder la meme valeur numérique 
    S2.tempsEnMillisecondes = S.tempsEnMillisecondes/60000; //conversion de la valeur du temps dans la structure en minutes
    
    
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE ) // Vérifier si on peut accéder au port série
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      //Afficher le contenu de la nouvelle structure
      Serial.print("La valeur de la nouvelle structure est: [ ");
      Serial.print(S2.analogique);
      Serial.print(" ; ");
      Serial.print(S2.numerique);
      Serial.print(" ; ");
      Serial.print(S2.tempsEnMillisecondes);
      Serial.println(" ]");

      xSemaphoreGive( xSerialSemaphore );//Libérer la sémaphore
    }

    vTaskDelay( xDelay  );
  }
}
