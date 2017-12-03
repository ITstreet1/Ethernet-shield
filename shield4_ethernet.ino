/*----------------------------------------- 
 * ino fajl je rađen za potrebe članka u Svetu Kompjutera.
 * Kod je jednim delom modifikacija projekta koji možete naći na www.tehnopage.ru/ethernet-arduino-control
 * Skeč da bi se pravilno upotrebio, pored ino fajla potrebno je preuzeti i komplet web fajlova koji treba da 
   budu snimljeni na mikro SD kartici
 * Author: Petrović Dejan
 * Date  : 16 novembar 2017
 * Arduino UNO, Ethernet shield, DHT22
-------------------------------------------*/
#include <DHT.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#define DHTPin 7
#define REQ_BUF_SZ 20

DHT dht;
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0};
char req_index = 0;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 2, 203);

EthernetServer server(80);
void setup() {
  Serial.begin(9600);
  Serial.print("server je na ");
  Serial.println(Ethernet.localIP()); // IP adresa koju kucamo u adresnu liniju našeg omiljenog web pregledača
  dht.setup(DHTPin);
  SD.begin(4);
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {

  // čekamo klijenta
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) { // ako je klijent povezan i...
      if (client.available()) { // .. ako je prisutan na času...
        char c = client.read();
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c; // čuvamo HTTP request u karakteru
          req_index++;
        }
        if (c == '\n' && currentLineIsBlank) {
          if (StrContains(HTTP_req, "GET / ") || StrContains(HTTP_req, "GET /index.htm")) {//ako postoji HTTP request index.htm
            client.println("HTTP/1.1 200 OK"); // standardni HTTP zahtev
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            webFile = SD.open("index.htm");
          } else if (StrContains(HTTP_req, "GET /temp.png")) {//ako postoji HTTP request temp.png
            webFile = SD.open("temp.png");
            if (webFile) {
              client.println("HTTP/1.1 200 OK");
              client.println();
            }
          } else if (StrContains(HTTP_req, "GET /vlaga.png")) {//ako postoji HTTP request vlaga.png
            webFile = SD.open("vlaga.png");
            if (webFile) {
              client.println("HTTP/1.1 200 OK");
              client.println();
            }
          }  else if (StrContains(HTTP_req, "GET /main.css")) {//ako postoji HTTP request main.css
            webFile = SD.open("main.css");
            if (webFile) {
              client.println("HTTP/1.1 200 OK");
              client.println();
            }
          } else if (StrContains(HTTP_req, "ajaxDHT")) {// ako je ajax poslao zahtev
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            float temp = dht.getTemperature();
            float vlaga = dht.getHumidity();
            client.print(temp);
            client.print(":");
            client.print(vlaga);
            Serial.print("Temperatura je ");
            Serial.println(temp);
            Serial.print("Vlaga je ");
            Serial.println(vlaga);
            // o ispisivanju dobijenih podataka sa DHT22 smo više puta pisali
          } 
          if (webFile) {
            while (webFile.available()) {
              client.write(webFile.read()); // šaljemo web stranicu klijentu
            }
            webFile.close();
          }
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void StrClear(char *str, char length)
{
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

char StrContains(char *str, char *sfind)
{
  char found = 0;
  char index = 0;
  char len;
  len = strlen(str);
  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }
  return 0;
}
