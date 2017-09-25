/** Reloj despertador para formato de 12hrs **/
/*
 *   Programa:   Reloj Despertador
 *   Autores:    Martín Alejandro Pérez Güendulain
 *               Angel Robeto Montez Murgas
 *   
 *   Funcionamiento: 
 *    
 *   El reloj puede configurar la hora y la alarma.
 *   Para la configuración de la hora se reliza mediante el monitor serie,
 *   siguiendo el formato como se muestra a continuación.
 *   
 *   - 01:30:00am
 *   - 11:30:00pm
 *   
 *   NOTA: La entrada debe ser "01" en vez de "1" para números menor a 10.
 *   
 *   Para la configuración de la alarma se realiza a través del lcd de 16x2,
 *   usando el botón de "select" ( entrada analógica A0) para entrar en un estado 
 *   de configuración y los botones restantes para el desplazamiento. Para salir 
 *   y guardar la alarmar pulsar el botón "select" nuevamente.
 *   
 *   Materiales;
 *   
 *   - Display lcd de 16x2
 *   - una bocina
 *   - Un arduino uno
 *
 */
# include <LiquidCrystal.h>
# include <TimerOne.h>

// Botones del LCD ( Entradas analógicas en A0 )
# define btnRIGHT  0
# define btnUP     1
# define btnDOWN   2
# define btnLEFT   3
# define btnSELECT 4
# define btnNONE   5

// Himno venga la alegria ( Frecuencia de las notas musicales )
# define  Re3   294
# define  Sol3  392
# define  La3   440
# define  Si3   494
# define  Do4   523
# define  Re4   587

// Botón de la bocina en el pin 2
# define P2_3 2  

// Variables para lectura de los datos
char inChar;
String entrada = "";
String alarma = "";
String dato = "";

char horas[3] = {1,0,'\0'};
char minutos[3] = {0,0,'\0'};
char segundos[3] = {0,0,'\0'};
char tiempo[3] = {'a','m','\0'};
String infoHora = " hora ";

// Variables extras
int lcd_key = 0;
int adc_key_in = 0;

int posicion = 1;
int celda = 1;

// variables para la hora actual
int hrs = 1;
int mint = 0;
int seg = 0;

// variables para la hora la alarma
int alarma_hrs = 12;
int alarma_mint = 0;
int alarma_seg = 0;
char alarma_tiempo[3] = {'a','m','\0'};
String infoAlarma = " alarm";

bool estado = false;                  // Variable auxiliar para controlar el retardo de la hora
bool configuracion = false;           // Variable para entrar en configuración de la alarma

LiquidCrystal lcd(8,9,4,5,6,7);       // Inicialización de la librería con los números de la interface

void setup(){
  Serial.begin(9600);                 // Inicia la comunicación con velocidad de transmisión de 9600
  entrada.reserve(50);

  pinMode(P2_3, OUTPUT);              // Salida de la bocina

  Timer1.initialize(1000000);         // Microsegundos (1 segundo)
  Timer1.attachInterrupt(reloj);      // Verifica el estado en cada interrupción
  
  lcd.begin(16,2);                    // Coloca el número de columnas y filas del LCD
  lcd.setCursor(0,0);                 // Comenzando en la primera posición
}

