/*------------------------------------------------------------------------------
  02/01/2019
  Author: Makerbro
  Platforms: ESP8266
  Language: C++/Arduino
  File: webserver_http_upload.ino
  ------------------------------------------------------------------------------
  Description: 
  Code for YouTube video demonstrating how to upload files to the File System 
  (SPIFFS) quickly using HTTP upload.
  https://youtu.be/QLGwI5tC9yk
  Do you like my videos? You can support the channel:
  https://patreon.com/acrobotic
  https://paypal.me/acrobotic
  ------------------------------------------------------------------------------
  Please consider buying products from ACROBOTIC to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use. 
  https://acrobotic.com/
  https://amazon.com/acrobotic
  ------------------------------------------------------------------------------
  License:
  Please see attached LICENSE.txt file for details.
------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

char* ssid = "913";
char* password = "Qwerty0123!";

#define D4 2

// hold uploaded file
File fsUploadFile;

void setup()
{
  pinMode(D4, OUTPUT);
  SPIFFS.begin();
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/",handleIndexFile);
  // list available files
  server.on("/list", HTTP_GET, handleFileList);
  // handle file upload
  server.on("/upload", HTTP_POST, [](){
    server.send(200, "text/plain", "{\"success\":1}");
  }, handleFileUpload);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  digitalWrite(D4, HIGH);
  webSocket.loop();
  server.handleClient();
  if(Serial.available() > 0) {
    char c[] = { (char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }
}

void handleFileUpload()
{
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if(!filename.startsWith("/"))
      filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE)
  {
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END)
  {
    if(fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileList()
{
  String path = "/";
  // Assuming there are no subdirectories
  Dir dir = SPIFFS.openDir(path);
  String output = "[";
  while(dir.next())
  {
    File entry = dir.openFile("r");
    // Separate by comma if there are multiple files
    if(output != "[")
      output += ",";
    output += String(entry.name()).substring(1);
    entry.close();
  }
  output += "]";
  server.send(200, "text/plain", output);
}

void handleIndexFile()
{
  File file = SPIFFS.open("/index.html","r");
  server.streamFile(file, "text/html");
  file.close();
  server.send_P(200, "text/html", "hola");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT) {
    for(int i = 0; i < length; i++)
      Serial.print((char) payload[i]);
    Serial.println();
  } 
}
