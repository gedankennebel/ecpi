#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>


#include <wiringPi.h>

#include "modbus.h"
#include "energycampi.h"

//includes von Projektwerkstatt
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

unsigned int m_dwPort=0;
unsigned int m_dwBaud= 115200;
modbus_t* m_ctx=NULL;


#ifndef TRUE
  #define	TRUE	(1==1)
  #define	FALSE	(1==2)
#endif

#ifndef MAX
  #define MAX(x,y) ((x>y) ? x:y)
  #define MIN(x,y) ((x>y) ? y:x)
#endif 

#define _MAX_PATH 275
#define  XSIZE 320
#define  YSIZE 80


//colorcoding

#define PRINTF_BRIGHT 1

#define PRINTF_BLACK 	0
#define PRINTF_RED		1
#define PRINTF_GREEN 	2
#define PRINTF_YELLOW 	3
#define PRINTF_BLUE 	4
#define PRINTF_MAGENTA  5
#define PRINTF_CYAN 	6
#define PRINTF_WHITE 	7



void Colour(int8_t c,bool cr) {
	printf("%c[%dm",0x1B,(c>0) ? (30+c) : c);
	if(cr) printf("\n");	
}





//open serial port
bool EnergyCamOpen(unsigned int dwPort ){
  m_dwPort=dwPort;
  char comDeviceName[100];
      
  sprintf(comDeviceName, "/dev/ttyUSB%d", dwPort);
  m_ctx = (modbus_t* )modbus_new_rtu(comDeviceName, m_dwBaud, 'E', 8, 1); // parity 'E'ven used by EnergyCam's freemodbus implementation
    
  if(NULL == m_ctx)
    return false;

  modbus_set_debug( m_ctx, false);
  modbus_set_slave( m_ctx, 1);

  if (modbus_connect(m_ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));

    modbus_free(m_ctx);
    m_ctx = NULL;
    return false; 
  } 
  else{
    printf,( "Connection OK: \n");
  }

  return true;
}


//free resources
bool EnergyCamClose(void){
  if(NULL != m_ctx){
    modbus_close(m_ctx);
    modbus_free(m_ctx);
    m_ctx = NULL;
  }
return true;    
}


//read manufacture ID
int EnergyCam_GetManufacturerIdentification(uint16_t* pData){
  uint32_t readRegCnt;

  const uint32_t regCnt = 1;
  uint16_t inputRegs[regCnt];

  if(NULL == pData) 
    return MODBUSERROR;

  readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_COMMON_INPUTREG_MEMMAP_MANUFACTURERIDENTIFICATION), regCnt, &inputRegs[0]);
  if (readRegCnt != -1) {
      *pData= inputRegs[0];
      return MODBUSOK;
  }
  else {
     //fprintf(stderr,"EnergyCamHost_GetManufacturerIdentification  failed \r\n");
     return MODBUSERROR;
  }    
 return MODBUSERROR; 
}	

//read Buildnumber of Firmware
int EnergyCam_GetAppFirmwareBuildNumber(uint32_t* pBuildNumber) {
    uint32_t readRegCnt;
    const uint32_t regCnt = 2;
    uint16_t inputRegs[regCnt];

    if(NULL == pBuildNumber) 
      return MODBUSERROR;

    readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_COMMON_INPUTREG_MEMMAP_APPBUILDNUMBER1), regCnt, &inputRegs[0]);
    if (readRegCnt != -1) {
        uint32_t tmp = 0;
        tmp  = ((uint32_t)inputRegs[0]) << 16;
        tmp |=            inputRegs[1];
        *pBuildNumber = tmp;
        return MODBUSOK;
    }
    else {
	     fprintf(stderr,"EnergyCam_GetAppFirmwareBuildNumber  failed \r\n");
       return MODBUSERROR;
    }    
 return MODBUSERROR; 
}