void loop(){
  if( Serial.available() > 0 ){       // Si el búffer tiene algo lee la hora introducida
    inChar = Serial.read();
    entrada += inChar;

    // Lectura de la hora ingresada por monitor serial
    if( inChar == 'm' ){              // La cadena con la hora introducida termina con m
      Serial.print("Hora establecida: ");
      Serial.println(entrada);

      Serial.print("Horas: ");
      horas[0] = entrada[0];
      horas[1] = entrada[1];
      hrs = atoi(horas);
      Serial.println(hrs);

      Serial.print("Minutos: ");
      minutos[0] = entrada[3];
      minutos[1] = entrada[4];
      mint = atoi(minutos);
      Serial.println(mint);
      
      Serial.print("Segundos: ");
      segundos[0] = entrada[6];
      segundos[1] = entrada[7];
      seg = atoi(segundos);
      Serial.println(seg);

      Serial.print("Tiempo: ");
      tiempo[0] = entrada[8];
      tiempo[1] = entrada[9];
      Serial.println(tiempo);
      
      //Borra la variable para almacenar nuevos datos
      entrada = ""; 
    }
  }

  // Lee la entrada analógica A0 para saber que botón se presionó
  lcd_key = read_LCD_buttons();
  if ( lcd_key == btnSELECT ){ // Si presiona el botón "select" configura la alarma
    configuracion = true;
    Serial.println("Entrando a la configuracion...");
    delay(500);
    configuraAlarma();
  }
  else{                        // Se imprime la hora
    posicion = 1;
    celda = 1;

    // Con la variable "estado" se controla la hora real
    if( estado == false ){
      lcd.setCursor(0,0);
      imprimeHoraActual();
      lcd.setCursor(0,1);
      dato = "Reloj - Alarma  ";
      lcd.print(dato);
    }

    comparaAlarma();           // Compara si la alarma debe sonar
  }
}

void reloj(){                  // Aumenta hora en cada interrupción
  estado = true;
  segReloj();
  estado = false;  
}

void segReloj(){               // Aumenta los segundos de la hora
  seg++;
  if( seg == 60 ){             // Si los segundos llegan a 60 incrementan los minutos
    seg = 0;
    minReloj();
  }
}

void minReloj(){               // Aumenta los minutos de la hora
  mint++;
  if( mint == 60 ){ // Si los minutos llegan a 60 incrementan las horas
    mint = 0;
    hrsReloj();
  }  
}

void hrsReloj(){               // Aumenta la hora
  hrs++;
  if( hrs == 12 ){             // Se cambia el tiempo am a pm( ante - post meridiam )
    merReloj();
  }
  else if( hrs == 13 ){
    hrs = 1;
  }
}

void merReloj(){               // Coloca "am" o "pm" según sea el caso
  if( tiempo[0] == 'a' )
    tiempo[0] = 'p';
  else
    tiempo[0] = 'a';  
}

void imprimeHoraActual(){      // Imprime la hora actual y se muestra en el display
  String horaLCD = "";

  if( hrs < 10 ){              // Si las horas son menor a 10, agrega un cero antes
    horaLCD += "0";
    horaLCD += hrs;
  }
  else
    horaLCD += hrs;
  
  horaLCD += ":";
  if( mint < 10 ){             // Si los minutos son menor a 10, agrega un cero antes
    horaLCD += "0";
    horaLCD += mint;
  }
  else
    horaLCD += mint;
    
  horaLCD += ":";
  if( seg < 10 ){              // Si los segundos son menor a 10, agrega un cero antes
    horaLCD += "0";
    horaLCD += seg;
  }
  else
    horaLCD += seg;
    
  horaLCD += tiempo;
  horaLCD += infoHora;
  lcd.print(horaLCD);
}

void imprimeAlarma(){          // Imprime la alarma configurada en el display
  String alarmaLCD = "";

  if( alarma_hrs < 10 ){       // Si las horas son menor a 10, agrega un cero antes
    alarmaLCD += "0";
    alarmaLCD += alarma_hrs;
  }
  else
    alarmaLCD += alarma_hrs;
  
  alarmaLCD += ":";
  if( alarma_mint < 10 ){      // Si los minutos son menor a 10, agrega un cero antes
    alarmaLCD += "0";
    alarmaLCD += alarma_mint;
  }
  else
    alarmaLCD += alarma_mint;
    
  alarmaLCD += ":";
  if( alarma_seg < 10 ){       // Si los segundos son menor a 10, agrega un cero antes
    alarmaLCD += "0";
    alarmaLCD += alarma_seg;
  }
  else
    alarmaLCD += alarma_seg;
    
  alarmaLCD += alarma_tiempo;
  alarmaLCD += infoAlarma;
  lcd.print(alarmaLCD);
}

