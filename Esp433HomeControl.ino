#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <RCSwitch.h>
#include "secrets.h"

RCSwitch mySwitch = RCSwitch();
MDNSResponder mdns;
// Replace with your network credentials

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
  server.begin();
}
void loop(void){
  server.handleClient();
}
