//////////////////////////WIFI//CREDENCIALES///////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <math.h>

//////////////////////////SD//LIBRERIAS///////////////////////////////////////////////////
#include <SD.h>
#include <SPI.h>
/////////////////////////USAMOS//LOS//PINES//SERIALES//DEL//PM2.5/////////////////////////////
// El pin TX del sensor debe conectarse al RX del NODE MCU
#include "PMS.h"
PMS pms(Serial);
PMS::DATA data;
////////////////////////DEFINIMOS//EL//PIN//DEL//SENSOR//DHT22/////////////////////////////////
#include "DHT.h" //cargamos la librería DHT
#define DHTPIN D1 //Seleccionamos el pin en el que se conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT22(hay otros DHT11)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
//////////////////////////CONFIGURACION//DEL//SERVIDOR////////////////////////////////////////
// /pm25/aire-lima-2.5/neodatokairosdate/registrarMedidasDateKairos;
// /aire-lima-2.5/arduinoneodatokairos/insertedToKairOS
const String url =  "/pm25/aire-lima-2.5/arduinoneodatokairos/insertedToKairOS";
const char* host = "www.tecnologiaperu.com"; //localhost or www.tecnologiaperu.com
const uint16_t port = 80;  // HTTP PORT
//////////////////////DEFINIMOS//EL//CODIGO//DEL//SENSOR//RESPECTIVAMENTE//////////////////////
String codigoPM = "LPLIBRESUCPM2501";
String codigoDT = "LPLIBRESUCTEMP01";
String codigoDH = "LPLIBRESUCHUME01";
String stateCorrectOperation = "1";
String stateWithoutInternet  = "2";
String stateDamageddevice    = "3";
///////////////////////////PARA//LEER//LOS//DATOS///////////////////////////////////////////////
#define PIN_CS D8
String fecha;
int posicion = 0;
byte byteLectura;
File dataWrite, dataRead;
const String FILE_Name = "aire_libre.txt";
////////////////////CREAMOS//VARIABLES//PARA//GUARDAR//LOS//DATOS/////////////////////////////
String token = "TeamKairos.ky.a17f4799d19a72390e0e35a7c4787ab6";
float medidaPM = 0;
float medidaTEM = 0;
float medidaHUM = 0;
double AQI_PM25 = 0;
////////////////////PARA//EL//SERVER/NTP////////////////////////////////////////////////////
const long utcOffsetInSeconds = -18000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "south-america.pool.ntp.org", utcOffsetInSeconds);
time_t t;
/////////////////////////////////////////////////////////////////////////////////////////////
WiFiClient client;
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long previousMillis = 0;
unsigned long previousMillis_2 = 0;
unsigned long previousMillis_3 = 0;
unsigned long previousMillis_4 = 0;
/////////////////////////////////////////////////////////////////////////////////////////////
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
String ssid     = "";
String password = "";

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";

