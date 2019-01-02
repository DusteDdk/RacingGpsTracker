
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>



const int SDCSPIN = 9;
const int FOUNDPIN = 7;
const int SRCHPIN = 6;
const int RECPIN = 5;
const int BTNPIN = 3;

const int RXPIN = 8;    // Receive = Yellow
const int TXPIN = 4;    // Transmit = Green
                        /// GNDPIN = Black


inline int nmeaSum(const char *msg)
{
    int checksum = 0;
    for (int i = 0; msg[i] && i < 32; i++)
        checksum ^= (unsigned char)msg[i];

    return checksum;
}

void nmeaWrite(char* msg) {

  char checkStr[8];
  snprintf(checkStr,7, "*%.2X", nmeaSum(msg));
  Serial1.print("$");
  Serial1.print(msg);
  Serial1.println(checkStr);
}

void nmeaDisable(char* nam) {
  char buf[32];
  snprintf(buf, 31,"PUBX,40,%s,0,0,0,0", nam);
  nmeaWrite(buf);
}

const unsigned char UBLOX_SLOW_RATE[] PROGMEM = {0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39};
void setSlow() {
    for(unsigned int i = 0; i < sizeof(UBLOX_SLOW_RATE); i++) {                       
    Serial1.write( pgm_read_byte(UBLOX_SLOW_RATE+i) );
  }
}

const unsigned char UBLOX_FAST_RATE[] PROGMEM = {0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12};

void setFast() {
    for(unsigned int i = 0; i < sizeof(UBLOX_FAST_RATE); i++) {                       
    Serial1.write( pgm_read_byte(UBLOX_FAST_RATE+i) );
  }
}

void showHelp(SoftwareSerial* ss) {
  ss->println("?=this sNAME=show rNAME=remove d=dir");
}

File root;
void setup() {
  // put your setup code here, to run once:
  pinMode(SDCSPIN, OUTPUT);

  pinMode(SRCHPIN, OUTPUT);
  digitalWrite(SRCHPIN, LOW);

  pinMode(FOUNDPIN, OUTPUT);
  digitalWrite(FOUNDPIN, LOW);

  pinMode(RECPIN, OUTPUT);
  digitalWrite(RECPIN, HIGH);
  
  pinMode(BTNPIN, INPUT);
  digitalWrite(BTNPIN, LOW);




  /*Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
*/
 //Serial.print("Initializing SD card...");

  if (!SD.begin(SDCSPIN)) {
//    Serial.println("initialization failed!");
//    while (1);

    err(FOUNDPIN, 1000);
  }

  delay(50);
  if(digitalRead(BTNPIN)) {
    pinMode(RXPIN, INPUT);
    pinMode(TXPIN, OUTPUT);
    SoftwareSerial ss(RXPIN, TXPIN);

      ss.begin(28800);
      showHelp(&ss);
      uint8_t mode=0;
      char buf[17];
      uint8_t cur=0;
      while(1) {
        if(mode==0) {
          switch( ss.read() ) {
            case '?':
            showHelp(&ss);
            break; 
            case 'd':            
            root=SD.open("/");
            root.rewindDirectory();
            if(!root) {
              ss.println("ERR opening dir.");
            }
            while(1) {
              File entry=root.openNextFile();
              if(!entry) {
                break;
              }
              ss.print(entry.name());
              if (entry.isDirectory()) {
                ss.println("/");
              } else {
                // files have sizes, directories do not
                ss.print("\t");
                ss.println(entry.size(), DEC);
              }
              entry.close();
            }
            root.close();
            break;
            case 's':
              ss.print("Show:");
              mode=1;
              cur=0;
              memset(buf,0,16);
            break;
            case 'r':
              ss.print("Remove:");
              mode=2;
              cur=0;
              memset(buf,0,16);
            break;
          }
        } else if(mode==1) {
          if(ss.available()>0) {
            uint8_t c = ss.read();
            if(c==10) {
            } else if(c==13) {
              buf[cur]=0;
              cur=0;
              mode=0;
              ss.println();
              ss.print("<");
              ss.print(buf);
              ss.println(">");
              File f=SD.open(buf, FILE_READ);
              if(!f) {
                ss.println("ERR");
              } else {
                memset(buf,0,17);
                while( f.read(buf, 16) > 0 ) {
                  ss.print(buf);
                  memset(buf,0,16);
                }

                f.close();
                ss.println("<EOF>");
                
              }
            } else {
              if(cur==16) {
                cur=0;
                mode=0;
                ss.println("ERR");
              } else {
                ss.print((char)c);
                buf[cur]=c;
                cur++;
              }
            }
          }
          
        } else  if(mode==2) {
          if(ss.available()>0) {
            uint8_t c = ss.read();
            if(c==10) {
            } else if(c==13) {
              buf[cur]=0;
              cur=0;
              mode=0;
              ss.println();
              if(SD.remove(buf)) {
                ss.println("REMOVED");
              } else {
                ss.println("ERR");
              }
            } else {
              if(cur==16) {
                cur=0;
                mode=0;
                ss.println("ERR");
              } else {
                ss.print((char)c);
                buf[cur]=c;
                cur++;
              }
            }
          }
          
        }
        
      }
     
  }


//  Serial.println("Gps?");
  Serial1.begin(9600);
  // delay a bit
  delay(500);
//  Serial.println("Gps!");
  setSlow();
  nmeaDisable("VTG");
  nmeaDisable("GGA");
  nmeaDisable("GSA");
  nmeaDisable("GSV");
  nmeaDisable("GLL");


}

