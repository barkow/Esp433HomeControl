#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <RCSwitch.h>
#include "secrets.h"

RCSwitch mySwitch = RCSwitch();
MDNSResponder mdns;
// Replace with your network credentials

class GenericSocketHandler : public RequestHandler {
  
  bool genericSocketStatus[16][4][4] = {{{0}}};
  
  bool canHandle(HTTPMethod method, String uri) {
    return uri != NULL && uri.startsWith("/generic/");
  }

  bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {   
    Serial.println(requestUri);
    //Extract 3 indexes
    unsigned int indexes[3];
    String command;
    unsigned int i = 0;
    char* tok;
    tok = strtok((char*)(requestUri.c_str()), "/");
    while (tok != NULL){
      Serial.println(tok);
      switch(i){
        case 1:
          if (strlen(tok) > 1){
            Serial.println("token too long");
            server.send(400, "text/html");
            return true;
          }
          if ((tok[0] < 'a') || (tok[0] > 'l')){
            Serial.println("token out of range");
            server.send(400, "text/html");
            return true;
          }
          indexes[0] = tok[0] - 'a';
          break;
        case 2:
        case 3:
          indexes[i-1] = atoi(tok);
          if ((indexes[i-1] < 1)||(indexes[i-1] > 4)){
            server.send(400, "text/html");
            return true;
          }
          break;
        case 4:
          command = String(tok);
          break;
      }
      i++;
      tok = strtok(NULL, "/");
    }

    if (i == 5){
      Serial.println("set [" + String('a' + indexes[0]) + "][" + String(indexes[1]) + "][" + String(indexes[2]) + "] to " + command);
      if(command == "on") {
        mySwitch.switchOn('a' + indexes[0], indexes[1], indexes[2]);
        delay(500);
        genericSocketStatus[indexes[0]][indexes[1]][indexes[2]] = true;
      }
      if(command == "off") {
        mySwitch.switchOff('a' + indexes[0], indexes[1], indexes[2]);
        delay(500);
        genericSocketStatus[indexes[0]][indexes[1]][indexes[2]] = false;
      }
      if(command == "status") {
        Serial.println(genericSocketStatus[indexes[0]][indexes[1]][indexes[2]]);
      }
      server.send(200, "text/html");
    }
    else {
      server.send(400, "text/html");
    }
    return true;
  }
} genericSocketHandler;

ESP8266WebServer server(80);
char* socketnames[] = {"LichtWohnSofaFenster", 
                       "LichtWohnKommode", 
                       "LichtWohnStehlampe", 
                       "LichtSchlafKommode", 
                       "LichtKindStehlampe", 
                       "LichtSchlafNachttisch"};
char socketcodes[][3] = {{'g', 1, 4}, 
                         {'g', 1, 1}, 
                         {'g', 1, 2}, 
                         {'g', 2, 3}, 
                         {'g', 2, 2}, 
                         {'g', 2, 1}};

int numofsockets = 6;

void setup(void){
  //433Mhz auf GPIO2
  mySwitch.enableTransmit(2);
  delay(1000);

  //Mit Wifi verbinden
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  MDNS.begin("espHomeControl");

  // this page is loaded when accessing the root of esp8266´s IP
  server.on("/", [](){
    //String webPage = mywebsite;
    //server.send(200, "text/html", webPage);
    server.send(200, "text/html");
  });
  
  // pages for all your sockets are created dynamical
  for(int i = 0; i < numofsockets; i++){
    String namesocket = socketnames[i];
    String pathOn = "/setPlug/"+namesocket+"/on";
    const char* pathOnChar = pathOn.c_str();
    
    String pathOff = "/setPlug/"+namesocket+"/off";
    const char* pathOffChar = pathOff.c_str();
    
    server.on(pathOnChar, [i](){
      server.send(200, "text/html");
      mySwitch.switchOn(socketcodes[i][0], socketcodes[i][1], socketcodes[i][2]);
      delay(1000);
    });
    
    server.on(pathOffChar, [i](){
      server.send(200, "text/html");
      mySwitch.switchOff(socketcodes[i][0], socketcodes[i][1], socketcodes[i][2]);
      delay(1000);
    });
  }

  //generic socket
  server.addHandler(&genericSocketHandler);
  server.begin();

  //DEBUG
  Serial.begin(115200);
}
void loop(void){
  server.handleClient();
}
