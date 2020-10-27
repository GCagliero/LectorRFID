/* 
   Creado por Gonzalo Cagliero 29/04/2020.  
   *Modo configuracion de SSID y Password de router.
   *Envia mensaje si se conecta o no.
   *Este codigo esta apareado con LectorBasicoOk_Disp-Bz-Btn6.
*/




//------------------------SETUP WIFI-----------------------------
void setup_wifi(boolean *estado) {
  WiFi.begin(ssid, pass);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(250);
  }
  if (contconexion <50) {
    lcd.clear();
    lcd.setCursor(0, 1);   
    lcd.print("Conectado!"); 
  }
  else { 
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Error conexion");
    *estado = HIGH;
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Listo para leer");
  }
}
//--------------------------------------------------------------

