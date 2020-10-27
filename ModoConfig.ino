//--------------------MODO_CONFIGURACION------------------------
void modoconf() {
  lcd.setCursor(0, 0);
  lcd.print("Configuracion");     
  WiFi.mode(WIFI_STA); //para que no inicie el SoftAP en el modo normal
  WiFi.softAP(ssidConf, passConf);
  server.on("/", paginaconf); //esta es la pagina de configuracion
  server.on("/guardar_conf", guardar_conf); //Graba en la eeprom la configuracion  
  server.begin();
  while (true) {
    server.handleClient();
  }
}

//---------------------GUARDAR CONFIGURACION-------------------------//
void guardar_conf() {  
  server.arg("ssid");//Recibimos los valores que envia por GET el formulario web
  grabar(0,server.arg("ssid"));
  server.arg("pass");
  grabar(50,server.arg("pass"));
  mensaje = "Configuracion Guardada...";
  //mySerial.print("configOk");
  paginaconf();
  delay(1000);
  ESP.restart(); //ESP.reset();
}
//-------------------------------------------------------------------//


//----------------Función para grabar en la EEPROM-------------------//
void grabar(int addr, String a) {
  int tamano = a.length(); 
  char inchar[50]; 
  a.toCharArray(inchar, tamano+1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr+i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr+i, 255);
  }
  EEPROM.commit();
}
//-------------------------------------------------------------------//


//-----------------Función para leer la EEPROM----------------------//
String leer(int addr) {
  byte lectura;
  String strlectura;
  for (int i = addr; i < addr+50; i++) {
    lectura = EEPROM.read(i);
    if (lectura != 255) {
      strlectura += (char)lectura;
    }
  }
  return strlectura;
}
//-----------------------------------------------------------------//
