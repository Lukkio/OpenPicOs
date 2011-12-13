////////////////////////////////////////////////////////////////////////
/////////////////// FAT32 DRIVER FOR MMC OVER SPI //////////////////////
////////////////////////////////////////////////////////////////////////
//// Original code by Tamas Bodorics (Tomi),                        ////
//// http://www.ccsinfo.com/forum/viewtopic.php?t=23969             ////
//// Modified by Mattias "miniman" Eriksson, Sweden                 ////
//// Disclaimer:                                                    ////
//// This code is provided for you "AS IS", this means I can NOT be ////
//// held responsible for any damage or harm it may couse.          ////
////////////////////////////////////////////////////////////////////////
//// Hardware:                                                      ////
////   This is the hardware I use. My pic is running at 5v, the MMC ////
////   at 3.3V, so I need voltage level conversion. That is what    ////
////   the resistors and zener diodes are for. First a 820 ohm      ////
////   resistor from is connected to the pic, then a 3.3V           ////
////   zenerdiode is connected with its anode to ground and its     ////
////   cathode to the resistor. This resistor-zenerdiode connection ////
////   leaves about 3V on the DATA/CS/CLOCK lines which is enough.  ////
////   A 100K pull-up resistor from Data out(Do) of the MMC is      ////
////   required. Note: this "picture" do not have the right pin     ////
////   assignments. Hope you can see what I'm trying to paint..     ////
////                                                                ////
////    __ __  3x820 ohm                       ______________       ////
////   |  U  |--\/\/\----------- o---DATA->---|=2            |      ////
////   | PIC |--\/\/\-------o----+---CS--->---|=1            |      ////
////   |     |--\/\/\--o----+----+---CLOCK>---|=5    MMC     |      ////
////   |     |        _|_, _|_, _|_,       o--|=7            |      ////
////   |     |       '/_\ '/_\ '/_\        |   \_____________|      ////
////   |     |     GND_|____|____|_GND     | 100k ohm               ////
////   |_____|-----------<-DATA-<----------o--\/\/\---<3.3V         ////
////                                                                ////
//// Notes:                                                         ////
//// - The card that is going to be used by this code has to be     ////
////   pre-formated with FAT32, 512byte/sector. To do this, run cmd ////
////   and type the following command:                              ////
////   format X: /A:512 /V:LABEL /FS:FAT32                          ////
////   where X is the diverletter of your card and LABEL is the     ////
////   volyme name. This can also be done in the diskmanager        ////
//// - Use '/' as directory separator, example: "MYDIR/ELEMENT.WND" ////
////   "MYDIR/SUBDIR/OTHER.TXT" etc.                                ////
////                                                                ////
//// Functions:                                                     ////
////   MMCInit()                                                    ////
////      Initializes the MMC, this function must be called before  ////
////      any other MMC functions, and at the top of the program.   ////
////      Returns a MMCResponse.                                    ////
////   InitFAT()                                                    ////
////      Initializes the FAT filesystem, this function must be     ////
////      called before any other MMC functions and after MMCInit,  ////
////      only if the later one returned MMC_OK.                    ////
////      Returns a MMCResponse.                                    ////
////   InitList(char *path)                                         ////
////      Initializes file and directory listing in the any         ////
////      directory that is specified by path. To list files and    ////
////      directorys in root directory path should be a empty       ////
////      string(""). Ottherwise path should end with a "/" e.g:    ////
////      "DIR/SUBDIR/". Returns a filehandle.                      ////
////   Listfiles(char f)                                            ////
////      List files and directorys in any directory that have been ////
////      openes by InitList(). Access the list through FileList    ////
////      array. See examples. There can only be one file handle    ////
////      open for file listing at the same time. Returns the       ////
////      number of files and directorys that where listed.         ////
////   NextPage(char f)                                             ////
////      To be able to handle many files in a dir, file listing    ////
////      is parted up in pages. To list next page, call NextPage   ////
////      and then ListFiles again. ListFiles must be called        ////
////      before a NextPage function, otherwise the function will   ////
////      not have any effect. Returns a MMCResponse.               ////
////   SetPage(char f)                                              ////
////      Set the page that you want to list, see above function    ////
////      to get better understanding. Note: no need to call        ////
////      ListFiles before this function. Returns a MMCResponse.    ////
////   CloseList(char f)                                            ////
////      Closes filelisting, in other words, the there are an      ////
////      extra free file handle to use. Returns a MMCResponse      ////
////   FreeList()                                                   ////
////      Frees all the strings in FileList array. This function    ////
////      is called before any new files are listed in ListFiles    ////
////      but not in CloseList. THis means you can access the last  ////
////      listed files even though you already have closed the      ////
////      list. But to free the mem that are used by FileList call  ////
////      this function.                                            ////
////   fopen(char *fname, char mode)                                ////
////      Opens a file specified by fname in the desired mode.      ////
////      Valid modes are: 'r' = read, 'w' = write and 'a' = append ////
////      No long filenames can be used. To open a file with long   ////
////      filename, listed by ListFiles(), use the shortName field  ////
////      in the FileList, FileList[i].shortName. Note: ListFiles   ////
////      do only filename, not the path..                          ////
////      Returns a filehandle if succeed, else a errorcode         ////
////   fclose(char f)                                               ////
////      Closes a file that previously have been opened by fopen.  ////
////      Returns a MMCResponse                                     ////
////   fputch(char be, char f)                                      ////
////      Puts a char, be, in file f that previously have been      ////
////      opened by fopen. Returns a MMCResponse                    ////
////   fgetch(char *ki, char f)                                     ////
////      get a char from file f that previously have been opened   ////
////      by , and puts it at the position specified by ki     ////
////      Returns a MMCSresponse                                    ////
////   fputstring(char *be, char f)                                 ////
////      puts a entire string to file f that previously have been  ////
////      opened by fopen. Returns MMCResponse                      ////
////   fread(char *buffer, int16 leng, char f)                      ////
////      Reads a block of data with length leng from file f that   ////
////      previously have been opened by fopen.                     ////
////      Returns the number of bytes that were read.               ////
////   fwrite(char *buffer, int16 leng, char f)                     ////
////      Writes a block of data with length leng to a file f that  ////
////      previously have been opened by fopen.                     ////
////      Returns a MMCResponse                                     ////
////   getfsize(char *fname, int32 *fsiz)                           ////
////      Reads the size of a file with a filename specified by     ////
////      fname. Returns a MMCResponse.                             ////
////                                                                ////
//// Examples:                                                      ////
////   // This example open a file for append, and writes "System   ////
////   // started". It also shows small error handling              ////
////   char f,gfilename[12];                                        ////
////   strcpy(gfilename,"EVENTS.LOG");                              ////
////   f = fopen(gfilename,'a'); // open EVENTS.LOG for append      ////
////   if (f & MMC_ERROR)                                           ////
////   {                                                            ////
////      printf("Couldn't open file!\r\n");                        ////
////      if(f == MMC_NO_CARD_INSERTED)                             ////
////         printf("Please, insert MMC!");                         ////
////      else if(f == MMC_MAX_FILES_REACHED)                       ////
////         printf("ops.. =)");                                    ////
////   }                                                            ////
////   else                                                         ////
////   {                                                            ////
////      strcpy(msg,"System started\r\n");                         ////
////      fputstring(msg,f);                                        ////
////      fclose(f);                                                ////
////   }                                                            ////
////                                                                ////
////   // Here is a exampel that covers almost everything...        ////
////   // It first lists all files and dirs in DIR/ and then open   ////
////   // The first and prints the file with printf, take a look:   ////
////   #include <MMC_SPI_FAT32.h> // As usual you also need to      ////
////   #include <MMC_SPI_FAT32.c> // include your device .h file... ////
////   void main(void)                                              ////
////   {                                                            ////
////      char f,filename[20],res,i,c;                              ////
////      while(TRUE)                                               ////
////      {                                                         ////
////         if(MMCInit() == MMC_OK)                                ////
////         {                                                      ////
////            printf("MMC initialized\r\n");                      ////
////            InitFAT();                                          ////
////            strcpy(filename,"DIR/");                            ////
////            f = InitList(filename);                             ////
////            if(f & MMC_ERROR)                                   ////
////               printf("Error");                                 ////
////            else                                                ////
////            {                                                   ////
////               do                                               ////
////               {                                                ////
////                  res = ListFiles(f);                           ////
////                  for(i=0;i<res;i++)                            ////
////                  {                                             ////
////                     printf(FileList[i].name);                  ////
////                     if(FileList[i].isDir)                      ////
////                        printf("\\\r\n");//a "\" with row break ////
////                     else                                       ////
////                        printf("\r\n");                         ////
////                  }                                             ////
////               } while(NextPage(f) == MMC_OK);                  ////
////               CloseList(f); // frees the file, but the list is ////
////            }                // still there...                  ////
////            strcpy(filename,FileList[0].shortName);             ////
////            f = open(filename,'r'); // open file for reading    ////
////            if((f & MMC_ERROR) == 0) // No error, same as       ////
////            {                        // if(f < MAXFILES)        ////
////               while(fgetch(&c,f) == MMC_OK)                    ////
////                  printf(c);                                    ////
////               fclose(f);                                       ////
////            }                                                   ////
////            else if(f == MMC_NOT_FOUND)                         ////
////               printf("1st file in list was probebly a dir");   ////
////            else                                                ////
////               printf("Other error\r\n");                       ////
////            while(TRUE)                                         ////
////               ; // Loop forever, program is finished           ////
////         }                                                      ////
////         else                                                   ////
////            printf("MMC init failed!\r\n");                     ////
////         delay_ms(1000);                                        ////
////         printf("Trying once more..\r\n");                      ////
////      }                                                         ////
////   }                                                            ////
////                                                                ////
////////////////////////////////////////////////////////////////////////
//// Changelog:                                                     ////
//// YYYY-MM-DD VER LOG                                             ////
//// 2007-08-15 0.9 Fixed a hardware SPI error in writesector.      ////
////                Thanks to wielen who found the bug.             ////
//// 2007-07-25 0.8 This, and almost all other text is written      ////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// If you want to use software SPI keep this line uncommented
// If you want to use hardware SPI comment this line..
// Software SPI are slower, but you can use any clock and data
// pins that you want..
#define MMC_SPI_SOFTWARE