IPAddress local_IP(172, 168, 0, 7);
IPAddress gateway(172, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server(80);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// HTML web page to handle 2 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="X-UA-Compatible" content="ie=edge">
<title>KairOS</title>
<style>
body{
  background-color: #E6C00B;
  margin:0;
}
.form{
  width:660px;
  height:440px;
  background:#e6e6e6;
  border-radius:8px;
  box-shadow:0 0 40px -10px #000;
  margin:calc(50vh - 220px) auto;padding:20px 30px;
  max-width:calc(100vw - 40px);
  box-sizing:border-box;
  font-family:'Montserrat',sans-serif;
  position:relative;
}

h2{
  margin:10px 0;
  padding-bottom:10px;
  width:260px;
  color:#78788c;
  border-bottom:3px solid #78788c
}

h4{
  color:#78788c;
}
.mensaje{  
  font-family:'Montserrat';
  color:#78788c;
}

input{
  width:100%;
  padding:10px;
  box-sizing:border-box;
  background:none;
  outline:none;
  resize:none;
  border:0;
  font-family:'Montserrat',sans-serif;transition:all .3s;
  border-bottom:2px solid #bebed2;
}

input:focus{
  border-bottom:3px solid #78788c;
}
p:before{
  content:attr(type);
  display:block;
  margin:28px 0 0;
  font-size:14px;
  color:#5a5a5a;
}
.enviar{
  float:right;
  padding:8px 12px;
  margin:8px 0 0;
  font-family:'Montserrat',sans-serif;
  border:2px solid #78788c;
  background:0;
  color:#5a5a6e;
  cursor:pointer;
  transition:all .3s;
  border-radius: 5px;
}
.enviar:hover{
  background:#78788c;
  color:#fff
}

.contacto{
  content:'Hi';
  position:absolute;
  bottom:-15px;
  right:-20px;
  background:#50505a;
  color:#fff;
  width:330px;
  padding:16px 4px 16px 0;
  border-radius:6px;
  font-size:13px;
  box-shadow:10px 10px 40px -14px #000;
}
span{
  margin:0 5px 0 15px;
}

.card {
  box-shadow: 0 5px 8px 0 rgba(0,0,0,0.2);
  transition: 0.3s;
  width: 70%;
  padding: 35px;
}

.card:hover {
  box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2);
}
</style>
<script type="text/javascript">
 function validarPasswd () {   
  var p1 = document.getElementById("password").value;
  var p2 = document.getElementById("passwordconf").value;
  var espacios = false;
  var cont = 0;
  // Este bucle recorre la cadena para comprobar
  // que no todo son espacios
  while (!espacios && (cont < p1.length)) {
    if (p1.charAt(cont) == " ")
      espacios = true;
    cont++;
  }   
  if (espacios) {
   alert ("La contraseña no puede contener espacios en blanco");
   return false;
  }
   
  if (p1.length == 0 || p2.length == 0) {
   alert("Los campos de la password no pueden quedar vacios");
   return false;
  }
   
  if (p1 != p2) {
   alert("Las passwords deben de coincidir");
   return false;
  } else {
   alert("Todo esta correcto");
   return true; 
  }
 }
</script>
</head>
<body>
<form class="form" onSubmit="return validarPasswd()" action="/get">  
  <table>
    <tr>
       <td>
        <div class="card">       
            <h4>⌨ Panel de Configuración</h4> 
            <p class="mensaje">
              ⚡ 
              Bienvenid@, en el siguiente formulario escriba correctamente sus credenciales de WiFi.
              <br><br>
              Gracias por colaborar con PM 2.5, usted ahora podra monitorear la calidad de aire en su zona.

            </p> 
          </div>        
       </td>
       
       <td>
        <h2>Air Quality ☁ KairOS </h2>
        <p type="SSID:">
          <input type="text" 
          name="ssid" 
          id="ssid" 
          placeholder="✎ Ingrese su SSID"></input>
        </p>

        <p type="Password:">
          <input type="text" 
          name="password" 
          id="password" 
          placeholder="✎ Ingrese su password"></input>
        </p>

        <p type="Password:">
          <input type="text" 
          name="passwordconf" 
          id="passwordconf" 
          placeholder=" ✎Vuelva a escribir su password ">
        </input>
        </p>
        <input class="enviar" type="submit" value="Enviar">
       </td>
    </tr>
  </table>
  <div class="contacto">
    <span class="fa fa-phone"></span>☏ (+51) 912 101 970
    <span class="fa fa-envelope-o"></span>✉ rcabello@kairos.com.pe
  </div>
