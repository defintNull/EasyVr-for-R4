/*
  Module for EasyVR 1 with EasyVR Shield rev.1d for Arduino uno R4 wifi
*/


#include "Arduino_LED_Matrix.h"
#include "EasyVR.h"
#include "WiFiS3.h"
#include "arduino_secrets.h"

#define easyvrSerial Serial1
EasyVR easyvr(easyvrSerial);


ArduinoLEDMatrix matrix;

/*
  Manage the request for the bridge mode
*/
void bridge_mode() {
  int mode = easyvr.bridgeRequested(Serial);

  switch (mode) {
    case EasyVR::BRIDGE_NONE:
      // setup EasyVR serial port
      easyvrSerial.begin(9600);
      // run normally
      Serial.println(F("Bridge not requested, run normally"));
      Serial.println(F("---"));
      break;
      
    case EasyVR::BRIDGE_NORMAL:
      // setup EasyVR serial port (low speed)
      easyvrSerial.begin(9600);
      // soft-connect the two serial ports (PC and EasyVR)
      easyvr.bridgeLoop(Serial);
      // resume normally if aborted
      Serial.println(F("Bridge connection aborted"));
      Serial.println(F("---"));
      break;
      
    case EasyVR::BRIDGE_BOOT:
      // setup EasyVR serial port (high speed)
      easyvrSerial.begin(115200);
      Serial.end();
      Serial.begin(115200);
      // soft-connect the two serial ports (PC and EasyVR)
      easyvr.bridgeLoop(Serial);
      // resume normally if aborted
      Serial.println(F("Bridge connection aborted"));
      Serial.println(F("---"));
      break;
  }
}

/*
  Check the status of the grammars of the board
*/
void grammarCheck() {
  // Check for grammars
  int8_t grammars = 0;
  int8_t set = 0;
  char name[33];
  Serial.println(F("--------------------"));
  Serial.print(F("Custom Grammars: "));
  grammars = easyvr.getGrammarsCount();
  if (grammars > 4)
  {
    Serial.println(grammars - 4);
    for (set = 4; set < grammars; ++set)
    {
      Serial.print(F("Grammar "));
      Serial.print(set);

      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
      {
        Serial.print(F(" has "));
        Serial.print(num);
        if (flags & EasyVR::GF_TRIGGER)
          Serial.println(F(" trigger"));
        else
          Serial.println(F(" command(s)"));
      }
      else
      {
        Serial.println(F(" error"));
        continue;
      }

      for (int8_t idx = 0; idx < num; ++idx)
      {
        Serial.print(idx);
        Serial.print(F(" = "));
        if (!easyvr.getNextWordLabel(name))
          break;
        Serial.println(name);
      }
    }
  }
  else {
    Serial.println(F("n/a"));
  }
  Serial.println(F("--------------------"));
}

/*
  Check the status of the sound table
*/
void soundTableCheck() {
  // Check sound table
  int16_t count = 0;
  char name[33];
  Serial.println(F("--------------------"));
  Serial.print(F("Sound table: "));
  if (easyvr.dumpSoundTable(name, count))
  {
    Serial.println(name);
    Serial.print(F("Sound entries: "));
    Serial.println(count);
  }
  else {
    Serial.println(F("n/a"));
  }
  Serial.println(F("--------------------"));
}

