
/*  ~ SmartLights ~ -> ESP8266 and LED strip
              ╦═╗╔╗ ╦╔═
      By--    ╠╦╝╠╩╗╠╩╗     2014 Rakshith BK
              ╩╚═╚═╝╩ ╩         
 *  
 *  No licences no copyrights, well may be MIT.
 *  Use modify distribute and do whatever you want
 *  And even though this code will not launch any Nukes, 
 *  I'm not responsible for any destruction caused by it.!
 *  
 *  For Details and working -- 
 */

#include <ESP8266WiFi.h>
//#include <ESP8266.h>

//////////////////// enter your details here  /////////////////////////////////
const char* ssid     = "--your wifi name--";
const char* password = "--wifi password--";
const char* channel_id = "--your channel id--"; //you will find this id and key
const char* apiKey = "--your api key--";        // in your thingspeak account.
/////////////////////////////////////////////////////////////////////////////


const char* host = "api.thingspeak.com";
const int httpPort = 80;
int led = 2 ; // ESP01 Pin 2 for output
String line1, line2 ;
String url1, url2 ;
int action = 0 ;
unsigned long latency = 0 ;

void check_field1() ;
void check_field2() ;
void mail();
void Strip_out();
void facebook();

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(led, OUTPUT);
  analogWrite(led, 0);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

       url1 = "/channels/";
       url1 += channel_id;
       url1 += "/fields/1/last.txt?api_key=";
       url1 += apiKey;
       
       url2 = "/channels/";
       url2 += channel_id;
       url2 += "/fields/2/last.txt?api_key=";
       url2 += apiKey;

  delay(1000);
  check_field1() ;
  Serial.println("initializing strip..!") ;
  Strip_out() ;
}

////////////////////////////////////////////////////////////////
void loop() { 
                             // checks online every 5 minutes
                            //// 60,000 x 5 = 5 minutes
  if((millis() - latency > 60000*5)) { 
    latency = millis();
    check_field2();
    delay(300);
    check_field1();
    yield();
 }

}
  
/////////////////////////////////////////////////////////////////////// 
// check_field1 -> This is for listening for basic commands from IFTTT
// These include ON, OFF and brightness controll.
// It also checks for weather and changes the lights accordingly..!
//////////////////////////////////////////////////////////////////////

void check_field1() {
  client.connect(host, httpPort);
   client.print(String("GET ") + url1 + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "User-Agent: SmartLights-ESP8266\r\n"+
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
    
  while(client.available()){
    line1 = client.readStringUntil('\r');
   
  if(line1.indexOf("Dim") > 0) {
    action = 1 ;      
  }
  else if(line1.indexOf("Bright") > 0){
    action = 2 ;  
  }
  else if(line1.indexOf("ON")>0) {
    action = 3 ; // turn on lights 
  }
  else if(line1.indexOf("OFF")>0) {
    action = 4 ; // turn off lights 
  }
  else if(line1.indexOf("Rain")>0) {
    action = 5 ; //slow dim-glow of strip
  }
 }
 Strip_out() ;
}
////////////////////////////////////////////////////////////
// Check_field2 -> looks for any notification
//                 from connected social media.
////////////////////////////////////////////////////////////

void check_field2() {
  client.connect(host, httpPort);
  client.print(String("GET ") + url2 + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "User-Agent: SmartLights-ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long Timeout = millis();
  while (client.available() == 0) {
    if (millis() - Timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    line2 = client.readStringUntil('\r');
             
  if(line2.indexOf("Mail") > 0){
    Mail();    
    Serial.println("notification -- mail");
  }
  else if(line2.indexOf("Facebook")>0) {
    facebook();
    Serial.println("notification -- facebook");
  }
  }
}
///////////////////////////////////////////////////////////////
void Mail()  {
  int i =0 ;
  for(i=0; i<=1023; i=i+40) {
    analogWrite(led, i);
    delay(100) ;
  }
  for(i=1023; i>=0; i=i-40) {
    analogWrite(led, i);
    delay(100) ;
  }
  Strip_out();
}
///////////////////////////////////////////////////////////
void facebook()  {
  int i=0;
  for(i=0; i<=1023; i=i+20) {
    analogWrite(led, i);
    delay(50);
  }
  analogWrite(led,0);
  delay(250);
  analogWrite(led,1023);
  delay(250);
  analogWrite(led,0);
  delay(250);
  for(i=1023; i>=0; i=i-40) {
    analogWrite(led, i);
    delay(100);
  }
  Strip_out();
}
////////////////////////////////////////////////////////////////

void Strip_out() {
  
  while(action == 0) {    //This action will be active if there is a problem
    analogWrite(led, 0);  //in connecting to the WiFi or the server. 
    delay(700);
    analogWrite(led, 512);
    delay(700);
    analogWrite(led, 1023);
    delay(700);
    if( millis() - latency > 60000) {
      latency = millis();
    Serial.println("Something went wrong.!");
    Serial.println("Please check your server details.!");
    check_field1() ;
    }
  }
  if(action == 1) {
    analogWrite(led,512);// For DIM settings 
  }
  else if(action == 2) {
    analogWrite(led, 768); // For Bright settings 
  }
  else if(action == 3) {
    analogWrite(led, 1023);// For ON  
  }
  else if(action == 4) {
    analogWrite(led, 0);// For OFF
  }
  while(action == 5) {// For rain
    int i ;
    for(i=0; i<=1023; i++) {
      analogWrite(led, i);
      delay(3);
    }
    for(i=1023; i>=0; i--) {
      analogWrite(led, i) ;
      delay(3);
    }
      
      if( millis() - latency > 60000) {
        latency = millis() ;
        check_field1() ;
      }    
  }
}
////////////////////////////////////////////////////////////////