// Change the port to whatever port you are going to use
#use FAST_IO(B)
//#use FAST_IO(C)
// Change this to set the right tris for your pins
#define SET_MMC_TRIS() set_tris_b(0b10000000) // 0b0001000 1=input,0=output
// Change these pins to whatever pins you are using
#define ChipSel      pin_B4 // Chip-Select pin
#define ChipClk      pin_B5 // SPI-Clock pin
#define ChipDout   pin_B6 // SPI-Data out pin
//#define ChipDin      pin_B0 // SPI-Data in pin
#define ChipDin      pin_B7
#ifdef MMC_SPI_SOFTWARE
   // You can also specify a baud-rate in the line below, if not the fastest possible speed will be used.
   // For me that was 800kHz on a 18F4550 with 48MHz external clock
   #USE SPI(MASTER, SAMPLE_RISE, MSB_FIRST, IDLE=1, DI=ChipDin, DO=ChipDout, CLK=ChipClk, BITS=8, STREAM=MMC_SPI)
   #define MMC_Xfer(b) SPI_Xfer(MMC_SPI,b)
#else
   // If MMC_SPI_SOFTWARE isn't defined (se above) these variables are used, you should check if they
   // matches your target device, but for most pic's they should do
   #byte SSPBUF   = 0xFC9
   #byte SSPSTAT   = 0xFC7
   #byte SSPCON1   = 0xFC6
   #bit  BF      = SSPSTAT.0
#endif

#ifdef ENABLE_FILELISTNG
   #include <STDLIBM.H> // used for malloc and free functions
#endif

int32 FATTable[128];
int32 gFirstEmptyCluster;

FAT32Vars gFAT32Vars;
diskinforec DiskInfo;
FILE gFiles[MAXFILES];

// Time stores the date and time, this is used when writing and/or creating files
TimeRecord Time;

#ifdef ENABLE_FILELISTNG
   // Set the maximum number of files/dirs to be listed by Listfile()
   #define MAX_FILE_LIST 10
   // Variables used by InitList(), ListFiles(), NextPage() and SetPage()
   ListPos StartList;
   ListPos CurrentList;
   BOOLEAN changeList; // Do not use this; it is only used by SetPage() and ListFiles() and Initialized by InitFAT()
   // The result of ListFiles() function..
   LongFileName FileList[MAX_FILE_LIST];
#endif