</form>
</body></html>
)rawliteral";
/////////////////////////////////////////////////////////////////////////////////////////
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
/////////////////////////////////////////////////////////////////////////////////////////////
String enviardatos(String datos) {
  String linea = "procesando";
  // Use WiFiClient para crear conexiones TCP
  Serial.print("conectando al servidor:  ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  if (!client.connect(host, port)) {
    Serial.println("Conexion Fallida - Intente Nuevamente");
    delay(500);
    return linea;
  }

  client.print(String("POST ") + url + " HTTP/1.1" + "\r\n" +
               "Host: " + host + "\r\n" +
               "Accept: */*" + "*\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
               "\r\n" + datos);
  delay(10);
  Serial.print("enviando por la URL: ");
  Serial.println(url + "->" + datos);
  Serial.println("Enviando datos a Servidor (BD)...");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Cliente fuera de tiempo!");
      client.stop();
      return linea;
    }
  }
  Serial.println(linea);
  return linea;

}
void setup() {

  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(9600);
  /////////////////////////////////////////////////////////////////////////////////////////////
  WiFi.mode(WIFI_STA);
  Serial.println();

  Serial.print("Setting AP_KairOS_PL configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  Serial.print("Setting AP_KairOS_PL ... ");
  Serial.println(WiFi.softAP("AP_KairOS_PL", "987654321") ? "Ready" : "Failed!");

  Serial.print("AP_KairOS_PL IP address = ");
  Serial.println(WiFi.softAPIP());
  /////////////////////////////////////////////////////////////////////////////////////////////
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });
  /////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////
  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String gettingSSID;
    String gettingPASS;
    String inputParamSSID;
    String inputParamPASS;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      gettingSSID  = request->getParam(PARAM_INPUT_1)->value();
      gettingPASS = request->getParam(PARAM_INPUT_2)->value();
      inputParamSSID  = PARAM_INPUT_1;
      inputParamPASS  = PARAM_INPUT_2;
      //WiFi.softAPdisconnect(true);
      //WiFi.disconnect(true);
    }
    else {
      gettingSSID = "No message sent SSID";
      inputParamSSID = "none SSID";
      gettingPASS = "No message sent PASS";
      inputParamPASS = "none PASS";
    }
    Serial.println(gettingSSID);
    Serial.println(gettingPASS);
    ssid = gettingSSID;
    password = gettingPASS;
    double ipWiFi_Print;
    String ip;
    /////////////////////////////////////////////////////////////////////////////////////////////
    if (ssid != "" && password != "") {
      Serial.print("'Conectando Modulo' a la RED WiFi espere por favor (...) : ");
      Serial.println(ssid);
      delay(700);
      WiFi.begin(ssid, password);
      Serial.println("WiFi Conectado");
      Serial.println("Mi direccion IP: ");
      Serial.println(WiFi.localIP());
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    request->send(200, "text/html", "<script>alert('Gracias'); window.location.href = '/';</script>");
  });
  /////////////////////////////////////////////////////////////////////////////////////////////
  server.onNotFound(notFound);
  server.begin();
  /////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (!SD.begin(PIN_CS)) {
    Serial.println("Memoria SD - No Inicializada (...)");
  } else {
    Serial.println("Memoria SD, listo para ser conectada (...)");
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("Node MCU Chip Id : ");
  String chip_id = String(ESP.getChipId());
  Serial.println(chip_id);
  /////////////////////////////////////////////////////////////////////////////////////////////
  dht.begin(); //Se inicia el sensor de DHT22
  ////////////////////////////////////////////////////////////////////////////////////////////
  timeClient.begin();
  setTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds(), 2, 1, 2019);
  timeClient.update();
}

void loop() {

  t = now();//Declaramos la variable time_t t

  if (pms.read(data)) {

    unsigned long currentMillis_2 = millis();
    if (currentMillis_2 - previousMillis_2 >= 360000) { //Definimos el tiempo de envio 360000
      previousMillis_2 = currentMillis_2;
      medidaPM  = data.PM_AE_UG_2_5;
      AQI_PM25  = calcularAQIPM25(medidaPM);
      medidaTEM = dht.readTemperature(); //Se lee la temperatura
      medidaHUM = dht.readHumidity(); //Se lee la humedad

      Serial.println("-----------------------------------------");
      Serial.print("PM 2.5 AQI: ");
      Serial.println(AQI_PM25);
      Serial.println("-----------------------------------------");
      Serial.print("Humedad: ");
      Serial.println(medidaHUM);
      Serial.print("Temperatura: ");
      Serial.println(medidaTEM);
      Serial.println("-----------------------------------------");

      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Guardando datos en la memoris SD (...)");
        getWriteSD();
      } else {
        if (SD.exists(FILE_Name)) {
          Serial.println("Enviando datos al Sever desde el SD (...)");
          getDataReadSD();
        } else {
          Serial.println("Enviando datos al Sever en tiempo Real (...)");
          getSaveHost();
        }

      }

    }
  }
}