//read Result of Installation
int EnergyCam_GetResultOCRInstallation(uint16_t* pData) {
    uint32_t readRegCnt;

    const uint32_t regCnt = 1;
    uint16_t inputRegs[regCnt];
    
    if(NULL == pData) 
      return MODBUSERROR;

    readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_INPUTREG_MEMMAP_RESULTINSTALLATION), regCnt, &inputRegs[0]);
    if (readRegCnt != -1) {
        *pData = inputRegs[0];
         
        return MODBUSOK;
    } else {
	    //fprintf(stderr,"EnergyCam_GetResultOCRInstallation  failed \r\n");
        return MODBUSERROR;
    }    
  return MODBUSERROR; 
}

//Trigger a new Reading
int EnergyCam_TriggerReading(void) {
    uint32_t wroteRegCnt;
    const uint32_t regCnt = 1;
    uint16_t holdingRegs[1] = {1};

    wroteRegCnt = modbus_write_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_HOLDINGREG_MEMMAP_ACTIONOCR), regCnt, &holdingRegs[0]);
    if (wroteRegCnt == -1) {
        fprintf(stderr, "EnergyCam_TriggerReading failed with '%s'\n", modbus_strerror(errno));
    return MODBUSERROR;
    } else {
      fprintf(stdout, "TriggerReading \n");
      return MODBUSOK;
    }
  return MODBUSERROR;     
}

int EnergyCam_TriggerInstallation(void) {
    uint32_t wroteRegCnt;
    const uint32_t regCnt = 2;
    uint16_t holdingRegs[2] = {100,1};

    wroteRegCnt = modbus_write_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_HOLDINGREG_MEMMAP_ACTIONOCRINSTALLATIONTO), regCnt, &holdingRegs[0]);
    if (wroteRegCnt == -1) {
        fprintf(stderr, "TriggerInstallation failed with '%s'\n", modbus_strerror(errno));
    return MODBUSERROR;
    } else {
      fprintf(stdout, "TriggerInstallation \n");
      return MODBUSOK;
    }
  return MODBUSERROR;     
}

//Read the Status of the Reading
int EnergyCam_GetStatusReading(uint16_t* pStatus) {
  uint32_t readRegCnt;
  const uint32_t regCnt = 3;
  uint16_t inputRegs[regCnt];

  if(NULL == pStatus) return MODBUSERROR;

  readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_INPUTREG_MEMMAP_RESULTOCRVALID), regCnt, &inputRegs[0]);
  if (readRegCnt != -1) {
    *pStatus = inputRegs[0];
    return MODBUSOK;
  } else {
   fprintf(stderr,"EnergyCam_GetStatusReading  failed \r\n");
   return MODBUSERROR;
  }    
return MODBUSERROR; 
}

//Read OCR Result
int EnergyCam_GetResultOCRInt( uint32_t* pInt, uint16_t* pFrac) {
  uint32_t readRegCnt;
  const uint32_t regCnt = 3;
  uint16_t inputRegs[regCnt];

  if(NULL == pInt) return MODBUSERROR;
  if(NULL == pFrac) return MODBUSERROR;

  readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_INPUTREG_MEMMAP_RESULTOCRINTHIGH), regCnt, &inputRegs[0]);
  if (readRegCnt != -1) {
    uint32_t tmp = 0;
    tmp  = ((uint32_t)inputRegs[0]) << 16;
    tmp |=            inputRegs[1];
    *pInt = tmp;
    *pFrac = inputRegs[2];
    return MODBUSOK;
  } else {
      fprintf(stderr,"EnergyCam_GetResultOCRInt  failed \r\n");
        return MODBUSERROR;
    }    
   return MODBUSERROR; 
}