#byte MMCAddressL = gFAT32Vars
#byte MMCAddressH = gFAT32Vars+1
#byte MMCAddressHL = gFAT32Vars+2
#byte MMCAddressHH = gFAT32Vars+3
#byte gStartSectorL = gFAT32Vars+8
#byte gStartSectorH = gFAT32Vars+9
#byte gStartSectorHL = gFAT32Vars+10

// You should check that these values matches your pic device
// but for most devices they should be correct
#define FSR0L      *0xFE9
#define POSTINC0   *0xFEE

// A bunch of error codes
#define MMC_OK                0
#define MMC_ERROR             0x80
#define MMC_INVALID_FILE       0x88
#define MMC_MAX_FILES_REACHED    0x90
#define MMC_NO_CARD_INSERTED    0x98
#define MMC_TIME_OUT         0xA0
#define MMC_INVALID_RESPONSE   0xA8
#define MMC_NOT_FOUND         0xB0
#define MMC_INVALID_CLUSTER      0xB8
#define MMC_END_OF_DIR         0xC0
#define MMC_INVALID_POSITION   0xC8

// I don't like having constants in the code... yeah, I do have some anyway ;)
#define END_OF_DIR                  17
#define DIRENTRYS_PER_SECTOR         16
#define CHARACTERS_IN_LONG_FILENAMES   13

// If you want to have a cardinserted sens-pin, define CardInserted to a input, like:
// #define CardInserted !input(PIN_XX)
// MMC Card have two GND pins, pull one up with a resistor (10K or somwthing like that) to Vcc
// and connect to desired pin
#define CardInserted() 1

// Looks a bit nicer in the code
#define MMC_Select()    output_low(ChipSel)
#define MMC_Deselect()   output_high(ChipSel)


// Below this, there should be no need to change anything


char IsSelfDir(char *be)
{
   if (be[0] == '.' && be[1] == '.')
      return 0xFF;
   else
      return 0;
}

void MMCOut(char indata)
{
  #ifdef MMC_SPI_SOFTWARE
   MMC_Xfer(indata);
  #else
   char i;
   SSPBUF=indata;
   while (!BF);
   i = SSPBUF;
  #endif
}

void MMC8Clock()
{
  #ifdef MMC_SPI_SOFTWARE
   MMC_Xfer(0xFF);
  #else
   char i;
   SSPBUF=0xFF;
   while (!BF);
   i = SSPBUF;
  #endif
}

char MMCIn()
{


  #ifdef MMC_SPI_SOFTWARE
   return MMC_Xfer(0xFF);
  #else
   char i;
   SSPBUF=0xFF;
   while (!BF);
   i = SSPBUF;
   return i;
  #endif
}

MMCResponse MMCInit()
{
   char response,iii,errcnt;
   

   if (!CardInserted())
      return MMC_NO_CARD_INSERTED;
   SET_MMC_TRIS();  // Set input/output
   restart_wdt();
   MMC_Deselect();

   #ifndef MMC_SPI_SOFTWARE
      bit_clear(SSPCON1,5); // Disables serial port and configures these pins as I/O port pins
      SSPCON1 = 0x30; // modify this number to change SPI clock rate
                     // Enables serial port and configures SCK, SDO, SDI and SS as serial port pins
                     // Idle state for clock is a high level
                     // 0000 = SPI Master mode, clock = FOSC/4

      SSPSTAT = 0;   // Only bit 6,7 are writable
                     // 0 = Input data sampled at middle of data output time
                     // 0 = Transmit occurs on transition from Idle to active clock state
   #endif

   iii = 10;
   errcnt = 100;
   do
   {
      MMCOut(0xFF);
   } while (--iii);
   delay_us(1000);


   MMC_Select(); // if the /CS line of the MMC is low during CMD0,(below), the card enters SPI-mode
   MMCOut(0x40); // CMD0
   MMCOut(0x00);
   MMCOut(0x00);
   MMCOut(0x00);
   MMCOut(0x00);
   MMCOut(0x95); // CRC
   MMC8Clock();
   response = MMCIn();
   MMC_Deselect();
   do
   {
      delay_us(1000);
      //output_low(ChipClk);
      MMC_Select();
      MMCOut(0x41); // CMD1
      MMCOut(0x00);
      MMCOut(0x00);
      MMCOut(0x00);
      MMCOut(0x00);
      MMCOut(0x01);
      MMC8Clock();
      response = MMCIn();
      MMC_Deselect();
      MMC8Clock();
      errcnt--;
   } while (response && errcnt);
   if(response == 0)
      return MMC_OK;
   else
      return MMC_TIME_OUT;
}


// "Packed" Date:
// +-------+------------+-------+-----+
// | BITS  |    15:9    |  8:5  | 4:0 |
// | VALUE |Year – 1980 | Month | Day |
// +-------+------------+-------+-----+
int16 GetCurrentDOSDate()
{
   int16 retval;

   retval = Time.Year - 1980;
   retval <<= 9;
   retval |= ((int16)Time.Month << 5);
   retval |= (int16)Time.Day;
   return retval;
}

// "Packed" Time:
// +-------+-------+--------+----------+
// | BITS  | 15:11 |  10:5  |   4:0    |
// | VALUE | Hour  | Minute | Second/2 |
// +-------+-------+--------+----------+
int16 GetCurrentDOSTime()
{
   int16 retval;

   retval = Time.Hour;
   retval <<= 11;
   retval |= ((int16)Time.Minute << 5);
   retval |= (int16)Time.Second >> 1;
   return retval;
}

// Function: Reads a sector from MMC
MMCResponse ReadSector(int32 sector, char *hova)
{
   char errs,response;
   char cnt2,cnt3;

   #byte sectorL = sector
   #byte sectorH = sector+1
   #byte sectorHL = sector+2

   if (!CardInserted())
      return MMC_NO_CARD_INSERTED;
   Disable_interrupts(GLOBAL);
   Restart_wdt();
   MMCAddressL = 0;
   MMCAddressH = sectorL;
   MMCAddressHL = sectorH;
   MMCAddressHH = sectorHL;
   gFAT32Vars.MMCAddress <<= 1;

   MMC_Select();
   MMCOut(0x51);
   MMCOut(MMCAddressHH);
   MMCOut(MMCAddressHL);
   MMCOut(MMCAddressH & 0xFE);
   MMCOut(0);
   MMCOut(0x01);
   errs = 8;
   do
   {
      response = MMCIn();
   } while (--errs && response==0xFF);
   errs = 50;
   do
   {
      response = MMCIn();
      if (response == 0xFE)
         break;
      delay_ms(1);
   } while (--errs);
   FSR0L = (int16)hova; // *0xFE9
   cnt3 = 2;
   cnt2 = 0;
   do
   {
      do
      {
         #ifdef MMC_SPI_SOFTWARE
            POSTINC0 = MMC_Xfer(0xFF); // *0xFEE
         #else
            SSPBUF=0xFF; // Writes 0xFF on SPI
            while(!BF); // Wait until Transmitted/Received
            POSTINC0 = SSPBUF; // Read the received byte and place it in adress oxFEE
         #endif
      } while (--cnt2);
   } while (--cnt3);
   response = MMCIn();
   response = MMCIn();
   MMC_Deselect();
   Enable_interrupts(GLOBAL);
   return MMC_OK;
}