/*
  Check the status of the custom command groups
*/
void customGroupMaskCheck() {
  uint8_t train = 0;
  uint32_t mask = 0;
  int8_t group = 0;
  char name[33];
  int16_t count = 0;
  Serial.println(F("--------------------"));
  Serial.println(F("Checking command Groups"));
  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        Serial.print(F("Trigger: "));
      else if (group == EasyVR::PASSWORD)
        Serial.print(F("Password: "));
      else
      {
        Serial.print(F("Group "));
        Serial.print(group);
        Serial.print(F(" has "));
      }
      count = easyvr.getCommandCount(group);
      Serial.print(count);
      if (group == 0)
        Serial.println(F(" trigger(s)"));
      else
        Serial.println(F(" command(s)"));
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          Serial.print(idx);
          Serial.print(F(" = "));
          Serial.print(name);
          Serial.print(F(", Trained "));
          Serial.print(train, DEC);
          if (!easyvr.isConflict())
            Serial.println(F(" times, OK"));
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              Serial.print(F(" times, Similar to Word "));
            else
            {
              confl = easyvr.getCommand();
              Serial.print(F(" times, Similar to Command "));
            }
            Serial.println(confl);
          }
        }
      }
    }
  } else {
    Serial.println(F("No command groups"));
  }
  Serial.println(F("--------------------"));
}

/*
  Check the status of the board
*/
void statusCheck() {
  grammarCheck();

  soundTableCheck();

  customGroupMaskCheck();
}

/*
  Manage the initialization of the board
*/
void initialization() {
  // Sarting EasyVR initialization
  Serial.println(F("--------------------"));
  Serial.println(F("Initializing EasyVR..."));
  bridge_mode();

  // Checking if the device is present
  while (!easyvr.detect()) {
    Serial.println(F("EasyVR NOT found!"));

    // Checking if going in bridge mode
    for (int i = 0; i < 10; ++i) {
      if (Serial.read() == EasyVR::BRIDGE_ESCAPE_CHAR) {
        bridge_mode();
      }
      delay(100);
    }
  }

  // Playing a BEEP
  easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);

  // Detecting version
  Serial.print(F("EasyVR detected, version "));
  Serial.print(easyvr.getID());

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // Shield 2.0 LED off

  if (easyvr.getID() < EasyVR::EASYVR)
    Serial.print(F(" = VRbot module"));
  else if (easyvr.getID() < EasyVR::EASYVR2)
    Serial.print(F(" = EasyVR module"));
  else if (easyvr.getID() < EasyVR::EASYVR3)
    Serial.print(F(" = EasyVR 2 module"));
  else
    Serial.print(F(" = EasyVR 3 module"));
  Serial.print(F(", FW Rev."));
  Serial.println(easyvr.getID() & 7);

  easyvr.setDelay(0);

  // Setting timeout for recognition
  easyvr.setTimeout(5);
  // Setting Language
  easyvr.setLanguage(EasyVR::ITALIAN);

  // use fast recognition
  easyvr.setTrailingSilence(EasyVR::TRAILING_MIN);
  easyvr.setCommandLatency(EasyVR::MODE_FAST);

  // Setting microphone distance
  easyvr.setMicDistance(EasyVR::FAR_MIC);

  statusCheck();
}

/*
  Recognize a word from a given word set.
  If it fails print the error

  Return the word index or -1 if error
*/
int8_t recognizeWord(int8_t wordset, EasyVR::Distance distance = EasyVR::FAR_MIC) {
  Serial.println();
  Serial.println("--------------------");
  Serial.print(F("Recognizing trigger from trigger group"));
  easyvr.setPinOutput(EasyVR::IO1, HIGH);
  easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
  easyvr.setMicDistance(distance);
  easyvr.recognizeCommand(EasyVR::TRIGGER_SET);
  while (!easyvr.hasFinished()) {
    delay(50);
  }
  easyvr.setPinOutput(EasyVR::IO1, LOW);
  int8_t word = easyvr.getCommand();
  if(word >= 0) {
    Serial.print(F("Trigger: "));
    Serial.println(word);
  } else {
    Serial.print(F("Error: "));
    Serial.println(easyvr.getError());
    return word;
  }

  Serial.println("--------------------");
  Serial.print(F("Recognizing word from wordset "));
  Serial.println(wordset);

  easyvr.setPinOutput(EasyVR::IO1, HIGH);
  easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
  easyvr.recognizeWord(wordset);
  while (!easyvr.hasFinished()) {
    delay(50);
  }
  easyvr.setPinOutput(EasyVR::IO1, LOW);
  word = easyvr.getWord();
  if(word >= 0) {
    Serial.print(F("Word: "));
    Serial.println(word);
  } else {
    Serial.print(F("Error: "));
    Serial.println(easyvr.getError());
  }
  Serial.println(F("--------------------"));
  return word;
}

