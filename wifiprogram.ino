

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payloadTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadTemp += (char)payload[i];
  }

  int indeks_awal = payloadTemp.indexOf('*') + 1;
  int indeks_akhir = payloadTemp.indexOf('#');

  String data = payloadTemp.substring(indeks_awal,indeks_akhir);

   
    if (data == "pompa on") {
      digitalWrite(relay, HIGH);
      Serial.print("on");
    }
    else if (data == "pompa off"){
      digitalWrite(relay, LOW);
      Serial.print("off");
    }
  
  Serial.println();
 }

void setup_wifi() {

  delay(10);
  //we start by connecting to a Wifi network 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP adress: ");
  Serial.println(WiFi.localIP());
}

 

 void reconnect (){
  //loop until we're connected
  while (!client.connected()){
    Serial.print("Attemping MQTT connection. . .");
    //create a random client ID 
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    //attemp to connect 
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/esp32/mqtt/in");
      client.subscribe("/mqtt-esp32/controlpump/shafa");
    }else {
      Serial.print("failed,rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