void configuraAlarma(){        // Muestra los efectos durante la configuración de la alarma
  Timer1.stop();
  int botonLCD;
  dato = "Horas           ";
  
  while( configuracion ){
    delay(50);
    botonLCD = read_LCD_buttons();

    lcd.noBlink();
    lcd.setCursor(0,0);
    imprimeAlarma();
    lcd.blink();

    switch(posicion){
      case 1: dato = "Horas           ";
              break;
      case 2: dato = "Minutos         ";
              break;
      case 3: dato = "Segundos        ";
              break;
      case 4: dato = "Tiempo          ";
              break;
    }

    if( botonLCD == btnRIGHT ){    // Si presiona el botón derecha
      Serial.println("Derecha");
      if( posicion == 1 ){         // Si esta ubicado en horas
        posicion = 2;
        celda = 4;
        //dato = "Horas           ";
        delay(75);
      }
      else if( posicion == 2 ){    // Si esta ubicado en minutos
        posicion = 3;
        celda = 7;
        //dato = "Minutos         ";
        delay(75);
      }
      else if( posicion == 3 ){    // Si esta ubicado en segundos
        posicion = 4;
        celda = 8;
        //dato = "Segundos        ";
        delay(75);
      }
      else if( posicion == 4 ){    // Si esta ubicado en tiempo (am o pm)
        /* ¯\_(ツ)_/¯ */           // No realiza ninguna acción
        //dato = "Tiempo          ";
      }
    }
    else if ( botonLCD == btnLEFT ){ // Si presiona el botón izquierda
      Serial.println("Izquierda");
      if( posicion == 1 ){           // Si esta ubicado en horas
        /* ¯\_(ツ)_/¯ */             // No realiza ninguna acción
        //dato = "Horas           ";
      }
      else if( posicion == 2 ){   // Si esta ubicado en minutos
        posicion = 1;
        celda = 1;
        //dato = "Minutos         ";
        delay(75);
      }
      else if( posicion == 3 ){   // Si esta ubicado en segundos
        posicion = 2;
        celda = 4;
        //dato = "Segundos        ";
        delay(75);
      }
      else if( posicion == 4 ){   // Si esta ubicado en tiempo (am o pm)
        posicion = 3;
        celda = 7;
        //dato = "Tiempo          ";
        delay(75);
      }
    }
    else if ( botonLCD == btnUP ){ // Si presiona el botón arriba
      Serial.println("Arriba");
      if( posicion == 1 ){         // Si esta ubicado en horas
        if( alarma_hrs < 12 ){
          alarma_hrs++;
          delay(75);
        }
      }
      else if( posicion == 2 ){    // Si esta ubicado en minutos
        if( alarma_mint < 59 ){
          alarma_mint++;
          delay(75);
        }
      }
      else if( posicion == 3 ){    // Si esta ubicado en segundos
        if( alarma_seg < 59 ){
          alarma_seg++;
          delay(75);
        }
      }
      else if( posicion == 4 ){    // Si esta ubicado en tiempo (am o pm)
        alarma_tiempo[0] = 'p';
        delay(75);
      }
    }
    else if ( botonLCD == btnDOWN ){ // Si presiona el botón abajo
      Serial.println("Abajo");
      if( posicion == 1 ){           // Si esta ubicado en horas
        if( alarma_hrs > 1 ){
          alarma_hrs--;
          delay(75);
        }
      }
      else if( posicion == 2 ){      // Si esta ubicado en minutos
        if( alarma_mint > 0 ){
          alarma_mint--;
          delay(75);
        }
      }
      else if( posicion == 3 ){      // Si esta ubicado en segundos
        if( alarma_seg > 0 ){
          alarma_seg--;
          delay(75);
        }
      }
      else if( posicion == 4 ){      // Si esta ubicado en tiempo (am o pm)
        alarma_tiempo[0] = 'a';
        delay(75);
      }
    }
    else if ( botonLCD == btnSELECT ){ // Si presiona el botón select
      delay(75);
      Serial.println("Saliendo de la configuracion...");
      configuracion = false;
    }

    lcd.setCursor(0,1);
    lcd.print(dato);

    lcd.setCursor(celda,0);
  }

  delay(500);
  lcd.noBlink();
  Timer1.start();
}