/*
  Recognize a command from a given group set.
  If it fails print the error

  Return the command name or nullptr if error
*/
const char* recognizeCommand(EasyVR::Distance distance = EasyVR::FAR_MIC) {
  Serial.println();
  Serial.println("--------------------");
  Serial.print(F("Recognizing command from group 1"));

  easyvr.setPinOutput(EasyVR::IO1, HIGH);
  easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
  easyvr.recognizeCommand(1);
  while (!easyvr.hasFinished()) {
    delay(50);
  }
  easyvr.setPinOutput(EasyVR::IO1, LOW);
  int8_t command = easyvr.getCommand();
  if(command >= 0) {
    Serial.print(F("Command: "));
    Serial.println(command);

    // Buffer locale per il nome
    static char name[32];
    uint8_t training = 0;

    if (easyvr.dumpCommand(1, command, name, training)) {
      Serial.print(F("Command Name: "));
      Serial.println(name);
      Serial.println(F("--------------------"));
      return name;
    } else {
      Serial.println(F("Errore nel dump del comando"));
      Serial.println(F("--------------------"));
      return nullptr;
    }
  } else {
    Serial.print(F("Error: "));
    Serial.println(easyvr.getError());
    Serial.println(F("--------------------"));
    return nullptr;
  }
}

/*
  Create a command and make the user train it
  Return 0 if ok, -1 if something went wrong and print the error
*/
int8_t createCommand(char* name, int8_t group = 1, EasyVR::Distance distance = EasyVR::ARMS_LENGTH) {
  Serial.println();
  Serial.println(F("--------------------"));
  Serial.print(F("Creating command in group "));
  Serial.println(group);
  int8_t command_count = easyvr.getCommandCount(group);
  if(command_count < 32) {
    easyvr.addCommand(group, command_count);
    easyvr.setCommandLabel(group, command_count, name);
    Serial.print(F("Created command "));
    Serial.print(name);
    Serial.print(F(" in group "));
    Serial.print(group);
    Serial.print(F(" with index "));
    Serial.println(command_count);
    Serial.println(F("--------------------"));
    // Training phase
    Serial.println(F("Starting training phase of the command"));
    for(int8_t i=0; i<3; i++) {
      Serial.print(F("Training "));
      Serial.println(i);
      easyvr.setPinOutput(EasyVR::IO1, HIGH);
      easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
      easyvr.setMicDistance(distance);
      easyvr.trainCommand(group, command_count);
      while(!easyvr.hasFinished()) {
        delay(50);
      }
      easyvr.setPinOutput(EasyVR::IO1, LOW);
      Serial.println(F("OK"));
      delay(1000);
    }
    return 0;
  }
  Serial.println(F("Too many command in this group"));
  Serial.println(F("--------------------"));
  return -1;
}

/*
  Retrain a given command
*/
void reTrainCommand(int8_t index, int8_t group = 1, EasyVR::Distance distance = EasyVR::ARMS_LENGTH) {
  Serial.println();
  Serial.println(F("--------------------"));
  Serial.print(F("Re-Training command "));
  Serial.print(index);
  Serial.print(F(" of group "));
  Serial.println(group);
  easyvr.eraseCommand(group, index);
  Serial.println(F("--------------------"));
  // Training phase
  Serial.println(F("Starting training phase of the command"));
  for(int8_t i=0; i<3; i++) {
    Serial.print(F("Training "));
    Serial.println(i);
    easyvr.setPinOutput(EasyVR::IO1, HIGH);
    easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
    easyvr.setMicDistance(distance);
    easyvr.trainCommand(group, index);
    while(!easyvr.hasFinished()) {
      delay(50);
    }
    easyvr.setPinOutput(EasyVR::IO1, LOW);
    Serial.println(F("OK"));
    delay(1000);
  }
}