void getSaveHost() {
  ///////////////TODOS//LOS//PINES//DEBEN//ESTAR//CONECTADOS//AL//NODE//MCU////////////////////
  unsigned long currentMillis = millis();
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (currentMillis - previousMillis >= 360000) { //Definimos el tiempo de envio
    previousMillis = currentMillis;
    //token=$2y$10$nqrcd7ycxMR.Z533JNPjuusmnWh7XLWm/a6HKZ6mAeVKXqXHnMUee&codigox=ALIPMSA001&medidax=6&codigoy=ALIPMSA001&mediday=6&codigoz=ALIMDTE001&temperaturaz=6&codigon=ALIMDHU001&humedadn=6&estado=6
    enviardatos("token=" + token +
                "&codigow=" + codigoPM +
                "&medidaw=" + AQI_PM25 +
                "&codigox=" + codigoDT +
                "&medidax=" + medidaTEM +
                "&codigoy=" + codigoDH +
                "&mediday=" + medidaHUM +
                "&fecha" +
                "&estado=" + stateCorrectOperation);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
}
String getRealTime() {
  //2019-07-23 21:48:01
  String anio = String(year(t));
  String mes  = String(month(t));
  String dia  = String(day(t));
  String hora = timeClient.getFormattedTime();
  String fecha =  anio + "/" + mes + "/" + dia + " " + hora;
  fecha.trim();
  delay(1000);
  return fecha;
}

void getWriteSD() {
  dataWrite = SD.open(FILE_Name, FILE_WRITE);
  unsigned long currentMillis_3 = millis();
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (currentMillis_3 - previousMillis_3 >= 360000) { //Definimos el tiempo de envio
    previousMillis_3 = currentMillis_3;
    fecha = getRealTime();   //Obtenemos la fecha del Servidor
    if (dataWrite) {

      dataWrite.println("token=" + token +
                        "&codigow=" + codigoPM +
                        "&medidaw=" + AQI_PM25 +
                        "&codigox=" + codigoDT +
                        "&medidax=" + medidaTEM +
                        "&codigoy=" + codigoDH +
                        "&mediday=" + medidaHUM +
                        "&fecha=" + getRealTime() +
                        "&estado=" + stateWithoutInternet);

      Serial.println("token=" + token +
                     "&codigow=" + codigoPM +
                     "&medidaw=" + AQI_PM25 +
                     "&codigox=" + codigoDT +
                     "&medidax=" + medidaTEM +
                     "&codigoy=" + codigoDH +
                     "&mediday=" + medidaHUM +
                     "&fecha=" + getRealTime() +
                     "&estado=" + stateWithoutInternet);

      dataWrite.flush(); //saving the file
      dataWrite.close(); //closing the file
    }
    else {
      Serial.println("No se puede registrar en el archivo 'aire_libre.txt'");
    }
  }
}

String getDataReadSD() {
  String fila;
  char c;
  String getCadena;
  unsigned long currentMillis_4 = millis();

  if (currentMillis_4 - previousMillis_4 >= 2000) { //Definimos el tiempo de envio
    previousMillis_4 = currentMillis_4;
    dataRead = SD.open(FILE_Name, FILE_READ);
    dataRead.seek(posicion);
    if (dataRead) {
      while (dataRead.available()) {
        c = dataRead.read();
        fila = String(fila + c);
        if (c == '\n') {
          getCadena = fila;
          posicion = dataRead.position();
          break;
        }
      }
      dataRead.close();
    } else {
      Serial.println("ERROR al realizar las lecturas");
    }
    getCadena.trim();
    //Se procedera a enviar al Servidor Principal
    enviardatos(getCadena);
    if (fila.length() == 0) {
      Serial.println("Todos los datos se enviaron correctamente");
      SD.remove(FILE_Name);
      Serial.println("El archivo " + FILE_Name + " se elimino...");
    }
    return getCadena;
  }
  delay(2000);
}
static double calcularAQIPM25(double concentration) {

  double conc = concentration;
  double c;
  double AQI;
  c = floor(10 * conc) / 10;

  if (c >= 0 && c < 12.1) {
    AQI = Linear(50, 0, 12, 0, c);
  } else if (c >= 12.1 && c < 35.5) {
    AQI = Linear(100, 51, 35.4, 12.1, c);
  } else if (c >= 35.5 && c < 55.5) {
    AQI = Linear(150, 101, 55.4, 35.5, c);
  } else if (c >= 55.5 && c < 150.5) {
    AQI = Linear(200, 151, 150.4, 55.5, c);
  } else if (c >= 150.5 && c < 250.5) {
    AQI = Linear(300, 201, 250.4, 150.5, c);
  } else if (c >= 250.5 && c < 350.5) {
    AQI = Linear(400, 301, 350.4, 250.5, c);
  } else if (c >= 350.5 && c < 500.5) {
    AQI = Linear(500, 401, 500.4, 350.5, c);
  } else {
    AQI = 1000;
  }
  return AQI;
}

static double Linear(double AQIhigh, double AQIlow, double Conchigh, double Conclow, double concentration) {
  double linear;
  double conc = concentration;
  double a;
  a = (conc - Conclow) / (Conchigh - Conclow) * (AQIhigh - AQIlow) + AQIlow;
  linear = round(a);
  return linear;
}