//Read number of pictures done by OCR
int EnergyCam_GetOCRPicDone(uint16_t* pCount) {
  uint32_t readRegCnt;
  const uint32_t regCnt = 1;
  uint16_t inputRegs[regCnt];

  if(NULL == pCount) return MODBUSERROR;


  readRegCnt = modbus_read_input_registers(m_ctx, MODBUS_GET_INTERNAL_ADDR_FROM_OFFICIAL(MODBUS_SLAVE_INPUTREG_MEMMAP_OCRPICDONE), regCnt, &inputRegs[0]);
  if (readRegCnt != -1) {
    *pCount = inputRegs[0];
    return MODBUSOK;
  } else {
      fprintf(stderr,"EnergyCam_GetOCRPicDone  failed \r\n");
      return MODBUSERROR;
    }    
   return MODBUSERROR; 
}

//Log Reading with date info to CSV File
int EnergyCam_Log2CSVFile(const char *path,	 uint32_t Int, uint16_t Frac)
{
  FILE    *hFile;
  uint32_t FileSize = 0;

  char  CurrentTime[250];

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);	

	if ( (hFile = fopen(path, "rb")) != NULL ) {
		fseek(hFile, 0L, SEEK_END);
		FileSize = ftell(hFile);
		fseek(hFile, 0L, SEEK_SET);
		fclose(hFile);
	}
	else MODBUSERROR; 
	
	if ( (hFile = fopen(path, "a")) != NULL ) {
		  if(FileSize == 0)  //start a new file with Header
				fprintf(hFile, "Date, Value \n");
			fprintf(hFile,"%d-%02d-%02d %02d:%02d, %d.%d\r\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min, Int,Frac);
			fclose(hFile);
		}
	else MODBUSERROR; 
		
 return MODBUSOK; 
}
	

uint16_t DisplayInstallationStatus()
{
	uint16_t Data = 0;	
	if (MODBUSOK == EnergyCam_GetResultOCRInstallation(&Data)) {
       switch(Data){
		   case INSTALLATION_FAILED:  	Colour(PRINTF_RED,false);printf("Installation failed");Colour(0,true);
										break;
		   case INSTALLATION_NODIGITS:
		   case INSTALLATION_NOTDONE:   Colour(PRINTF_RED,false);printf("EnergyCAM not installed");Colour(0,true); 
										break;
		   case INSTALLATION_ONGOING:   Colour(PRINTF_YELLOW,false);printf("Installation ongoing");Colour(0,true);
										break;
		   default :    				Colour(PRINTF_GREEN,false);printf("Installed with %d digits",(Data >>8));Colour(0,true);
										break;		
		}
	}
	
	return Data;
}


int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}



static int iTime;
static int iMinute;

int IsNewSecond(int iS)
{
	int CurTime;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	CurTime = tm.tm_hour*60*60+tm.tm_min*60+tm.tm_sec;
	if(iS > 0)
	CurTime = CurTime/iS;


	if(CurTime != iTime){
		iTime = CurTime;
		return 1;
	}				
return(0);
}

int IsNewMinute(void)
{
	int CurTime;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	CurTime = tm.tm_hour*60+tm.tm_min;

	if(CurTime != iMinute){
		iMinute = CurTime;
		return 1;
	}
			
return(0);
}

void Intro(int Read)
{
	printf("   \n");
	Colour(62,false);	
	printf("############################################\r\n");
	printf("## ecpi - EnergyCam on raspberry Pi/Weezy ##\r\n");	
	printf("############################################\r\n");

	Colour(0,true);
	printf("   Usage\n");
	printf("   q   : Quit\n");
	printf("   R   : Trigger Reading\n");
	printf("   r   : read Value\n");
	printf("   Reading is triggered every %d minutes \n",Read);	
	printf("   \n");  	  
   
}


void ErrorAndExit(const char *info)
{
	Colour(PRINTF_RED,false);
	printf("%s",info);
	Colour(0,true);	

	EnergyCamClose();
			
	exit(0);
}