/*
  Add training to a given command
*/
void TrainCommand(int8_t index, int8_t group = 1, EasyVR::Distance distance = EasyVR::ARMS_LENGTH) {
  Serial.println();
  Serial.println(F("--------------------"));
  Serial.print(F("Re-Training command "));
  Serial.print(index);
  Serial.print(F(" of group "));
  Serial.println(group);
  Serial.println(F("--------------------"));

  // Training phase
  Serial.println(F("Starting training phase of the command"));
  for(int8_t i=0; i<3; i++) {
    Serial.print(F("Training "));
    Serial.println(i);
    easyvr.setPinOutput(EasyVR::IO1, HIGH);
    easyvr.playSound(EasyVR::BEEP, EasyVR::VOL_FULL);
    easyvr.setMicDistance(distance);
    easyvr.trainCommand(group, index);
    while(!easyvr.hasFinished()) {
      delay(50);
    }
    easyvr.setPinOutput(EasyVR::IO1, LOW);
    Serial.println(F("OK"));
    delay(1000);
  }
}

/*
  Delete a given command
*/
void deleteCommand(int8_t index, int8_t group = 1) {
  Serial.println();
  Serial.println(F("--------------------"));
  Serial.print(F("Re-Training command "));
  Serial.print(index);
  Serial.print(F(" of group "));
  Serial.println(group);
  easyvr.removeCommand(group, index);
  Serial.println(F("Command deleted!"));
  Serial.println(F("--------------------"));
}

/*
  Main loop sleep function
*/
void operate() {
  Serial.println();
  Serial.println(F("--------------------"));
  Serial.println(F("Entering in sleep mode..."));

  easyvr.sleep(EasyVR::WAKE_ON_LOUDSOUND);

  // Loop fino al wake
  while (true) {
    if(!easyvr.hasFinished()) {
      delay(1);
      continue;
    }
    if(easyvr.isAwakened()) {
      break;
    }
    delay(1);
  }

  Serial.println(F("Woke up!"));
  delay(1000);

  const char* command = recognizeCommand();

  if(command != nullptr) {
    clientRequest(command);
  }

  Serial.println(F("--------------------"));
}


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(192,168,10,3);  // numeric IP for Google (no DNS)
IPAddress server(10,172,139,188);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

/* -------------------------------------------------------------------------- */
void printWifiStatus() {
/* -------------------------------------------------------------------------- */  
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void initializeWifi(){
  //Initialize serial and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    Serial.println(WiFi.status());
     
    // wait 10 seconds for connection:
    delay(10000);
  }
  
  printWifiStatus();

  Serial.println("\nStarting connection to server...");

  if (client.connect(server, 1234)) {
    Serial.println("connected to server");
  } else {
    Serial.println("Not connected to server");
  }
}

void clientRequest(const char* command) {

  Serial.println(F("Richiesta"));

  if (!client.connect(server, 1234)) {
    Serial.println(F("Connessione fallita"));
    return;
  }

  client.print("GET /command?id=");
  client.print(command);
  client.println(" HTTP/1.1");
  client.println("Host: 10.128.44.167:1234");
  client.println("Connection: close");
  client.println();

  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }

  client.stop();
  Serial.println(F("\nConnessione chiusa"));
}



void setup() {
  Serial.begin(9600);
  initializeWifi();
  
  delay(5000);
  matrix.begin();

  initialization();
}




const uint32_t felice[] = {
    0x19819,
    0x80000001,
    0x81f8000
};
const uint32_t cuore[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};

void loop() {
  matrix.loadFrame(felice);
  delay(1000);
 
  matrix.loadFrame(cuore);
  delay(1000);

  operate();

  //delay(2000000000000);
}