// Writes a sector to MMC
MMCResponse WriteSector(int32 sector, char *honnan)
{
   char errs,response;
   char cnt2,cnt3;
   #byte sectorL = sector
   #byte sectorH = sector+1
   #byte sectorHL = sector+2

   if (!CardInserted())
      return MMC_NO_CARD_INSERTED;
   Disable_interrupts(GLOBAL);
   Restart_wdt();
   MMCAddressL = 0;
   MMCAddressH = sectorL;
   MMCAddressHL = sectorH;
   MMCAddressHH = sectorHL;
   gFAT32Vars.MMCAddress <<= 1;
   response = 0;
   //output_low(ChipClk);
   MMC_Select();
   MMCOut(0x58);
   MMCOut(MMCAddressHH);
   MMCOut(MMCAddressHL);
   MMCOut(MMCAddressH & 0xFE);
   MMCOut(0);
   MMCOut(0x01);
   MMC8Clock();
   errs = 8;
   do
   {
      response = MMCIn();
   } while (--errs && response==0xFF);
   if (response)
   {
      MMC_Deselect();
      //output_high(ChipClk);
      MMC8Clock();
      Enable_interrupts(GLOBAL);
      return MMC_INVALID_RESPONSE;
   }
   MMC8Clock();
   MMCOut(0xFE);
   FSR0L = (int16)honnan; // *0xFE9
   cnt3 = 2;
   cnt2 = 0;
   do
   {
      do
      { /*

        */
         #ifdef MMC_SPI_SOFTWARE
              MMC_Xfer(POSTINC0); // *0xFEE
         #else
            SSPBUF=POSTINC0; // Write the byte on address oxFEE to SPI
            while (!BF);
            response = SSPBUF; // thanks to wielen
         #endif
      } while (--cnt2);
   } while (--cnt3);
   MMCOut(0x00);
   MMCOut(0x01);
   response = MMCIn();
   response ^= 0xE5; // Bitwise exclusive or assignment operator, x^=y, is the same as x=x^y
   if (response)
   {
      goto endwr3;
   }
   do
   {
      response = MMCIn();
   } while (response == 0);
   response = 0;
endwr3:
   MMC_Deselect();
   //output_high(ChipClk);
   MMC8Clock();
   Enable_interrupts(GLOBAL);
   return MMC_OK;
}

// Function: Initializes the FAT system
void InitFAT()
{
   int32 actsector;
   char i;

   gFirstEmptyCluster = 0;
   gFAT32Vars.gStartSector = 0;
   ReadSector(gFAT32Vars.gStartSector,gFiles[MAXFILES-1].IOpuffer);
   if (gFiles[MAXFILES-1].IOpuffer[0] != 0xEB)
   {
      gStartSectorL = gFiles[MAXFILES-1].IOpuffer[0x1C6];
      gStartSectorH = gFiles[MAXFILES-1].IOpuffer[0x1C7];
      gStartSectorHL = gFiles[MAXFILES-1].IOpuffer[0x1C8];
      ReadSector(gFAT32Vars.gStartSector,gFiles[MAXFILES-1].IOpuffer);
   }
   memcpy(&DiskInfo,gFiles[MAXFILES-1].IOpuffer,sizeof(DiskInfo));
   actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1;
   ReadSector(actsector,FATTable);
   gFAT32Vars.FATstartidx = 0;
   gFAT32Vars.gFirstDataSector = gFAT32Vars.gStartSector + DiskInfo.FATCopies*DiskInfo.hSectorsPerFat+DiskInfo.Reserved1 - 2;

   for (i=0;i<MAXFILES;i++)
   {
      gFiles[i].Free = TRUE;
   }
   #ifdef ENABLE_FILELISTNG
      // Code for initializing file-listing
      for(i=0;i<MAX_FILE_LIST;i++)
         FileList[i].name = NULL;
      changeList = TRUE;
   #endif
//   MMC_Debug("-FATInit ");
}

int32 GetNextCluster(int32 curcluster)
{
   int32 actsector;
   int32 clpage;
   char clpos;
   clpage = curcluster >> 7;
   clpos = curcluster & 0x7F;
   if (clpage != gFAT32Vars.FATstartidx) // read in the requested page
   {
      actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + clpage;
      ReadSector(actsector,FATTable);
      gFAT32Vars.FATstartidx = clpage;
   }
   return (FATTable[clpos]);
}

void SetClusterEntry(int32 curcluster,int32 value)
{
   int32 actsector;
   int32 clpage;
   char clpos;

   clpage = curcluster >> 7;
   clpos = curcluster & 0x7F;
   actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + clpage;
   if (clpage != gFAT32Vars.FATstartidx)
   {
      ReadSector(actsector,FATTable);
      gFAT32Vars.FATstartidx = clpage;
   }
   FATTable[clpos] = value;
   WriteSector(actsector,FATTable);
   actsector += DiskInfo.hSectorsPerFat;
   WriteSector(actsector,FATTable);
}

void ClearClusterEntry(int32 curcluster)
{
   int32 actsector;
   int32 clpage;
   char clpos;

   clpage = curcluster >> 7;
   clpos = curcluster & 0x7F;
   if (clpage != gFAT32Vars.FATstartidx)
   {
      actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + gFAT32Vars.FATstartidx;
      WriteSector(actsector,FATTable);
      actsector += DiskInfo.hSectorsPerFat;
      WriteSector(actsector,FATTable);
      actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + clpage;
      ReadSector(actsector,FATTable);
      gFAT32Vars.FATstartidx = clpage;
   }
   FATTable[clpos] = 0;
}