void err(uint8_t pin, int d) {
  // Turn them all off
  digitalWrite(FOUNDPIN, HIGH);
  digitalWrite(SRCHPIN, HIGH);
  digitalWrite(RECPIN, HIGH);
  // Enter infinite loop.
    while(1) {
      digitalWrite(pin, LOW);
      delay(d);
      digitalWrite(pin, HIGH);
      delay(d);
    }
}


char buf[128];
uint8_t ci=0;

char recFileName[16];
File recFile;
uint8_t recording=0;
int sample=0;

void saveAndReopen() {
  recFile.close();
  recFile=SD.open(recFileName, FILE_WRITE);
  if(!recFile) {
    err(RECPIN,500);
  }
}

void startRecord() {
  recording=1;
  sample=0;
  ci=0;

  for(uint8_t i=0; i < 256; i++) {

    snprintf(recFileName,15,"%.2X.rmc", i);

    if(!SD.exists(recFileName)) {
      break;
    }
    
    if(i==255) {
      err(FOUNDPIN,100);
    }
  }

  recFile=SD.open(recFileName, FILE_WRITE);

  if(!recFile) {
    err(SRCHPIN, 100);
  }

  digitalWrite(FOUNDPIN, HIGH);
  digitalWrite(SRCHPIN, HIGH);
  digitalWrite(RECPIN, LOW);

  
  while(Serial1.available()>0) {
    Serial1.read();
  }
  setFast();
}

void stopRecord() {
  digitalWrite(FOUNDPIN, HIGH);
  digitalWrite(SRCHPIN, HIGH);
  digitalWrite(RECPIN, HIGH);
  setSlow();
  recording=0;
  recFile.close();

}

void loop() {

 // Serial.println(digitalRead(BTNPIN));

  //Button
  if( digitalRead(BTNPIN) ) {
    //Serial.println("Btn push if");
    delay(500);
    while(digitalRead(BTNPIN)) {
      //Serial.println("Btn push while");
      delay(1000);
    }

    if(recording) {
      Serial.println("Stop record");
      stopRecord();
    } else {
      Serial.println("Start record");
      startRecord();
    }
  }


  //GPS stuff
  while( Serial1.available() > 0 ) {
    char c=Serial1.read();
    
    if(c==13) {
      //Just skip it (I guess this can't be optimized out, right?)
    } else if(c==10) {

      buf[ci]='|';
      buf[ci+1]=0;
        
      //Got message, let's see if it's valid:
      //$GPRMC,,V,,,,,,,,,,N*53
      //$GPRMC,224439.00,V,,,,,,,120718,,,N*7A
      //$GPRMC,165328.00,A,5707.38417,N,00937.64974,E,0.041,,120718,,,A*7A
      if(!recording && ci>17) {
        if(buf[17]=='A') {
          digitalWrite(SRCHPIN,HIGH );
          digitalWrite(FOUNDPIN,LOW);
        } else {
          digitalWrite(FOUNDPIN,HIGH);
          digitalWrite(SRCHPIN,LOW );
        }
      } else if(recording && ci > 17) {
        if(buf[3]=='R' && buf[4]=='M' && buf[5]=='C') {
          recFile.write(buf+7, ci+1-7);

          sample++;
          if(sample==600) {
            sample=0;
            saveAndReopen();
          }
        }
      }
      ci=0;    

    } else {
      buf[ci]=c;
      ci++;
    }
  }
}

