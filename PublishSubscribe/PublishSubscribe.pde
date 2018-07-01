// This example sketch connects to shiftr.io
// https://github.com/256dpi/processing-mqtt

import mqtt.*;

MQTTClient client;

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://weatherSensors:bme280Sensors@broker.shiftr.io", "processing");
  client.subscribe("/sensors/temperature");
  client.subscribe("/sensors/humidite");
  client.subscribe("/sensors/pression");
  // client.unsubscribe("/sensors/temperature");
}

void draw() {}

void keyPressed() {
 // client.publish("/sensors/temperature", "20");
}

void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
}