int32 FindFirstFreeCluster()
{
   int32 i,st,actsector,retval;
   char j;

   st = gFirstEmptyCluster;
   for (i=st;i<DiskInfo.hSectorsPerFat;i++)
   {
      if (i != gFAT32Vars.FATstartidx)
      {
         actsector = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + i;
         ReadSector(actsector,FATTable);
         gFAT32Vars.FATstartidx = gFirstEmptyCluster = i;
      }
      for (j=0;j<128;j++)
      {
         if (FATTable[j] == 0)
         {
            retval = i;
            retval <<= 7;
            retval |= j;
            return retval;
         }
      }
   }
   return 0x0FFFFFFF;
}

// Function: Converts a dir-entry to a 8.3 filename
void ConvertFilename(DIR *beDir,char *name)
{
   char i,j,c;

   j = 0;
   name[0] = 0;
   for (i=0;i<8;i++)
   {
      c = beDir->sName[i];
      if (c == ' ')
         break;
      name[j++] = c;
   }
   for (i=0;i<3;i++)
   {
      c = beDir->spam[i];
      if (c == ' ' || c == 0)
         break;
      if (!i)
         name[j++] = '.';
      name[j++] = c;
   }
   name[j++] = 0;
}

// Function: Converts a dir-entry to a (part of a) long filename.
//           One dir-entry can hold 13 unicode characters
void ConvertLongFilename(DIR *beDir,char *name)
{
   char i,j,c;

   j = 0;
   name[0] = 0;
   for (i=1;i<11;i+=2)
   {
      c = (char)(*(((char*)beDir)+i));
      if (c == 0x00 || c == 0xFF)
         break;
      name[j++] = c;
   }
   if(c!=0x00 && c!= 0xFF)
   {
      for (i=14;i<26;i+=2)
      {
         c = (char)(*(((char*)beDir)+i));;
         if (c == 0 || c == 0xFF)
            break;
         name[j++] = c;
      }
      if(c!=0x00 && c != 0xFF)
      {
         for (i=28;i<31;i+=2)
         {
            c = (char)(*(((char*)beDir)+i));
            if (c == 0 || c == 0xFF)
               break;
            name[j++] = c;
         }
      }
   }
   name[j++] = 0;
}

void GetDOSName(DIR *pDir, char *fname)
{
   char i,j,leng,c,toext;

   toext = FALSE;
   j = 0;
   leng = strlen(fname);
   for (i=0;i<8;i++)
      pDir->sName[i] = ' ';
   for (i=0;i<3;i++)
      pDir->spam[i] = ' ';
   for (i=0;i<leng;i++)
   {
      c = fname[i];
      c = toupper(c);
      if (c == '.')
      {
         toext = TRUE;
         continue;
      }
      if (toext)
         pDir->spam[j++] = c;
      else
         pDir->sName[i] = c;
   }
}

// Function: Sets the file f to root dir
MMCResponse ReadRootDirectory(char fil)
{
   int32 actsector;

   if (fil > (MAXFILES-1))
      return MMC_INVALID_FILE;
   actsector = gFAT32Vars.gStartSector + DiskInfo.FATCopies*DiskInfo.hSectorsPerFat+DiskInfo.Reserved1;
   ReadSector(actsector,gFiles[fil].IOpuffer);
   gFAT32Vars.gDirEntrySector = actsector;
   gFiles[fil].dirSector = actsector;
   gFiles[fil].dirIdx = 0;
   memcpy(&(gFiles[fil].DirEntry),gFiles[fil].IOpuffer,32);
   gFiles[fil].CurrentCluster = DiskInfo.hRootStartCluster;
   return MMC_OK;
}

// Funciton: Finds a file
char FindDirEntry(char *fname,char f)
{
   DIR *pDir;
   int16 i;
   char filename[16];
   int32 nextcluster,actsector;

   if (f > (MAXFILES-1))
   {
      return FALSE;
   }
   gFAT32Vars.gFirstEmptyDirEntry = 0xFF;
   gFAT32Vars.gFirstDirEntryCluster = 0x0FFFFFFF;
   do
   {
      pDir = (DIR*)(gFiles[f].IOpuffer);
      for (i=0;i<DIRENTRYS_PER_SECTOR;i++)
      {
         if ((pDir->sName[0] == 0xE5 || pDir->sName[0] == 0) && gFAT32Vars.gFirstEmptyDirEntry == 0xFF) // store first free
         {
            gFAT32Vars.gFirstEmptyDirEntry = i;
            gFAT32Vars.gFirstDirEntryCluster = gFiles[f].CurrentCluster;
         }
         if (pDir->sName[0] == 0)
         {
            return FALSE;
         }
         ConvertFilename(pDir,filename);
         if (!strcmp(filename,fname))
         {
            memcpy(&(gFiles[f].DirEntry),pDir,32);
            gFiles[f].dirIdx = i;
            gFAT32Vars.gDirEntryIdx = i;
            return TRUE;
         }
         pDir++;
      }
      nextcluster = GetNextCluster(gFiles[f].CurrentCluster);
      if (nextcluster != 0x0FFFFFFF && nextcluster != 0)
      {
         actsector = nextcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFAT32Vars.gDirEntrySector = actsector;
         gFiles[f].dirSector = actsector;
         gFiles[f].CurrentCluster = nextcluster;
      }
   } while (nextcluster != 0x0FFFFFFF && nextcluster != 0);
   return FALSE;
}

// Function: Assign a filenumber(f) and open the directory
//           where the file are and return the filename.
char* TryFile(char *fname, char *f)
{
   char i,leng;
   char *filename;

   *f = 0xFF;
   for (i=0;i<MAXFILES;i++)
   {
      if (gFiles[i].Free)
      {
         *f = i;
         break;
      }
   }
   if (*f == 0xFF)
   {
      return 0;
   }
   ReadRootDirectory(*f);
   filename = fname;
   leng = strlen(fname);
   for (i=0;i<leng;i++)
   {
      if (fname[i] == '/')
      {
         fname[i] = 0;
         if (!cwd(filename,*f))
         {
            gFiles[*f].Free = TRUE;
            return 0;
         }
         filename = fname+i+1;
      }
   }
   return filename;
}

void strfill(char *dest,char *source,int position)
{
   char *d;
   for(d=dest+position; *source!=0; d++, source++)
   {
      *d = *source;
   }
}

