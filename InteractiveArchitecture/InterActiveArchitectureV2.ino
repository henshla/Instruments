#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

//WIFI
char ssid[] = "Test";     // your network SSID (name)
char pass[] = "test1234";  // your network password

WiFiUDP Udp;                               // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(10, 0, 1, 255);  // remote IP of your computer
const unsigned int outPort = 9999;         // remote port to receive OSC
const unsigned int localPort = 8888;       // local port to listen for OSC packets (actually not used for sending)


//PINS
#define CS2 18
#include <Adafruit_MCP3008.h>

Adafruit_MCP3008 adc1;


//The differents reading system, 0 is to use the touchRead of the ESP, 2 to use analogRead of the ESP, 1 the an analogRead in the multiplexer
int adc[] =  { 0, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 };  

//1 2 5 for the capacitiv sensor, 7 6 for the  iF sensor, 1 2 3 4 5 6 7 for the potentiometer on the multiplexer
int pins[] = { 1, 2, 5, 7, 6, 0, 1, 2, 3, 4, 5, 6, 7 };

//Name of the msg
String paths[] = {
  "/T1", "/T2", "/T3", "/F1", "/F2", "/P1", "/P2", "/P3", "/P4", "/P5", "/P6", "/P7", "/P8"
};

//CODE NOT UPDATE ! => 4 capacitive touch, 4 iF, 12 potentiometer and a second multiplexer.
#define MAX_PINS 13




void setup() {
  Serial.begin(115200);
  Serial.println("MCP3008 simple test.");
  //ChipSelect : 18 et 4  mosi=14  miso =20 clk = 21
  // Hardware SPI (specify CS, use any available digital)
  // Can use defaults if available, ex: UNO (SS=10) or Huzzah (SS=15)
  pinMode(CS2, OUTPUT);
  adc1.begin(21, 14, 9, 18);  //Faudrait mettre 47 à la place de 20 ?
  pinMode(18,OUTPUT);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  
  // delete old config
  WiFi.disconnect(true);
  WiFi.begin(ssid, pass);

  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif
}

void loop() {
  char cMesg[64] = { 0 };

  //Reading the list for sending the value. 
  //The data are not filters, it's done in TouchDesigner.
  
  for (int i = 0; i < MAX_PINS; i++) {
    OSCMessage msg(paths[i].c_str());

    if (adc[i] == 0) {
      // filtA = (A * 20 + filt * 80) / 100;  //filtrage "1 moins alpha" (20% de la valeur brute, 80% de la valeur filtree, a modifier suivant les besoins, avec X% et Y%, en prenant garde que X+Y reste = à 100)
      // Serial.println(touchRead(pins[i]));
      msg.add((int32_t)touchRead(pins[i])); 

    } else if (adc[i] == 2) {
      // Serial.println(analogRead(pins[i]));
      msg.add((int32_t)analogRead(pins[i]));

    } else if (adc[i] == 1) {
      // Serial.printf("adc %i: %i\t", pins[i], adc1.readADC(pins[i]));
      msg.add((int32_t)adc1.readADC(pins[i]));
    }
    //Envoie des valeurs
    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);    // send the bytes to the SLIP stream
    Udp.endPacket();  // mark the end of the OSC Packet
    msg.empty();      // empty the bundle to free room for a new one
    delay(10);
  }