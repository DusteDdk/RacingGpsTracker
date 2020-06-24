// This is ported back to the prototype hardware: different pin usage, only has two LEDs, and has a pole switch instead of a momentary pushbutton.

/*
 * Error codes, interval length in ms.
 * 
 * LEDs alternate:
 * 100 - no more file names
 * 500 - saveAndOpen: couldn't open file for writing
 * 1000 - startRecord: couldn't open file for writing
 * 3000 - sd card init error
 * 
 * Both LEDs at same time:
 * 100 - record button engaged on boot
 */

#include <SPI.h>
#include <SD.h>


const int SDCSPIN = 10;

const int SRCHPIN = 9;
const int RECPIN = 6;

const int BTNPIN = 7;



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



void setup() {
  // put your setup code here, to run once:
  pinMode(SDCSPIN, OUTPUT);

  pinMode(SRCHPIN, OUTPUT);
  digitalWrite(SRCHPIN, LOW);


  pinMode(RECPIN, OUTPUT);
  digitalWrite(RECPIN, HIGH);
  
  pinMode(BTNPIN, INPUT);
  digitalWrite(BTNPIN, HIGH);

  while( digitalRead( BTNPIN ) == 1 )
  {
    digitalWrite(SRCHPIN, LOW);
    digitalWrite(RECPIN, LOW);
    delay(100);  
    digitalWrite(SRCHPIN, HIGH);
    digitalWrite(RECPIN, HIGH);
    delay(100);
  }


  if (!SD.begin(SDCSPIN)) {
    err(3000);
  }


  delay(50);


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


    digitalWrite(SRCHPIN, LOW);
    delay(1000);  
    digitalWrite(RECPIN, LOW);
    delay(1000);

    
    digitalWrite(SRCHPIN, HIGH);
    delay(1000);  
    digitalWrite(RECPIN, HIGH);
    delay(500);

}

void err(int d) {
    // Enter infinite loop.
    while(1) {
      digitalWrite(SRCHPIN, LOW);
      digitalWrite(RECPIN, HIGH);
      delay(d);
      digitalWrite(SRCHPIN, HIGH);
      digitalWrite(RECPIN, LOW);
      delay(d);
    }
}


char buf[128];
uint8_t ci=0;
int fileNum=0;
int subFileNum=0;

char recFileName[16];
File recFile;
uint8_t recording=0;
int sample=0;

void saveAndReopen() {
  subFileNum++;
  snprintf(recFileName, 15,"%.2X.%.3X", fileNum, subFileNum);

  recFile.close();  
  recFile=SD.open(recFileName, FILE_WRITE);
  if(!recFile) {
    err(500);
  }
}

void startRecord() {
  recording=1;
  sample=0;
  ci=0;

  digitalWrite(RECPIN, LOW);


  for(uint8_t i=0; i < 256; i++) {

    snprintf(recFileName,15,"%.2X.000", i);
  
    if(!SD.exists(recFileName)) {
      fileNum=i;
      subFileNum=0;
      break;
    }
    
    if(i==255) {
      err(100);
    }
  }

  recFile=SD.open(recFileName, FILE_WRITE);

  if(!recFile) {
    err(1000);
  }


  
  while(Serial1.available()>0) {
    Serial1.read();
  }
  setFast();
}

void stopRecord() {
  digitalWrite(RECPIN, HIGH);
  setSlow();
  recording=0;
  recFile.close();

}

bool recPinState=0;

void loop() {

 

  //Button
  if( digitalRead(BTNPIN) ) {
    //Serial.println("Btn push if");
    if(!recording) {
      delay(100);
      if(digitalRead(BTNPIN)) {
        startRecord();
      }
    }
  } else {
    if(recording) {
      delay(100);
      if(!digitalRead(BTNPIN)) {
        stopRecord();
      }
      
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
        } else {
          digitalWrite(SRCHPIN,LOW );
        }
          digitalWrite(RECPIN, recPinState);
          recPinState=!recPinState;

      } else if(recording && ci > 17) {
        if(buf[3]=='R' && buf[4]=='M' && buf[5]=='C') {
          recFile.write(buf+7, ci+1-7);

          sample++;
          if(sample==600) {
            sample=0;
            saveAndReopen();
          }
          
          digitalWrite(RECPIN, recPinState);
          recPinState=!recPinState;
        }
      }
      ci=0;    

    } else {
      buf[ci]=c;
      ci++;
    }
  }
}
