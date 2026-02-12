# EasyVr-for-R4

Codice dedicato allimplementazione di un sistema di riconoscimento vocale comandi per un iRobot Create 3 con Arduino uno Q.

## Scopo
Il repository contiene il codice per un R4 WiFi e un EasyVr 1 utilizzato con il suo shield.  
Implementa tutti i comportamenti necessari con appositi log per la registrazione, cancellazione e allenamento di comandi vocali che vengono inviati mediante modulo wifi ad un Arduino Uno Q collegato con un iRobot Create 3 per la gestione del comportamento di robot e algoritmo attraverso appositi nodi ROS2

## Struttura
- `EasyVr.ino` — codice per l'Arduino Uno R4.
- `arduino_secrets.h.template` — templete per secrets necessari per connessione wifi.

## Uso
0. Assicurarsi di aver installato l'Arduino IDE e le librerie necessarie per l'R4 WiFi e la libreria EasyVr
   
1. A causa dell'impossibilità di utilizzare SoftwareSerial su Arduino Uno R4 si è dovuto utilizzare la Serial1, ovvero la UART sui pin D0 e D1 che richiede, prima di eseguire il codice, di impostare il selettore sullo shield dell'EasyVr su HW.
   
2. Dopo vari tentativi nell'utilizzo della funzione sleep e wakeup del modulo EasyVr è stato necessario andare ad effettuare una modifica alla libreria EasyVr per correggere comportamento che dopo primo wakeup non veniva più settata la flag dello sleep.  
Alla riga 180 di EasyVr.cpp aggiungere prima del return
    ```bash
    _status.b._awakened = false;  
    ```

3. Attualmente il codice è configurato per il comportamento di default durante operatività standard del modulo, entrando in stato di sleep e attendendo comandi da inviare.  
Commentando la funzione di inizializzazione del wifi e l'invio della richiesta http è possibile escludere la funzione wifi e, all'interno del loop, è possibile cambiare funzioni a seconda del comportamento che si vuole ottenere dal modulo, andando a scegliere la funzione desiderata.

4. Prima di usare le funzioni wifi copiare il file arduino_secrets.h.template nel file arduino_secrets.h e inserire nome e password della rete alla quale ci si desidera collegare.  
Successivamente all'interno del codice andare a cambiare l'ip con quello dell'Arduino Uno Q o del server al quale si vogliono inviare i comandi.
   
5. Finite le varie configurazioni è possibile flashare il codice all'interno dell'Arduino Uno R4.