// Parameters: path, the path to the directory you
//             want to view
// Function    : Initializes file/dir listing
// Examples:
// strcpy(path,""); // the "path" to the root directory
// f = InitList(path);
// ...
// strcpy(path,"DIR3/DIR32/");
// f = InitList(path);
// if(InitList == MMC_OK)
// {
//   res = Listfiles(f);
//   CloseList(f);
// }
#ifdef ENABLE_FILELISTNG
MMCResponse InitList(char *path)
{
   char f;
   if(TryFile(path,&f) == 0)
   {
      return MMC_NOT_FOUND;
   }
   gFiles[f].Free = FALSE;   
   StartList.dirSector = gFiles[f].dirSector;
   StartList.CurrentCluster = gFiles[f].CurrentCluster;
   StartList.dirIdx = 0;
   CurrentList.dirSector = gFiles[f].dirSector;
   CurrentList.CurrentCluster = gFiles[f].CurrentCluster;
   CurrentList.dirIdx = 0;
   return f;
}

// Function: Lists a part (aka page) of the files in the directory
//           specified by InitList()
// Returns : The number of files/dirs that were listed
int8 ListFiles(char f)
{
   DIR *pDir;
   char filename[(CHARACTERS_IN_LONG_FILENAMES+1)]; // should be enough with 13+1
   char i,u,fni,len;
   BOOLEAN isLongFileName = FALSE;
   int32 nextcluster,actsector;

   if (f > (MAXFILES-1))
      return 0;

   if(changeList)
   {
      FreeList();
   }

   if((gFiles[f].CurrentCluster != CurrentList.CurrentCluster) || (gFiles[f].dirSector != CurrentList.dirSector))
   {
      gFiles[f].dirSector = CurrentList.dirSector;
      gFiles[f].CurrentCluster = CurrentList.CurrentCluster;   
      ReadSector(gFiles[f].dirSector,gFiles[f].IOpuffer);
   }
   gFiles[f].dirIdx = CurrentList.dirIdx;


   u=0;
   do
   {
      pDir = (DIR*)(&(gFiles[f].IOpuffer[32*(int16)gFiles[f].dirIdx]));
      for (i=gFiles[f].dirIdx;i<DIRENTRYS_PER_SECTOR;i++) // loop throu all direntrys in the sector
      {
         if ((pDir->sName[0] != 0xE5 && pDir->sName[0] != 0)) // if file/dir isn't deleted and isn't the end of the directory
         {
            if(pDir->bAttr != 0x0F) // Normal filename (8.3)
            {
               if(isLongFilename) // If this is the short version of the long filename, just save the number
               {
                  if(changeList)
                  {
                     FileList[u].shortName = malloc(13); // 8.3 = 8chars + '.' + 3chars + 0x00
                     FileList[u].isLong = TRUE;
                     ConvertFilename(pDir,filename);
                     strcpy(FileList[u].shortName,filename); // copy the 8.3 name of the file
                  }
                  u++;
                  isLongFilename = FALSE;
                  if(u == MAX_FILE_LIST)
                  {
                     gFiles[f].dirIdx = i+1;
                     return u;
                  }
               }
               else // normal 8.3 filename
               {
                  if(changeList)
                  {
                     ConvertFilename(pDir,filename);
                     FileList[u].name = malloc(strlen(filename)+1); // +1 for char 0x00
                     strcpy(FileList[u].name,filename);
                     FileList[u].shortName = FileList[u].name; // point to same string
                     FileList[u].isLong = FALSE;
                     if(pDir->bAttr & 0x10) // is directory
                        FileList[u].isDir = TRUE;
                     else
                        FileList[u].isDir = FALSE;
                  }

                  u++;
                  if(u == MAX_FILE_LIST)
                  {
                     gFiles[f].dirIdx = i+1;
                     return u;
                  }
               }
            }
            else if((pDir->bAttr & 0x0F) == 0x0F) // If it is a long filename entry
            {
               fni = (pDir->sName[0] & 0x3F); // Filename Index
               if((pDir->sName[0] & 0x40)) // First LongFilename entry, the last characters of a long filename
               {
                  if(changeList)
                  {
                     ConvertLongFilename(pDir,filename);
                     len = strlen(filename)+CHARACTERS_IN_LONG_FILENAMES*(fni-1); // Length of the long filename
                     FileList[u].name = malloc(len+1);// number of chars in this strinng + 13 in each other long filname entry
                     FileList[u].name[len] = 0x00; // set last char to 0
                     strfill(FileList[u].name,filename,(fni-1)*CHARACTERS_IN_LONG_FILENAMES); //Fills the name from position (fni-1)*13
                     if(pDir->bAttr & 0x10) // is directory
                        FileList[u].isDir = TRUE;
                     else
                        FileList[u].isDir = FALSE;
                  }
                  isLongFilename = TRUE;
               }
               else if((pDir->sName[0] & 0x80) == 0) // If it is a long filname, but not deleted
               {
                  if(isLongfilename && changeList)
                  {
                     ConvertLongFilename(pDir,filename);
                     strfill(FileList[u].name,filename,(fni-1)*CHARACTERS_IN_LONG_FILENAMES); //Fills the name from position (fni-1)*13
                  }
                  else
                     printf(fonts,"NoLongFileName!");
               }
            }
         }
         if (pDir->sName[0] == 0)
         {
            gFiles[f].dirIdx = END_OF_DIR;
            return u;
         }
         pDir++;
      }
      nextcluster = GetNextCluster(gFiles[f].CurrentCluster);
      if (nextcluster != 0x0FFFFFFF && nextcluster != 0)
      {
         actsector = nextcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFiles[f].dirSector = actsector;
         gFiles[f].CurrentCluster = nextcluster;
         gFiles[f].dirIdx = 0;
      }
   } while (nextcluster != 0x0FFFFFFF && nextcluster != 0);
   gFiles[f].dirIdx = END_OF_DIR;
   return u;
}

// Function: Go to next page in the file/dir list
// Returns : MMCResponse
MMCResponse NextPage(char f)
{
   int32 nextcluster,actsector;
   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;

   CurrentList.dirSector = gFiles[f].dirSector;
   CurrentList.CurrentCluster = gFiles[f].CurrentCluster;

   if(gFiles[f].dirIdx == DIRENTRYS_PER_SECTOR)
   {
      nextcluster = GetNextCluster(gFiles[f].CurrentCluster);
       if (nextcluster != 0x0FFFFFFF && nextcluster != 0)
      {
         actsector = nextcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);

         CurrentList.dirSector = actsector;
         CurrentList.CurrentCluster = nextcluster;
         CurrentList.dirIdx = 0;
         return MMC_OK;
       }
      return MMC_INVALID_CLUSTER;
   }
   else if(gFiles[f].dirIdx == END_OF_DIR)
   {
      return MMC_END_OF_DIR; // Last file/dir have already been listed by ListFiles
   }
   else
   {
      CurrentList.dirIdx = gFiles[f].dirIdx;
   }
   return MMC_OK;
}

