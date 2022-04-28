# TP projet RTOS 
Nous sommes ammenée à écrire un programme sur Arduino en utilisant FreeRTOS pour accomplir 5 tâches.
Le but du programme c'est d'envoyer des données d'une tache à une autre via les Queues tout en assurant la sécurité du port série étant une ressource partagé entre taches par le biais d'une sémaphore


1- On commennce par ajouter la librairie FreeRTOS pour arduino ainsi que celle des sémaphores pour assurer la protection des ressources partagés  
```bash
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
```
2- On Initliasie la sémaphore , ainsi que les Queues qPT , qBT, qStruct et qStruct_new
```bash
SemaphoreHandle_t xSerialSemaphore = NULL;
QueueHandle_t qPT; //lire la valeur du potentiométre
QueueHandle_t qBT; //digital sum of 2 push buttons
QueueHandle_t qStruct; //structure "valeurCapteurs" 
QueueHandle_t qStruct_new; //structure "valeurCapteurs" used by task 4 in order to send data
```

3- On Commence Par la 1 ere tache : Récupère une valeur analogique sur l’entrée A0 qui est branchée avec un potentiomètre puis l’envoie à la tâche 3 via la Queue qPT.
```bash
  xTaskCreate(Task1,"lecture_ANALOG",128,NULL,1,NULL);
  Pot_Data= analogRead(PT);
  xQueueSend(qPT,&Pot_Data,0);
```
4-Création de la 2éme tache qui Récupère une valeur numérique qui est la résultante de l'addition des deux valeurs des deux entrées numérique 3 et 4 qui sont branchées avec des boutons poussoirs en montage pull down, puis envoie cette valeur numérique à la tâche 3 via la Queue qBT
```bash
xTaskCreate(Task2,"lecture_BTN",128,NULL,1,NULL);
Bt_Data = digitalRead(BT1)+digitalRead(BT2);
xQueueSend(qBT,&Bt_Data,0)
```
5-Création de la 3éme tache qui reçoit les deux valeurs des tâches 1 et 2 puis les mets dans une structure en ajoutant la valeur de la fonction millis() via la Queue qStruct
```bash
xQueueReceive(qPT,&PTRead,0);
xQueueReceive(qBT,&BTRead,0);
s.analogique = PTRead;
s.numerique = BTRead;
s.tempsEnMillisecondes = millis();
xQueueSend(qStruct,&s,0)
```
6-Création de la 4éme tache qui reçoit la valeur de la structure et utilise le port série pour l’afficher s'il est libre, ensuite envoie cette structure à la tâche 5 via la Queue qStruct_new
```bash
xQueueReceive(qStruct,&S,0);
Serial.print(S.analogique);
Serial.print(S.numerique);
Serial.print(S.tempsEnMillisecondes);
xQueueSend(qStruct_new,&S,0);
```
6- Création de la 5éme tache qui transforme la valeur du temps dans la structure en minutes, ensuite elle affiche cette nouvelle structure à travers le port série (s'il est disponible)
```bash
xQueueReceive(qStruct_new,&S,0);
S2.analogique = S.analogique;
S2.numerique = S.numerique;
S2.tempsEnMillisecondes = S.tempsEnMillisecondes/60000;
```




