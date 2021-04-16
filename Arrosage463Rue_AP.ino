/*********
 * Arrosage au --- 
 * Coté rue
 * 
 * 6 zones d'arrosage
 * 
 * Guy Gauthier - 2021
 * 
 * Code du projet inspire de celui de :
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

// Import required libraries
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "RTClib.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* ssid     = "yourSSID";
const char* password = "yourPassword";

const char* PARAM_INPUT = "value";
String heureDepart = "20:00";
String heureValue = "2021-01-31T13:30";
String duree1 = "10";
String duree2 = "10";
String duree3 = "10";
String duree4 = "10";
String duree5 = "10";
String duree6 = "10";
String duree9 = "10";
String dureeA = "10";
String dureeB = "10";

const char* dur1Path = "/duree1.txt";
const char* dur2Path = "/duree2.txt";
const char* dur3Path = "/duree3.txt";
const char* dur4Path = "/duree4.txt";
const char* dur5Path = "/duree5.txt";
const char* dur6Path = "/duree6.txt";
const char* dur9Path = "/duree9.txt";
const char* durAPath = "/dureeA.txt";
const char* durBPath = "/dureeB.txt";
const char* startPath = "/depart.txt";

int moments[9] = {0};
int moments2[4] = {0};
int durees[6] = {10,10,10,10,10,10};
int durees2[3] = {10,10,10};
int etat[8] = {0};
String zoneState1 = "OFF";
String zoneState2 = "OFF";
String zoneState3 = "OFF";
String zoneState4 = "OFF";
String zoneState5 = "OFF";
String zoneState6 = "OFF";
const int sortie[6] = {12,33,27,32,14,15};
const int modeLed = LED_BUILTIN;
int enCours = -1;
int modeArr = 1;  // mode automatique par defaut
String modeFnct = "AUTOMATIQUE";

int annee, mois, jour, heure, minu, minuActu, minuPast, k;
int tentatives = 0;
DateTime maintenant;

RTC_DS3231 rtc;

void initSPIFFS(){
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Lecture du fichier: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- ne peut ouvrir le fichier en lecture");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Ecriture dans le fichier: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- ne peut ouvrir le fichier en ecriture");
    return;
  }
  if(file.print(message)){
    Serial.println("- ecriture reussie");
  } else {
    Serial.println("- ecriture ratee");
  }
}

void initWiFi(){
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

// Replaces placeholder with button section in your web page
String processorH(const String& var){
  Serial.println(var);
  if (var == "HEURE"){
    DateTime now = rtc.now();
    heureValue = String(now.year())+"-";
    if (now.month()<10)
      heureValue = heureValue+"0";
    heureValue = heureValue+String(now.month())+"-";
    if (now.day()<10)
      heureValue = heureValue+"0";
    heureValue = heureValue+String(now.day())+"T";
    if (now.hour()<10)
      heureValue = heureValue+"0";
    heureValue = heureValue+String(now.hour())+":";
    if (now.minute()<10)
      heureValue = heureValue+"0";
    heureValue = heureValue+String(now.minute());
      
    //Serial.println(heureValue);
    return heureValue;
  }
  if (var == "TEMP"){
    return String(rtc.getTemperature());
  }
  if (var == "DEPART"){
    return heureDepart;
  }
  if (var == "DUR1"){
    return duree1;
  }
  if (var == "DUR2"){
    return duree2;
  }
  if (var == "DUR3"){
    return duree3;
  }
  if (var == "DUR4"){
    return duree4;
  }
  if (var == "DUR5"){
    return duree5;
  }
  if (var == "DUR6"){
    return duree6;
  }
  if (var == "DUR9"){
    return duree9;
  }
  if (var == "DURA"){
    return dureeA;
  }
  if (var == "DURB"){
    return dureeB;
  }
  if (var == "ETAT1"){
    if (etat[0]==0) zoneState1 = "OFF";
    else zoneState1 = "ON";
    
    return zoneState1;
  }
  if (var == "ETAT2"){
    if (etat[1]==0) zoneState2 = "OFF";
    else zoneState2 = "ON";
    
    return zoneState2;
  }
  if (var == "ETAT3"){
    if (etat[2]==0) zoneState3 = "OFF";
    else zoneState3 = "ON";
    
    return zoneState3;
  }
  if (var == "ETAT4"){
    if (etat[3]==0) zoneState4 = "OFF";
    else zoneState4 = "ON";
    
    return zoneState4;
  }
  if (var == "ETAT5"){
    if (etat[4]==0) zoneState5 = "OFF";
    else zoneState5 = "ON";
    
    return zoneState5;
  }
  if (var == "ETAT6"){
    if (etat[5]==0) zoneState6 = "OFF";
    else zoneState6 = "ON";
    
    return zoneState6;
  }
  if (var == "MODE"){
    if (modeArr == 1) modeFnct = "AUTOMATIQUE";
    else if (modeArr == 0) modeFnct = "MANUEL";
    else modeFnct = "ANNULATION";

    return modeFnct;
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  for(k=0;k<6;k++){
    pinMode(sortie[k],OUTPUT);
    digitalWrite(sortie[k],LOW);
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  
  initWiFi();
  initSPIFFS();

  duree1 = readFile(SPIFFS, dur1Path);
  durees[0] = duree1.toInt();
  duree2 = readFile(SPIFFS, dur2Path);
  durees[1] = duree2.toInt();
  duree3 = readFile(SPIFFS, dur3Path);
  durees[2] = duree3.toInt();
  duree4 = readFile(SPIFFS, dur4Path);
  durees[3] = duree4.toInt();
  duree5 = readFile(SPIFFS, dur5Path);
  durees[4] = duree5.toInt();
  duree6 = readFile(SPIFFS, dur6Path);
  durees[5] = duree6.toInt();
  duree9 = readFile(SPIFFS, dur9Path);
  durees2[0] = duree9.toInt();
  dureeA = readFile(SPIFFS, durAPath);
  durees2[1] = dureeA.toInt();
  dureeB = readFile(SPIFFS, durBPath);
  durees2[2] = dureeB.toInt();
  heureDepart = readFile(SPIFFS, startPath);

  moments[0] = 60*heureDepart.substring(0,2).toInt()+heureDepart.substring(3,5).toInt();
  for(k=1;k<=6;k++)
    moments[k] = moments[k-1]+durees[k-1];

  moments2[0] = 60*heureDepart.substring(0,2).toInt()+heureDepart.substring(3,5).toInt();
  for(k=1;k<=3;k++)
    moments2[k] = moments2[k-1]+durees2[k-1];
  
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processorH);
  });

  // Route for root / web page
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processorH);
  });

  // Route for root / web page
  server.on("/manuel.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/manuel.html", String(), false, processorH);
  });

  // Route for root / web page
  server.on("/automatique.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/automatique.html", String(), false, processorH);
  });
  
  // Route for root / web page
  server.on("/heure.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/heure.html", String(), false, processorH);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

// Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/heure", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      heureValue = inputMessage;
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.print("Nouvelle heure:");
    Serial.println(inputMessage);
    annee = inputMessage.substring(0,4).toInt();
    //Serial.println(annee);
    mois = inputMessage.substring(5,7).toInt();
    //Serial.println(mois);
    jour = inputMessage.substring(8,10).toInt();
    //Serial.println(jour);
    heure = inputMessage.substring(11,13).toInt();
    //Serial.println(heure);
    minu = inputMessage.substring(14).toInt();
    //Serial.println(minu);
    rtc.adjust(DateTime(annee, mois, jour, heure, minu, 0));
    //maintenant = rtc.now();
    //Serial.println(maintenant.dayOfTheWeek());
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/depart", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      heureDepart = inputMessage;
      writeFile(SPIFFS, startPath, heureDepart.c_str());
      moments[0] = 60*inputMessage.substring(0,2).toInt()+inputMessage.substring(3,5).toInt();
      moments2[0] = moments[0];
      for(k=1;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
      for(k=1;k<=3;k++)
        moments2[k] = moments2[k-1]+durees2[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du1", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree1 = inputMessage;
      writeFile(SPIFFS, dur1Path, duree1.c_str());
      durees[0] = duree1.toInt();
      for(k=1;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

// Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du2", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree2 = inputMessage;
      writeFile(SPIFFS, dur2Path, duree2.c_str());
      durees[1] = duree2.toInt();
      for(k=2;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
// Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du3", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree3 = inputMessage;
      writeFile(SPIFFS, dur3Path, duree3.c_str());
      durees[2] = duree3.toInt();
      for(k=3;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
// Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du4", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree4 = inputMessage;
      writeFile(SPIFFS, dur4Path, duree4.c_str());
      durees[3] = duree4.toInt();
      for(k=4;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du5", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree5 = inputMessage;
      writeFile(SPIFFS, dur5Path, duree5.c_str());
      durees[4] = duree5.toInt();
      for(k=5;k<=6;k++)
        moments[k] = moments[k-1]+durees[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du6", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree6 = inputMessage;
      writeFile(SPIFFS, dur6Path, duree6.c_str());
      durees[5] = duree6.toInt();
      moments[6] = moments[5]+durees[5];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/du9", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      duree9 = inputMessage;
      writeFile(SPIFFS, dur9Path, duree9.c_str());
      durees2[0] = duree9.toInt();
      for(k=1;k<=3;k++)
        moments2[k] = moments2[k-1]+durees2[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/duA", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      dureeA = inputMessage;
      writeFile(SPIFFS, durAPath, dureeA.c_str());
      durees2[1] = dureeA.toInt();
      for(k=2;k<=3;k++)
        moments2[k] = moments2[k-1]+durees2[k-1];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

// Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/duB", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      dureeB = inputMessage;
      writeFile(SPIFFS, durBPath, dureeB.c_str());
      durees2[2] = dureeB.toInt();
      moments2[3] = moments2[2]+durees2[2];
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
      for(k=0;k<=3;k++)
        Serial.println(moments2[k]);
    request->send(200, "text/plain", "OK");
  });
   
  // Route to set GPIO to HIGH
  server.on("/marche01", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=0){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[0]=1;
      digitalWrite(sortie[0],HIGH);
    }
    else {
      etat[0] = 0;
      digitalWrite(sortie[0],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });
  
// Route to set GPIO to HIGH
  server.on("/marche02", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=1){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[1]=1;
      digitalWrite(sortie[1],HIGH);
    }
    else {
      etat[1] = 0;
      digitalWrite(sortie[1],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

// Route to set GPIO to HIGH
  server.on("/marche03", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=2){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[2]=1;
      digitalWrite(sortie[2],HIGH);
    }
    else {
      etat[2] = 0;
      digitalWrite(sortie[2],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/marche04", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=3){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[3]=1;
      digitalWrite(sortie[3],HIGH);
    }
    else {
      etat[3] = 0;
      digitalWrite(sortie[3],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/marche05", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=4){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[4]=1;
      digitalWrite(sortie[4],HIGH);
    }
    else {
      etat[4] = 0;
      digitalWrite(sortie[4],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/marche06", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    if (modeArr==0) {
      for(k=0;k<6;k++){
        if(k!=5){
          etat[k]=0;
          digitalWrite(sortie[k],LOW);
        }
      }
      etat[5]=1;
      digitalWrite(sortie[5],HIGH);
    }
    else {
      etat[5] = 0;
      digitalWrite(sortie[5],LOW);
    }
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret01", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[0]=0;
    digitalWrite(sortie[0],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret02", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[1]=0;
    digitalWrite(sortie[1],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret03", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[2]=0;
    digitalWrite(sortie[2],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret04", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[3]=0;
    digitalWrite(sortie[3],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret05", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[4]=0;
    digitalWrite(sortie[4],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });

  // Route to set GPIO to HIGH
  server.on("/arret06", HTTP_GET, [](AsyncWebServerRequest *request){
  String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    etat[5]=0;
    digitalWrite(sortie[5],LOW);
    request->send(SPIFFS, "/manuel.html", "text/html", false, processorH);
  });
  
// Route to set GPIO to HIGH
  server.on("/modAuto", HTTP_GET, [](AsyncWebServerRequest *request){
    modeArr = 1;
    for(k=0;k<6;k++){
      etat[k] = 0;
      digitalWrite(sortie[k],LOW);
    }
    request->send(SPIFFS, "/index.html", String(), false, processorH);
  });
  // Route to set GPIO to HIGH
  server.on("/modManu", HTTP_GET, [](AsyncWebServerRequest *request){
    modeArr = 0;   
    request->send(SPIFFS, "/index.html", String(), false, processorH);
  });
  // Route to set GPIO to HIGH
  server.on("/modAnnu", HTTP_GET, [](AsyncWebServerRequest *request){
    modeArr = -1;   
    for(k=0;k<6;k++){
      etat[k] = 0;
      digitalWrite(sortie[k],LOW);
    }
    request->send(SPIFFS, "/index.html", String(), false, processorH);
  });
  
  // Start server
  server.begin();
  minuActu = 0;
}
 
void loop(){
  maintenant = rtc.now();
  minuPast = minuActu;
  minuActu = maintenant.hour()*60+maintenant.minute();

  Serial.print(". :  ");
  Serial.println(minuActu);
  
  enCours = -1;
  // Branchement selon le jour
  if ((maintenant.dayOfTheWeek()==4)) //  jeudi
  {
    // Arrosage des pelouses permis
    for(k=0;k<6;k++)
    {
      if ((minuActu>=moments[k]) && (minuActu<moments[k+1])) enCours = k; 
    }
  }
  else
  {
    // Arrosage des pelouses interdit
    for(k=0;k<3;k++)
    {
      if ((minuActu>=moments2[k]) && (minuActu<moments2[k+1])) enCours = k+3; 
    }
  }
 // Serial.print(enCours);

  if (modeArr==1){
    for(k=0;k<6;k++){
      if (k==enCours) {
        digitalWrite(sortie[k],HIGH);
        etat[k] = 1;
      }
      else {
        digitalWrite(sortie[k],LOW);
        etat[k] = 0;
      }
    }
  }
  delay(10000);
}
