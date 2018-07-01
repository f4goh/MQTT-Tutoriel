/*
  Basic MQTT example
  with bme 280
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SparkFunBME280.h>



// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);         //adresse de l'arduino client
//IPAddress server(192, 168, 1, 150);  //si utilisation d'un broker sur raspberry pi par exemple
const char * server = "broker.shiftr.io";

EthernetClient ethClient;

PubSubClient mqtt(ethClient);

BME280 mySensor;

#define altitude 80
#define ecartPressionMer altitude/9.144  //1hpa = 30 pieds calcul ecart QNH/QFE

struct datas {
  float temperatureC;
  float pression ;
  float humidite;
};
datas measures;


void setup()
{
  Serial.begin(57600);

  Ethernet.begin(mac, ip);

  mqtt.setServer(server, 1883);  //adresse et port du serveur

  mqtt.setCallback(callback);   //au cas ou l'arduino devient subscriber

  initBme();
  
  delay(1500);
}

void loop()
{
  if (mqtt.connected()) {   //si connection au serveur publish 
    pubCapteur();
    mqtt.loop();
  }
  else {
    reconnect();      //sinon tentative de reconnection
  }
}

void pubCapteur()   
{
  char buffer[10];
  bmeRead();                                      //mesure des 3 valeurs : pression,temp,humidite
  dtostrf( measures.temperatureC, 2, 2, buffer);  //conversion en chaine de caractères
  mqtt.publish("/sensors/temperature", buffer);   //envoie de la donnée dans le bon topic
  dtostrf( measures.pression, 4, 2, buffer);
  mqtt.publish("/sensors/pression", buffer);
  dtostrf( measures.humidite, 2, 2, buffer);
  mqtt.publish("/sensors/humidite", buffer);
  Serial.println("datas published");
  delay(10000);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect("arduino", "weatherSensors", "bme280Sensors")) {
      Serial.println("connected");
      // subscribe if necessary
      //mqtt.subscribe("/sensors/temperature");       
      // mqtt.unsubscribe("/sensors/temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {    //si une donnée arrive en provenance du broker, celle ci est affichée
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

/*
 gestion du BME280
 */

void bmeRead()
{
  measures.temperatureC =  mySensor.readTempC();
  measures.pression = mySensor.readFloatPressure() / 100 + ecartPressionMer;
  measures.humidite = mySensor.readFloatHumidity();


  Serial.print("Temperature: ");
  Serial.print(measures.temperatureC, 2);
  Serial.println(" degrees C");

  Serial.print("Pressure: ");
  Serial.print(measures.pression, 2);
  Serial.println(" hPa");

  Serial.print("%RH: ");
  Serial.print(measures.humidite, 2);
  Serial.println(" %");

  Serial.println();


}



void initBme()
{
  //***Driver settings********************************//
  //commInterface can be I2C_MODE or SPI_MODE
  //specify chipSelectPin using arduino pin names
  //specify I2C address.  Can be 0x77(default) or 0x76

  //For I2C, enable the following and disable the SPI section
  mySensor.settings.commInterface = I2C_MODE;
  mySensor.settings.I2CAddress = 0x76;

  //For SPI enable the following and dissable the I2C section
  //mySensor.settings.commInterface = SPI_MODE;
  //mySensor.settings.chipSelectPin = 10;


  //***Operation settings*****************************//

  //renMode can be:
  //  0, Sleep mode
  //  1 or 2, Forced mode
  //  3, Normal mode
  mySensor.settings.runMode = 3; //Normal mode

  //tStandby can be:
  //  0, 0.5ms
  //  1, 62.5ms
  //  2, 125ms
  //  3, 250ms
  //  4, 500ms
  //  5, 1000ms
  //  6, 10ms
  //  7, 20ms
  mySensor.settings.tStandby = 0;

  //filter can be off or number of FIR coefficients to use:
  //  0, filter off
  //  1, coefficients = 2
  //  2, coefficients = 4
  //  3, coefficients = 8
  //  4, coefficients = 16
  mySensor.settings.filter = 0;

  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.tempOverSample = 1;

  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.pressOverSample = 1;

  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.humidOverSample = 1;


  Serial.print("Program Started\n");
  Serial.print("Starting BME280... result of .begin(): 0x");

  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  Serial.println(mySensor.begin(), HEX);
  
}