//////////////////////////////////////////////
int main(int argc, char *argv[]) 
{ 

	//Projektwerkstatt
	if(argc != 4) {
		ErrorAndExit("Illegal number of Parameters");
	}
	//Projektwerkstatt

	int key=0;
	int  Reading;
	char OCR[20];
	char Version[20];
		
	int iReadRequest=0;

	int ReadingPeriod=10;
	int ReadingTimer=ReadingPeriod+1;

	uint16_t Data = 0;
	uint32_t Build = 0;
	uint32_t OCRData = 0;

	int iRetry = 3;
	int iTimeout = 0;

	int sock;
	int connected;

	Intro(ReadingPeriod);
	 
	if (wiringPiSetup () == -1) {
		fprintf (stderr, "Not running on raspberry pi - now ending\n") ;
		exit(0);
	}
			
	EnergyCamOpen(0);  //open serial port

	//Projektwerkstatt
	iRetry = 3;
	do{
		sock = socket( AF_INET, SOCK_STREAM, 0 );
		if(sock != -1) break;
	}while(iRetry-- < 0);

	if(sock != -1) {
		Colour(PRINTF_GREEN,false);
		printf("Set up Socket");
		Colour(0,true);
	} else {
		ErrorAndExit("Troubles setting up Socket");
	}

	struct sockaddr_in server;
	unsigned long addr;

	memset( &server, 0, sizeof (server));

	addr = inet_addr( argv[1] );
	memcpy( (char *)&server.sin_addr, &addr, sizeof(addr));
	server.sin_family = AF_INET;
	server.sin_port = htons( atoi( argv[2]) );

	if(sock != -1)
	{
		// connect to Server
		iRetry = 3;
		do
		{
			connected = connect(sock,(struct sockaddr*)&server, sizeof(server));
			if(connected != -1) break;
		}while(iRetry-- > 0);
	}

	if(connected != -1) {
		Colour(PRINTF_GREEN,false);
		printf("Connected to Server ");
		Colour(0,true);
	} else {
		ErrorAndExit("No Connection to Server");
	}

	//Projektwerkstatt

	//get Status & wakeup
	iRetry = 3;
	do {
	if(iRetry-- < 0 ) break;
	}while(MODBUSERROR == EnergyCam_GetManufacturerIdentification(&Data));

	if(Data == SAIDENTIFIER) {
		Colour(PRINTF_GREEN,false);
		printf("EnergyCAM Sensor connected ");
		Colour(0,true);
	} else {
		ErrorAndExit("EnergyCAM not found ");
	}
	
		//Read Buildnumber
	if (MODBUSOK == EnergyCam_GetAppFirmwareBuildNumber(&Build)) {
	  printf("Build %d \n",Build);
	}
	
	//Check Buildnumber, GetResultOCRInt requires Build 8374
	if (Build < 8374) {
	  ErrorAndExit("This App requires a Firmwareversion >= 8374. ");
	}

	//Is EnergyCam installed
	Data = DisplayInstallationStatus();

	//try to install the device if not installed
	if((Data == INSTALLATION_NODIGITS) || (Data == INSTALLATION_NOTDONE)){
		EnergyCam_TriggerInstallation();
		usleep(2000*1000);   //sleep 2000ms - wait for Installation
		printf("Installing ");
		iTimeout = 20;
		do {
			usleep(500*1000);   //sleep 500ms
			printf(".");
			if (MODBUSERROR == EnergyCam_GetResultOCRInstallation(&Data)) {
				Data = 0xFFFD; //retry if MODBUS returns with an Error
			}
		}
		while((iTimeout-->0) && (Data == 0xFFFD));
		printf("\n");
		
		//Is EnergyCam installed
		Data = DisplayInstallationStatus();	
	}
	
	if((Data == INSTALLATION_NODIGITS) || (Data == INSTALLATION_NOTDONE) || (Data == INSTALLATION_FAILED) || (Data == INSTALLATION_ONGOING)){
		ErrorAndExit("EnergyCAM not installed ");
	}	


	

	//get last Reading
	if (MODBUSOK == EnergyCam_GetResultOCRInt(&OCRData,&Data)) {
	  time_t t = time(NULL);
	  struct tm tm = *localtime(&t);
			
	  printf("(%02d:%02d:%02d) Reading %04d.%d \n",tm.tm_hour,tm.tm_min,tm.tm_sec,OCRData,Data);
	  //Projektwerkstatt
	  int OffsetHours = tm.tm_gmtoff/3600;
	  int year = tm.tm_year + 1900;
	  int mon = tm.tm_mon + 1;
	  unsigned int OffsetMin = tm.tm_gmtoff/60 - OffsetHours*60;
	  char request[200];
	  sprintf(request,"POST /meterValue?date=%04d-%02d-%02dT%02d:%02d:%02d.000-%02d:%02d&value=%d HTTP/1.1\r\nHost: %s\r\nAuthorization: Basic %s\r\n\r\n", year, mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, OffsetHours, OffsetMin,OCRData,argv[1],argv[3]);
	  send(sock,request,strlen(request),0);
	  //Projektwerkstatt
	  EnergyCam_Log2CSVFile("/var/www/ecpi/data/ecpi.csv",OCRData,Data);
	}
		
	IsNewMinute();
	ReadingTimer=ReadingPeriod+1;
		
	while (!((key == 0x1B) || (key == 'q')))
	{
		usleep(500*1000);   //sleep 500ms
			
		key = getkey();
		
		if(key == 'r')   {
		  iReadRequest++; //Read now
		}
		if(key == 'R')   {
			ReadingTimer=1;  //read in 1 minute
			
		  //get Status & wakeup
			iRetry = 3;
			do {
				if(iRetry-- < 0 ) break;
			}while(MODBUSERROR == EnergyCam_GetStatusReading(&Data));
			printf("GetStatusReading %04X \n",Data);	
			
			//trigger new reading
			EnergyCam_TriggerReading();		  
		}			
		
		
		if(IsNewMinute()){
		  if(--ReadingTimer<=1)iReadRequest++;
		  printf("%02d ",ReadingTimer);
		}
		
		if(iReadRequest > 0) {
		  iReadRequest=0;
		  printf("%02d \n",ReadingTimer);	
		  ReadingTimer=ReadingPeriod+1;
			
		  //get Status & wakeup
	  	  iRetry = 3;
		  do {
		    if(iRetry-- < 0 ) break;
		  }while(MODBUSERROR == EnergyCam_GetStatusReading(&Data));
		    printf("GetStatusReading %04X \n",Data);
		    EnergyCam_GetOCRPicDone(&Data);
		    printf("Pictures %04d \n",Data);

		    if (MODBUSOK == EnergyCam_GetResultOCRInt(&OCRData,&Data)) {
		    time_t t = time(NULL);
		    struct tm tm = *localtime(&t);
	    	    //Projektwerkstatt
		    printf("(%02d:%02d:%02d) Reading %04d.%d \n",tm.tm_hour,tm.tm_min,tm.tm_sec,OCRData,Data);
	 	    int OffsetHours = tm.tm_gmtoff/3600;
		    int year = tm.tm_year + 1900;
		    int mon = tm.tm_mon + 1;
		    unsigned int OffsetMin = tm.tm_gmtoff/60 - OffsetHours*60;
		    char request[200];
		    sprintf(request,"POST /meterValue?date=%04d-%02d-%02dT%02d:%02d:%02d.000-%02d:%02d&value=%d HTTP/1.1\r\nHost: %s\r\nAuthorization: Basic %s\r\n\r\n", year, mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, OffsetHours, OffsetMin,OCRData,argv[1],argv[3]);
		    send(sock,request,strlen(request),0);
		    //Projektwerkstatt
		    EnergyCam_Log2CSVFile("/var/www/ecpi/data/ecpi.csv",OCRData,Data);
		  }	
		}
	
	} // end while
	
	EnergyCamClose();

	return 0;
}