void comparaAlarma(){     // Compara la alarma con la hora actual
  if( hrs == alarma_hrs && mint == alarma_mint && seg == alarma_seg && tiempo[0] == alarma_tiempo[0] ){
    Serial.println("ALARMA!!!");

    lcd.setCursor(0,0);
    imprimeAlarma();
    
    alarmaActivada();
    Timer1.stop();
    
    suenaAlarma();
    
    Timer1.start();
    
    lcd.setCursor(0,0);
    imprimeHoraActual();
  }
}

void alarmaActivada(){    // Muestra efecto de alarma activada en el display
  
  for( int i=1; i < 5; i++ ){
    lcd.noDisplay();
    delay(500);
    lcd.display();
    delay(500);
  }
}

int read_LCD_buttons(){   // Indentifica el botón presionado en el display
  adc_key_in = analogRead(0); // Leemos A0
      
  if( adc_key_in > 1023 )     // Ningun boton pulsado
    return btnNONE; 
  if( adc_key_in < 50 )       // Boton Derecha pulsado
    return btnRIGHT;
  if( adc_key_in < 250 )      // Boton Arriba pulsado
    return btnUP;
  if( adc_key_in < 300 )      // Boton Abajo pulsado
    return btnDOWN;
  if( adc_key_in < 450 )      // Boton Izquierda pulsado
    return btnLEFT;
  if( adc_key_in < 650 )      // Boton Seleccion pulsado
    return btnSELECT;

  return btnNONE;             // Si hay un error
}

void suenaAlarma(){           // Suena la canción del "Himno a la alegría"
  tone(P2_3, Si3);   delay(1000);
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Re4);   delay(500);  noTone(P2_3);
  tone(P2_3, Re4);   delay(500);
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Si3);   delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Sol3);  delay(500);  noTone(P2_3);
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(500);  noTone(P2_3);
  tone(P2_3, Si3);   delay(750);
  tone(P2_3, La3);   delay(250);  noTone(P2_3);
  tone(P2_3, La3);   delay(1000);

  tone(P2_3, Si3);   delay(1000);    
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Re4);   delay(500);  noTone(P2_3);
  tone(P2_3, Re4);   delay(500);
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Si3);   delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Sol3);  delay(500);  noTone(P2_3);
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(500);  noTone(P2_3);
  tone(P2_3, La3);   delay(750);
  tone(P2_3, Sol3);  delay(250);  noTone(P2_3);
  tone(P2_3, Sol3);  delay(1000);

  tone(P2_3, La3);   delay(500);  noTone(P2_3);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(500);   
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(250);   
  tone(P2_3, Do4);   delay(250);
  tone(P2_3, Si3);   delay(500);
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(250);   
  tone(P2_3, Do4);   delay(250);
  tone(P2_3, Si3);   delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Re3);   delay(1000);
  
  tone(P2_3, Si3);   delay(500);  noTone(P2_3);
  tone(P2_3, Si3);   delay(500);    
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Re4);   delay(500);  noTone(P2_3);
  tone(P2_3, Re4);   delay(500);
  tone(P2_3, Do4);   delay(500);
  tone(P2_3, Si3);   delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Sol3);  delay(500);  noTone(P2_3);
  tone(P2_3, Sol3);  delay(500);
  tone(P2_3, La3);   delay(500);
  tone(P2_3, Si3);   delay(500);  noTone(P2_3);
  tone(P2_3, La3);   delay(750);
  tone(P2_3, Sol3);  delay(250);  noTone(P2_3);
  tone(P2_3, Sol3);  delay(1000);
  noTone(P2_3);
}