// Note: 0 = first page
MMCResponse SetPage(char f, int32 page)
{
   int32 i;
   MMCResponse res;
   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;

   CurrentList.dirSector = StartList.dirSector;
   CurrentList.CurrentCluster = StartList.CurrentCluster;
   CurrentList.dirIdx = StartList.dirIdx; // should always be 0

   changeList = FALSE; // this tells the ListFiles function to not change the list, just loop through files
   for(i=0;i<page;i++)
   {
      ListFiles(f);
      res = NextPage(f);
      if(res != MMC_OK)
      {
         changeList = TRUE;
         return res;
      }
   }
   changeList = TRUE;
   return MMC_OK;
}

MMCResponse CloseList(char f)
{
   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   gFiles[f].Free = TRUE;
   return MMC_OK;
}

void FreeList()
{
   int i;
   for(i=0;i<MAX_FILE_LIST;i++)
   {
      if(FileList[i].isLong)           // If it is a long filename, name and short name are different string
         free(FileList[i].shortName); // then free both
      else
         FileList[i].shortName = NULL;
      free(FileList[i].name);          // else free ONLY name(they point to same string..)
   }
}
#endif// ENABLE_FILELISTNG

// Function: Creates a file
MMCResponse fcreate(char f,char *fname)
{
   DIR *pDir;
   int32 actsector,actcl;
   int16 i;

   if (f > (MAXFILES-1))
   {
      return MMC_INVALID_FILE;
   }
   if (gFAT32Vars.gFirstDirEntryCluster == 0x0FFFFFFF)
   {
      // extend the directory file !!!
      gFAT32Vars.gFirstDirEntryCluster = FindFirstFreeCluster();
      gFAT32Vars.gFirstEmptyDirEntry = 0;
      SetClusterEntry(gFiles[f].CurrentCluster,gFAT32Vars.gFirstDirEntryCluster);
      SetClusterEntry(gFAT32Vars.gFirstDirEntryCluster,0x0FFFFFFF);
      actsector = gFAT32Vars.gFirstDirEntryCluster + gFAT32Vars.gFirstDataSector;
      for (i=0;i<512;i++)
         gFiles[f].IOpuffer[i] = 0;
      WriteSector(actsector,gFiles[f].IOpuffer);
   }
   actsector = gFAT32Vars.gFirstDirEntryCluster + gFAT32Vars.gFirstDataSector;
   ReadSector(actsector,gFiles[f].IOpuffer);
   pDir = (DIR*)(&(gFiles[f].IOpuffer[32*(int16)gFAT32Vars.gFirstEmptyDirEntry]));
   gFiles[f].dirSector = actsector;
   gFiles[f].dirIdx = gFAT32Vars.gFirstEmptyDirEntry;
   GetDOSName(pDir,fname);
   pDir->bAttr = 0;
   actcl = FindFirstFreeCluster();
   pDir->hCluster = actcl & 0xFFFF;
   pDir->hClusterH = actcl >> 16;
   SetClusterEntry(actcl,0x0FFFFFFF);
   pDir->wSize = 0;
   gFiles[f].position = 0;
   pDir->hDate = GetCurrentDOSDate();
   pDir->hTime = GetCurrentDOSTime();
   WriteSector(actsector,gFiles[f].IOpuffer);
   memcpy(&(gFiles[f].DirEntry),pDir,32);
   return MMC_OK;
}

int32 ComposeCluster(char f)
{
   int32 retval;

   retval = gFiles[f].DirEntry.hClusterH;
   retval <<= 16;
   retval |= gFiles[f].DirEntry.hCluster;
   return retval;
}

// Function: Opens a file with the specified mode
// Returns : A file handle or error code
MMCResponse fopen(char *fname, char mode)
{
   char found;
   char f;
   int32 actsector,actcluster,nextcluster;
   char *filename;

   if (!CardInserted())
      return MMC_NO_CARD_INSERTED;

   filename = TryFile(fname,&f);
   if (filename == 0)
   {
      return MMC_NOT_FOUND; // probebly invalid directory
   }
   found = FALSE;
   found = FindDirEntry(filename,f);

   if (!found)
   {
      if (mode == 'r')
      {
         gFiles[f].Free = TRUE;
         return MMC_NOT_FOUND;
      }
      else
      {
         if (fcreate(f,filename) != MMC_OK)
            return MMC_NOT_FOUND;
         found = TRUE;
      }
   }
   if (found)
   {
      gFiles[f].Free = FALSE;
      gFiles[f].mode = mode;
      if  (mode == 'a')
      {
         gFiles[f].position = gFiles[f].DirEntry.wSize;
         actcluster = ComposeCluster(f);
         while (actcluster != 0x0FFFFFFF && nextcluster != 0)
         {
            nextcluster = GetNextCluster(actcluster);
            if (nextcluster == 0x0FFFFFFF || nextcluster == 0)
               break;
            actcluster = nextcluster;
         }
         actsector = actcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFiles[f].CurrentCluster = actcluster;
         gFiles[f].posinsector = gFiles[f].position & 0x01FF;
         if (gFiles[f].posinsector == 0 && gFiles[f].position != 0)
            gFiles[f].posinsector = 512;
      }
      else
      {
         gFiles[f].position = 0;
         actsector = ComposeCluster(f);
         actsector += gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFiles[f].CurrentCluster = ComposeCluster(f);
         gFiles[f].posinsector = 0;
      }
   }
   return f;
}

// Function: closes a open file and makes it avavible for a new file
MMCResponse fclose(char f)
{
   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   if ((gFiles[f].mode == 'a') || (gFiles[f].mode == 'w'))
      fflush(f);
   gFiles[f].Free = TRUE;
   return MMC_OK;
}

