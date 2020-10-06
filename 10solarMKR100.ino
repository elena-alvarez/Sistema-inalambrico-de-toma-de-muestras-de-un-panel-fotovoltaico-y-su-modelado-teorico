/*  TFM Elena Álvarez Castro
	Código microcontrolador Arduino mkr1000
*/

	#include <SPI.h>
	#include <WiFi101.h>
	#include <SPI.h>
	#include <SD.h>
	#include <avr/dtostrf.h>
	#include <Wire.h>
	#include <Adafruit_MCP4725.h>

	Adafruit_MCP4725 dac;

	#define MOSFET 2
	#define RELE   3

	#define PAGINA_INICIAL    0
	#define NUEVA_TRAZA       1
	#define FAVICON           2
	#define TEXTOSD           3
	#define NUEVO_FICHERO     4
	#define NUEVA_TRAZA_FLOAT 5
	#define NUEVO_NIVEL_LUZ   6
	#define MAXIMA_LONGITUD_NOMBRE_FICHERO 16 
	int peticion;
	char  ficheroSolicitado[MAXIMA_LONGITUD_NOMBRE_FICHERO]; // 8.3+null

	//mi wifi
	char ssid[] = "Andared";        // your network SSID (name)
	char pass[] = "llevalatararaunvestidoblancollenodecascabeles";  

									
	int keyIndex = 0;               // your network key Index number (needed only for WEP)

	int led =  LED_BUILTIN;
	int status = WL_IDLE_STATUS;
	WiFiServer server(80);

	void CortocircuitaPlacaSolar(boolean);
	void EnviaDatosCliente(WiFiClient);
	void EnviaDatosNuevaTraza(WiFiClient);
	void EnviaFicheroSD(WiFiClient);
	int TomaMuestras(void);
	void EnviaHtml(char *);
	void AnalizaActualPeticion(String);
	void ConversorDA();


	#define NUMERO_MUESTRAS 512
	unsigned int arrCorriente[NUMERO_MUESTRAS];
	unsigned int arrTension  [NUMERO_MUESTRAS];
	char  cadenaF[16]=" ";
	float corrienteF;
	float tensionF;
	char miBuffer[1048];

	//Sensor de Temperatura
	// Tabla generada
	#define NUMTEMPS 20
	short temptable[NUMTEMPS][2] = {
	   {1, 713},
	   {54, 236},
	   {107, 195},
	   {160, 172},
	   {213, 157},
	   {266, 144},
	   {319, 134},
	   {372, 125},
	   {425, 117},
	   {478, 110},
	   {531, 103},
	   {584, 96},
	   {637, 89},
	   {690, 83},
	   {743, 75},
	   {796, 68},
	   {849, 59},
	   {902, 48},
	   {955, 34},
	   {1008, 3}
	};
	int THERMISTOR_PIN = A3;


	const char nombre[]= "hola\
	 gIV = document.getElementById(\"IV\");\
	{hola}";

	File ficheroSd;
	void setup() {
	  Serial.begin(9600);
	  delay(1000); //Para poder continuar aunque no haya puerto serie.
	  Serial.println("Access Point Web Server");

	  pinMode(led, OUTPUT);      
	  pinMode(RELE, OUTPUT);
	  pinMode(MOSFET, OUTPUT);
	 
	  digitalWrite(RELE, LOW);
	  digitalWrite(MOSFET, HIGH);
	  
	  pinMode(A1, INPUT);
	  pinMode(A2, INPUT);

	  if (WiFi.status() == WL_NO_SHIELD) {
		Serial.println("WiFi shield not present");
		while (true);
	  }

	  Serial.print("Creating access point named: ");
	  Serial.println(ssid);
	  /* Codigo para modo punto de acceso
	  // Create open network. Change this line if you want to create an WEP network:
	  status = WiFi.beginAP(ssid);
	  if (status != WL_AP_LISTENING) {
		Serial.println("Creating access point failed");
		// don't continue
		while (true);
	  }

	  // wait 10 seconds for connection:
	  delay(10000);
	  */

	  while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		delay(10000);
	  }
	  // start the web server on port 80
	  server.begin();

	  // you're connected now, so print out the status
	  printWiFiStatus();

	  Serial.print("Initializing SD card...");

	  if (!SD.begin(4)) {
		Serial.println("initialization SD failed!");
	  }
	  else
		Serial.println("initialization SD done.");

	  dac.begin(0x62);
	  ConversorDA(2000);
	}


	void loop() {
	  if (status != WiFi.status()) {
		// it has changed update the variable
		status = WiFi.status();

		if (status == WL_CONNECTED) {
		  byte remoteMac[6];

		  Serial.print("Device connected to AP, MAC address: ");
		  WiFi.APClientMacAddress(remoteMac);
		  printMacAddress(remoteMac);
		} else {
		  
		  Serial.println("Device disconnected from AP");
		  status = WiFi.begin(ssid, pass);
		  Serial.println("Conectando de nuevo ...");
		  delay(1000);
		  server.begin();

		}
	  }
	  
	  WiFiClient client = server.available();   // listen for incoming clients

	  if (client) {                             // if you get a client,
		Serial.println("new client");           // print a message out the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		//int   peticion = PAGINA_INICIAL;       // solicitud del cliente
		int   escritos = 0;
		char  ficheroSolicitado[MAXIMA_LONGITUD_NOMBRE_FICHERO]; // 8.3+null
		unsigned int   longitudCadenaGet;
		int i,j;
		
		while (client.connected()) {            // loop while the client's connected
		  if (client.available()) {             // if there's bytes to read from the client,
			char c = client.read();             // read a byte, then
			Serial.write(c);                    // print it out the serial monitor
			if (c == '\n') {                    // if the byte is a newline character

			  // if the current line is blank, you got two newline characters in a row.
			  // that's the end of the client HTTP request, so send a response:
			  if (currentLine.length() == 0) {
				// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
				// and a content-type so the client knows what's coming, then a blank line:
				client.println("HTTP/1.1 200 OK");
				//client.println("Connection: close");  // the connection will be closed after completion of the response

				//client.println("Content-type:text/html");
				//client.println("Connection: close");  // the connection will be closed after completion of the response
				//client.println();
				switch (peticion){
				  case NUEVO_FICHERO:
					EnviaFicheroSD(client);
				  case PAGINA_INICIAL:
					/*
					Serial.println("Pide Pagina");
					EnviaHtml(client, codigoCabecera);
					EnviaHtml(client, codigoJavaScript);
					EnviaHtml(client, codigoBody);
					EnviaDatosCliente(client);
					EnviaHtml(client, codigoFinBody);
					client.println();
					*/
					break;
				  case NUEVA_TRAZA:
					EnviaDatosNuevaTraza(client);
					//EnviaHtml(client, cabeceraNuevaTraza);
					client.println();
					break;
				  case NUEVA_TRAZA_FLOAT:
					EnviaDatosNuevaTrazaFloat(client);
				  case FAVICON:
					break;
				  case TEXTOSD:
					EnviaFicheroSD(client);
					break;              
				  case NUEVO_NIVEL_LUZ:
					client.println();
					break;
				}
				// break out of the while loop:
				break;
			  }
			  else {      // if you got a newline, then clear currentLine:
				AnalizaActualPeticion(currentLine);
				currentLine = "";
			  }
			}
			else if (c != '\r') {    // if you got anything else but a carriage return character,
			  currentLine += c;      // add it to the end of the currentLine
			}

			// Check to see if the client request was "GET /H" or "GET /L":
			if (currentLine.endsWith("GET /H")) {
			  digitalWrite(led, HIGH);               // GET /H turns the LED on
			}
			if (currentLine.endsWith("GET /L")) {
			  digitalWrite(led, LOW);                // GET /L turns the LED off
			}
			if (currentLine.endsWith("GET /nDatos.txt")) {
			  peticion = NUEVA_TRAZA;
			}
			
			
		  }
		}
		// close the connection:
		client.stop();
		Serial.println("client disconnected");
	  }
	  //ConversorDA(2500);
	  /*
	  Serial.print("Temperatura:");
	  int celsius = read_temp();
	  Serial.println(celsius);
	  */
	}

	void printWiFiStatus() {
	  // print the SSID of the network you're attached to:
	  Serial.print("SSID: ");
	  Serial.println(WiFi.SSID());

	  // print your WiFi shield's IP address:
	  IPAddress ip = WiFi.localIP();
	  Serial.print("IP Address: ");
	  Serial.println(ip);

	  // print the received signal strength:
	  long rssi = WiFi.RSSI();
	  Serial.print("signal strength (RSSI):");
	  Serial.print(rssi);
	  Serial.println(" dBm");
	  // print where to go in a browser:
	  Serial.print("To see this page in action, open a browser to http://");
	  Serial.println(ip);

	}

	void printMacAddress(byte mac[]) {
	  for (int i = 5; i >= 0; i--) {
		if (mac[i] < 16) {
		  Serial.print("0");
		}
		Serial.print(mac[i], HEX);
		if (i > 0) {
		  Serial.print(":");
		}
	  }
	  Serial.println();
	}
	void CortocircuitaPlacaSolar (boolean conmuta){
	  if (conmuta){ 
		 digitalWrite(3,  LOW);    //Rele
		 delay(25);
		 digitalWrite(LED_BUILTIN, HIGH);
		 digitalWrite(2,  HIGH);   //Mosfet
		
	  }
	  else{
		 digitalWrite(2,  LOW);     //Mosfet
		 digitalWrite(LED_BUILTIN, LOW);
		 digitalWrite(3,  HIGH);   //Rele
	  }
	}
	void EnviaDatosNuevaTrazaFloat(WiFiClient cliente){
	  int indiceBuffer = 0;
	  char sIntensidad []= "{\"i\":";
	  char sTension    []= ",\"v\":";
	  char sTemperatura[]= "\"temperatura\":";
	  int temperatura;
	  int numeroMuestras;

	  /*Envia los datos de la traza en formato float json */
	  /* Preparamos los datos en un buffer antes de enviarlos */

	  Serial.println("EnviaDatosNuevaTrazaFloat");
	  numeroMuestras = TomaMuestras();
	  Serial.println ("nviando Muestras");
	  cliente.println("Content-type:text/html");
	  cliente.println("Connection: close");
	  cliente.println();
	  
	  //cliente.print("[");
	  miBuffer[indiceBuffer++]='{';
	  strncpy(miBuffer+indiceBuffer,sTemperatura,strlen(sTemperatura));
	  indiceBuffer += strlen(sTemperatura);
	  
	  temperatura = read_temp();
	  sprintf(miBuffer+indiceBuffer, "%d", temperatura);
	  indiceBuffer=strlen(miBuffer);
	  miBuffer[indiceBuffer++]=',';

	  strcpy(miBuffer+indiceBuffer, "\"muestras\":");
	  indiceBuffer=strlen(miBuffer);
	 
	  miBuffer[indiceBuffer++]='[';
	  
	  for (int i = 0; i<numeroMuestras; i++){ //formato float
		if (i) //cliente.print(',');
			 miBuffer[indiceBuffer++]=',';

		//cliente.print("{\"i\":");
		strncpy(miBuffer+indiceBuffer,sIntensidad,strlen(sIntensidad));
		indiceBuffer +=strlen(sIntensidad);
		
		corrienteF = arrCorriente[i]*3.22265*0.0001;
		dtostrf(corrienteF,-9,5,cadenaF);//dato, anchoTotal, decimales, buffer
		//cliente.print(cadenaF);
		strncpy(miBuffer+indiceBuffer,cadenaF,strlen(cadenaF));
		indiceBuffer +=strlen(cadenaF);

		
		//cliente.print(",\"v\":");
		strncpy(miBuffer+indiceBuffer,sTension,strlen(sTension));
		indiceBuffer +=strlen(sTension);

		tensionF   = arrTension[i]*3.22265*0.001*6.667;
		dtostrf(tensionF,-9,5,cadenaF);//dato, anchoTotal, decimales, buffer
		//cliente.print(cadenaF);
		strncpy(miBuffer+indiceBuffer,cadenaF,strlen(cadenaF));
		indiceBuffer +=strlen(cadenaF);

	 
		//cliente.print("}");
		miBuffer[indiceBuffer++]='}';

		if(indiceBuffer > 1000){
		   cliente.write(miBuffer, indiceBuffer);
		   indiceBuffer = 0;
		}
	  }
	  if(indiceBuffer){
		   cliente.write(miBuffer, indiceBuffer);
		   indiceBuffer = 0;
	   }
	  cliente.println("]}"); 
	}
	void EnviaDatosNuevaTraza(WiFiClient cliente){
	  int yPixel;
	  int xPixel;
	  Serial.println("......EnviaDatosNuevaTraza......");
	  TomaMuestras();
	  Serial.println ("nviando Muestras");
	  cliente.println("Content-type:text/html");
	  cliente.println("Connection: close");
	  cliente.println();
	  
	  cliente.print("[");
	  for (int i = 1; i<NUMERO_MUESTRAS; i++){ // 
		//Serial.println(arrCorriente[i]);
		corrienteF = arrCorriente[i]*3.22265*0.0001;
		//Serial.println(corrienteF);
		tensionF   = arrTension[i]*3.22265*0.001*6.667;
		//dtostrf(corrienteF,-9,5,cadenaF);//dato, anchoTotal, decimales, buffer
		yPixel = corrienteF * 2000; //yPixel = (int) corrienteF *(640.0/0.32);
		//Serial.println(yPixel);
		yPixel = 670-yPixel;
		xPixel = (int) (tensionF*(880.0/22.0));
		xPixel += 60; //El centro esta en el punto 60,670

		cliente.print("{\"cx\":");
		cliente.print(xPixel);
		cliente.print(",\"cy\":");
		cliente.print(yPixel);
		cliente.print("},");
		//cliente.print(cadenaF);
		//cliente.print(" ");
	  }
	  cliente.println("{\"cx\": 110,\"cy\": 200}]");
	  /*
	  cliente.print("[{\"cx\": 110,\"cy\": 200},");
	  cliente.print("{\"cx\": 150,\"cy\": 250}]");
	  */ 
	}
	void EnviaDatosCliente(WiFiClient cliente){
	  unsigned int maxArrCorriente = 0; // Para ajustar escalas
	  unsigned int maxArrTension   = 0;   // 
	  int xPixel;
	  int yPixel;
	  char lineaEje[80];
	  //cliente.println("Hola");
	  //Para octave
	  //cliente.print("corriente");
	  //cliente.print(numeroSerie);
	  /**
	   * Datos de la placa para ajuste de escala
	   * Voc = 22 V
	   * Isc = 0.32A
	   */
	  TomaMuestras();
	  cliente.print(F("<svg id=\"IV\" height=\"750\" width=\"1000\" style=\"border:1px solid black\">\n"));

	  //Cada pixel en x corresponde (880/22)*voltios
	  //Cada pixel en y corresponde a (640/0.32)*corriente
	  //el origen de coordenadas estará en los pixles 60,30+640
	  // <circle cx="50" cy="50" r="40" stroke="black" stroke-width="3" fill="red" />
	  // por cada punto dibujaremos un circulo
	  cliente.print(
		"<line x1=\"60\" y1=\"670\" x2=\"940\" y2=\"670\" stroke=\"black\" stroke-width=\"2\" />\n"); //eje x
	  cliente.print(
		"<line x1=\"60\" y1=\"670\" x2=\"60\"  y2=\"30\"  stroke=\"black\" stroke-width=\"2\" />\n"); //eje y
	  cliente.print(
		"<text x=\"300\" y=\"720\" fill=\"black\">Tension (Voltios)</text>\n");
	  cliente.print(
		"<text x=\"15\" y=\"500\" fill=\"black\" style=\"writing-mode: sideways-lr;\">Corriente (Amperios)</text>\n");

	  // Atributos de lineas horizontales y verticales
	  cliente.print ("<g stroke=\"grey\" stroke-width=\"1\" >\n");
	  for (int i = 1; i< 23; i++){ //Lineas verticales
		sprintf(lineaEje, 
		   "<line x1=\"%d\" y1=\"670\" x2=\"%d\" y2=\"30\" />\n",
		   i*(880/22)+60,
		   i*(880/22)+60
		   );
	   cliente.print(lineaEje);
	  }  
	  for (int i = 1; i< 33; i++){ //Lineas horizontales
		sprintf(lineaEje, 
		   "<line x1=\"60\" y1=\"%d\" x2=\"940\" y2=\"%d\" />\n",
		   670-(i*(640/32)+0),
		   670-(i*(640/32)+0)
		   );
	   cliente.print(lineaEje);
	  }
	  cliente.print ("</g>\n");
	  
	  for (int i = 0; i< 23; i++){ //leyenda lineas verticales
		//<text x="0" y="15">I love SVG!</text>
		sprintf(lineaEje, 
		   "<text x=\"%d\" y=\"690\">%d</text>\n",
		   i*(880/22)+60,
		   i
		   );
	   cliente.print(lineaEje);
	  }
	  
	  for (int i = 0; i< 33; i++){ //Leyenda Lineas horizontales
		sprintf(lineaEje, 
		   "<text x=\"20\" y=\"%d\">0.%02d</text>\n",
		   670-(i*(640/32)+0),
		   i
		   );
	   cliente.print(lineaEje);
	  }

	  // Atributos de punto de grafica
	  cliente.print ("<g stroke-width=\"0\" fill=\"green\" >\n");
	  for (int i = 1; i<NUMERO_MUESTRAS; i++){
		Serial.println(arrCorriente[i]);
		corrienteF = arrCorriente[i]*3.22265*0.0001;
		//Serial.println(corrienteF);
		tensionF   = arrTension[i]*3.22265*0.001*6.667;
		//dtostrf(corrienteF,-9,5,cadenaF);//dato, anchoTotal, decimales, buffer
		yPixel = corrienteF * 2000; //yPixel = (int) corrienteF *(640.0/0.32);
		//Serial.println(yPixel);
		yPixel = 670-yPixel;
		xPixel = (int) (tensionF*(880.0/22.0));
		xPixel += 60; //El centro esta en el punto 60,670

		cliente.print("<circle cx=\"");
		cliente.print(xPixel);
		cliente.print("\" cy=\"");
		cliente.print(yPixel);
		cliente.print("\" r=\"2\" />\n");
		//cliente.print(cadenaF);
		//cliente.print(" ");
	  }
	  cliente.print ("</g>\n");
	  //cliente.println(";");

	  //cliente.print("tension");
	  //cliente.print(numeroSerie);
	  /*
	  cliente.print(" ");
	  for (int i = 1; i<NUMERO_MUESTRAS; i++){
		tensionF   = arrTension[i]*3.22265*0.001*6.667;
		dtostrf(tensionF,-9,5,cadenaF);
		cliente.print(cadenaF);
		cliente.print(" ");

	  }
	  */
	  cliente.println(F("</svg>\n"));

	}
	// Retorna el numero de muestras significativas en el array de muestras.
	int TomaMuestras(void){
		 unsigned long microsAntes;
		 unsigned long microsDespues;
		 unsigned long ultimaMuestraCorriente;
		 int           nMuestra = 0;
		 int           incremento = 1;
		 int           mayorIndiceMuestras=0;
		 
		 
		 //CortocircuitaPlacaSolar(false);
		 microsAntes = micros();

		 ultimaMuestraCorriente = 1;  //Para que entre en el bucle
		 nMuestra               = 0;  //Primera muestra que vamos a tomar
		 incremento             = 1;  //Incremento inicial para llenar el 
									  // array de muestras

		  for (int i = 0; i<NUMERO_MUESTRAS; i++){
			arrTension   [i] = 0;
			arrCorriente [i] = 0;
		  }
		 /*
		  * Tomaremos muestras hasta tener corriente cero.
		  * Si antes de tener corriente cero, hemos llenado el array de muestras
		  * volveremos a recorrer el array de muestras
		  * insertando las nuevas tomas en incrementos de 2,3,4, ... y así
		  * sucesivamente hasta llegar a un máximo de 20 pasadas para no provocar
		  * un time out en el cliente.
		  */
		 digitalWrite(MOSFET,LOW);
		 digitalWrite(RELE, HIGH);
		 //delay(1); //Para dar tiempo al rele
		 while (ultimaMuestraCorriente>20 || (nMuestra<5)) {
			arrTension  [nMuestra] = analogRead(A1);
			ultimaMuestraCorriente = arrCorriente[nMuestra] = analogRead(A2);
			nMuestra +=incremento;
			if(nMuestra > mayorIndiceMuestras)
			   mayorIndiceMuestras = nMuestra;  

			if (nMuestra > NUMERO_MUESTRAS-1){
			  incremento ++;
			  nMuestra=incremento+5;
			}
			if (incremento == 15) break;
			delay(10);     
		 }
		 digitalWrite(RELE, LOW);
		 digitalWrite(MOSFET,HIGH);
	   
		 //CortocircuitaPlacaSolar(true);
		 microsDespues = micros();

		 Serial.print("Tiempo de muestreo en microsegundos:");
		 Serial.println(microsDespues-microsAntes);
		 Serial.print(F("Numero Muestra en corte: "));
		 Serial.println(nMuestra);

		 Serial.print(F("Incremento: "));
		 Serial.println(incremento);

		 Serial.print(F("ultimaMuestraCorriente: "));
		 Serial.println(ultimaMuestraCorriente);

		 int presentaColumnas=0;

		 if(presentaColumnas){
		 Serial.println("Corriente Tension");
		 for (int i = 0; i<NUMERO_MUESTRAS; i++){
		  Serial.println("Corriente --- Tensión");
		  Serial.print(arrCorriente[i]);
		  Serial.print("--");
		  Serial.println(arrTension[i]);
		 }
		 for (int i = 0; i<NUMERO_MUESTRAS; i++){
			//Tension de referencia a 3.2 V
			// 
			//3.3/1024 = 3.22265 exp -3  Para Vref = 3.3V

			/******************************************
			* Medimos caida de tension en una R de 1 Ohmio 
			* a la salida de un aplificador con Av=10
			* Valor Real de la corriente. Av=10, R=1 ohmio
			* Por ello dividimos por 10 
			*/
			corrienteF = arrCorriente[i]*3.22265*0.0001;
		   /********************************/
		   /*  Valor Real de Tension:
			*   Divisor de tension 
			*   Vi Tension en la Placa
			*   Vo Tension medida
			*   R1=56.67 Mohm
			*   R2=10 Mohm
			*   Vo=R2/(R1+R2)  * Vi
			*   Vi=(R1+R2)/R2  * Vo
			*   
			*   Vi=(56.67+10)/10 *Vo
			*   Vi=66.67/10 * Vo
			*   Vi = 6.667*Vo
			*/
			tensionF   = arrTension[i]*3.22265*0.001*6.667;

			//Presentamos resultados
			dtostrf(corrienteF,-9,5,cadenaF);//dato, anchoTotal, decimales, buffer
			Serial.print(cadenaF);
			Serial.print(" ");
			dtostrf(tensionF,-9,5,cadenaF);
			Serial.println(cadenaF);
		  }
		}    
		return (mayorIndiceMuestras); 
	}
	void EnviaHtml(WiFiClient cliente, const char *cadenaHtml){
	  int escritos;
	  
	  escritos = cliente.print(cadenaHtml);
	  Serial.print   ("Escritos=");
	  Serial.println (escritos);
	  Serial.print   ("Longitud=");
	  Serial.println (strlen(cadenaHtml));
	  Serial.println (cadenaHtml);
	}

	void EnviaFicheroSD(WiFiClient cliente){
	  int  leidos;
	  int  indicePunto;
	  int  i=0;

	  for (i= 0; i<strlen(ficheroSolicitado); i++){
		if (ficheroSolicitado[i] == '.')
		  indicePunto = i;
	  }

	  if (strcmp(ficheroSolicitado+i, "htm") == 0)
		 cliente.println("Content-type:text/html");
	  if (strcmp(ficheroSolicitado+i, "txt") == 0)
		 cliente.println("Content-type:text/css");
	  if (strcmp(ficheroSolicitado+i, "css") == 0)
		 cliente.println("Content-type:text/css");
	  if (strcmp(ficheroSolicitado+i, "jpg") == 0)
		 cliente.println("Content-type:image/jpeg");
	  if (strcmp(ficheroSolicitado+i, "pdf") == 0)
		 cliente.println("Content-type:application/pdf");
		 
	  cliente.println("Connection: close");
	  cliente.println();
	  Serial.print   ("Enviando Fichero ");
	  Serial.println (ficheroSolicitado);
	  ficheroSd = SD.open(ficheroSolicitado);
	  if (ficheroSd) {
		Serial.println(ficheroSolicitado);
		// read from the file until there's nothing else in it:
		while (ficheroSd.available()) {
		  leidos = ficheroSd.read(miBuffer, 1024);
		  Serial.print("Datos Leidos:");
		  Serial.println(leidos);
		  //Serial.write(ficheroSd.read());
		  cliente.write(miBuffer, leidos);
		}
		// close the file:
		ficheroSd.close();
	  } else {
		// if the file didn't open, print an error:
		cliente.println("ERROR 404");
		Serial.println("error opening fichero");
	  }
	}
	void AnalizaActualPeticion(String linea){
	  int longitudCadenaGet;
	  long nuevoNivel;
	  
	  int i,j;

	  //Cuando pide un fichero se recibe
	  // GET /nombre.abc  /HTTP1.1  hay que extrar el nombre.abc
	  
	  Serial.print("Linea Completa:");
	  Serial.println(linea);
	  if (linea.substring(0,5)=="GET /"){
			  peticion = NUEVO_FICHERO;
			  longitudCadenaGet = linea.length();
			  Serial.print("Longitu Cadena Get =");
			  Serial.println(longitudCadenaGet);
			  //Linea pidiendo pagina
			  if (longitudCadenaGet > 5){
				 for (i=0,j=0; 
					i<(MAXIMA_LONGITUD_NOMBRE_FICHERO -1) & j<longitudCadenaGet; 
					i++,j++){
					  if (linea.charAt(j+5)==' ') break; //!!! OJO....                    
					  ficheroSolicitado[i]=linea.charAt(j+5);
				 }
				 ficheroSolicitado[i]=0; //null
			  }
			  if(strlen(ficheroSolicitado)==0){
				//Fichero index.htm
				strcpy(ficheroSolicitado, "index.htm");
			  }
			  if (strcmp (ficheroSolicitado, "nDatos.txt") == 0){
				peticion = NUEVA_TRAZA;
			  }
			  else if (strcmp (ficheroSolicitado, "nDatosFloat.txt") == 0){
				peticion = NUEVA_TRAZA_FLOAT;
			  }
			  else if (strncmp (ficheroSolicitado, "nL.txt", strlen("nL.txt"))==0){
				Serial.print("Pide nuevo nivelLuz:");
				Serial.println(ficheroSolicitado);
				//La peticion viene como nL.txt?nv=nivel
				nuevoNivel=atol(ficheroSolicitado+strlen("nL.txt?nv="));
				Serial.print("Nuevo Nivel Solicitado:");
				Serial.println(nuevoNivel);
				peticion = NUEVO_NIVEL_LUZ;
				//Nivel bajo 1200
				//Nivel alto
				ConversorDA((nuevoNivel*28.95)+1200);
			  }
			  else {
				peticion = NUEVO_FICHERO;
			  }
			  Serial.print("Fichero Solicitado =");
			  Serial.println(ficheroSolicitado);                              
			} // GET /
	}

	void ConversorDA(long nivel){

		  Serial.print("nivel recibido = ");
		  Serial.println(nivel);
		  dac.setVoltage(nivel, false);

	}
	int read_temp()
	{
	   int rawtemp = analogRead(THERMISTOR_PIN);
	   int current_celsius = 0;

	   byte i;
	   for (i=1; i<NUMTEMPS; i++)
	   {
		  if (temptable[i][0] > rawtemp)
		  {
			 int realtemp  = temptable[i-1][1] + (rawtemp - temptable[i-1][0]) * (temptable[i][1] - temptable[i-1][1]) / (temptable[i][0] - temptable[i-1][0]);

			 if (realtemp > 255)
				realtemp = 255; 

			 current_celsius = realtemp;

			 break;
		  }
	   }

	   // Overflow: We just clamp to 0 degrees celsius
	   if (i == NUMTEMPS)
	   current_celsius = 0;

	   return current_celsius;
	}