// Function: writes the chach to the MMC
MMCResponse fflush(char f)
{
   int32 actsector;
   DIR *pDir;

   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   actsector = gFiles[f].CurrentCluster + gFAT32Vars.gFirstDataSector;
   WriteSector(actsector,gFiles[f].IOpuffer);
   ReadSector(gFiles[f].dirSector,gFiles[f].IOpuffer);
   pDir = (DIR*)(&(gFiles[f].IOpuffer[32*gFiles[f].dirIdx]));
   if (gFiles[f].DirEntry.bAttr & 0x10)  // if it is a directory
      pDir->wSize = 0;
   else
      pDir->wSize = gFiles[f].position;
   pDir->hDate = GetCurrentDOSDate();
   pDir->hTime = GetCurrentDOSTime();
   WriteSector(gFiles[f].dirSector,gFiles[f].IOpuffer);
   ReadSector(actsector,gFiles[f].IOpuffer);
   return MMC_OK;
}

// Function: Enter a specified directory
char cwd(char *fname, char f)
{
   int32 actsector;
   if (f > (MAXFILES-1))
   {
      return FALSE; // just in case of overaddressing
   }
   if (IsSelfDir(fname))
   {
      return TRUE; // already in Root dir
   }
   if (!FindDirEntry(fname,f))
   {
      return FALSE; // not found
   }

   actsector = ComposeCluster(f);
   actsector += gFAT32Vars.gFirstDataSector; // read current dir
   ReadSector(actsector,gFiles[f].IOpuffer);
   gFAT32Vars.gDirEntrySector = actsector;
   gFiles[f].dirSector = actsector;
   gFiles[f].CurrentCluster = ComposeCluster(f);

   return TRUE;
}

// Function: Put a char to the open file
MMCResponse fputch(char be, char f)
{
   int32 nextcluster,actsector;

   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   if (gFiles[f].posinsector == 512)
   {
      actsector = gFiles[f].CurrentCluster + gFAT32Vars.gFirstDataSector;
      WriteSector(actsector,gFiles[f].IOpuffer);
      nextcluster = FindFirstFreeCluster();
      if (nextcluster != 0x0FFFFFFF && nextcluster != 0)
      {
         SetClusterEntry(gFiles[f].CurrentCluster,nextcluster);
         SetClusterEntry(nextcluster,0x0FFFFFFF);
         actsector = nextcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFiles[f].CurrentCluster = nextcluster;
         gFiles[f].posinsector = 0;
      }
   }
   gFiles[f].IOpuffer[gFiles[f].posinsector] = be;
   gFiles[f].posinsector++;
   gFiles[f].position++;
   return MMC_OK;
}

// Function: Puts a string to the open file
MMCResponse fputstring(char *be, char f)
{
   int16 leng,i;

   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   leng = strlen(be);
   for (i=0;i<leng;i++)
      fputch(be[i],f);
   return MMC_OK;
}

// Function: Read a buffer from the open file
int16 fread(char *buffer, int16 leng, char f)
{
   int16 i,retv;
   char c,v;

   if (f > (MAXFILES-1))
      return 0;
   retv = 0;
   for (i=0;i<leng;i++)
   {
      v = fgetch(&c,f);
      if (v == MMC_OK)
      {
         buffer[i] = c;
         retv++;
      }
      else
         break;
   }
   return retv;
}

// Function: Write a buffer to the open file
MMCResponse fwrite(char *buffer, int16 leng, char f)
{
   int16 i;

   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   for (i=0;i<leng;i++)
      fputch(buffer[i],f);
   return MMC_OK;
}

// Function: Read a char from the open file
MMCResponse fgetch(char *ki,char f)
{
   int32 nextcluster,actsector;
   unsigned int16 numbers;
   if (f > (MAXFILES-1))
      return MMC_INVALID_FILE;
   if (gFiles[f].position >= gFiles[f].DirEntry.wSize)
      return MMC_INVALID_POSITION;
      numbers=gFiles[f].posinsector;
   *ki = gFiles[f].IOpuffer[numbers];
   gFiles[f].posinsector++;
   gFiles[f].position++;
   if (gFiles[f].posinsector == 512)
   {
      nextcluster = GetNextCluster(gFiles[f].CurrentCluster);
      if (nextcluster != 0x0FFFFFFF && nextcluster != 0)
      {
         actsector = nextcluster + gFAT32Vars.gFirstDataSector;
         ReadSector(actsector,gFiles[f].IOpuffer);
         gFiles[f].CurrentCluster = nextcluster;
         gFiles[f].posinsector = 0;
      }
   }
   return MMC_OK;
}

// Function: Removes a file
MMCResponse remove(char *fname)
{
   char i,found;
   char f;
   DIR *pDir;
   int32 nextcluster,currentcluster;
   char *filename;

   filename = TryFile(fname,&f);
   if (filename == 0)
      return MMC_NOT_FOUND;
   found = FindDirEntry(filename,f);
   if (!found)
   {
      gFiles[f].Free = TRUE;
      return MMC_NOT_FOUND;
   }

   pDir = (DIR*)(&(gFiles[f].IOpuffer[32*(int16)gFAT32Vars.gDirEntryIdx]));
   pDir->sName[0] = 0xE5;
   for (i=1;i<8;i++)
      pDir->sName[i] = ' ';
   for (i=0;i<3;i++)
      pDir->spam[i] = ' ';
   WriteSector(gFAT32Vars.gDirEntrySector,gFiles[f].IOpuffer);
   currentcluster = ComposeCluster(f);
   while (currentcluster != 0x0FFFFFFF && nextcluster != 0)
   {
      nextcluster = GetNextCluster(currentcluster);
      ClearClusterEntry(currentcluster);
      currentcluster = nextcluster;
   }
   ClearClusterEntry(currentcluster);
   SetClusterEntry(currentcluster,0);
   currentcluster = gFAT32Vars.gStartSector+DiskInfo.Reserved1 + gFAT32Vars.FATstartidx;
   WriteSector(currentcluster,FATTable);
   currentcluster += DiskInfo.hSectorsPerFat;
   WriteSector(currentcluster,FATTable);
   gFiles[f].Free = TRUE;

   return MMC_OK;
}


// Function: Gets the size of a file
MMCResponse getfsize(char *fname, int32 *fsiz)
{
   char found;
   char f;
   DIR *pDir;
   char *filename;

   filename = TryFile(fname,&f);
   if (filename == 0)
      return MMC_NOT_FOUND;
   found = FindDirEntry(filename,f);
   if (!found)
   {
      gFiles[f].Free = TRUE;
      return MMC_NOT_FOUND;
   }
   pDir = (DIR*)(&(gFiles[f].IOpuffer[32*(Int16)gFAT32Vars.gDirEntryIdx]));
   gFiles[f].Free = TRUE;
   *fsiz = pDir->wSize;
   return MMC_OK;
} 
