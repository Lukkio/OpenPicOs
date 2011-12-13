#define MAXFILES 1 // This define specifies the maximum number
               // of simultaneously open files, more files
               // requires more ram. The maximun number of
               // this define is 8, this is because of my
               // error handlig.. but that should be more
               // then enough..

// This enable functions and variables needed for filelisting
// If you aren't going to use it, comment this line out to
// save some RAM
#define ENABLE_FILELISTNG

typedef struct _diskinforec
{
   char   hJumpCode[3];
   char  OEMName[8];
   int16   hBytesPerSector;
   char   bSectorsPerCluster;
   int16 Reserved1;
   char   FATCopies;
   int16 hMaxRootEntries;
   int16 hSectors;
   char   Descriptor;
   int16 holdSectorsPerFat;
   int16 hSectorsPerTrack;
   int16 hNumberofHeads;
   int32   hNumberofHidden;
   int32   hNumberofSectors;

   int32 hSectorsPerFat;
   int16 hFlags;
   int16 hFAT32Version;
   int32 hRootStartCluster;
} diskinforec;

typedef struct _direntry
{
   char   sName[8];
   char   spam[3];
   char   bAttr;
   char   bReserved[8];
      int16 hClusterH;
   int16   hTime;
   int16   hDate;
   int16   hCluster;
   int32   wSize;
} DIR;

typedef struct
{
   char    IOpuffer[512];
   DIR    DirEntry;
   int32 CurrentCluster;
   int16 posinsector;
   int32   position;
   int32 dirSector;
   int8   dirIdx;
   char   mode;
   BOOLEAN   Free;
} FILE;

typedef struct
{
   int32 MMCAddress;
   int32 FATstartidx;
   int32 gStartSector;
   int32 gFirstDataSector;
   int8 gDirEntryIdx;
   int32 gDirEntrySector;
   int8 gFirstEmptyDirEntry;
   int32 gFirstDirEntryCluster;
} FAT32Vars;

typedef struct
{
   unsigned long   Year;
   char         Month;
   char            Day;   
   char            Hour;   
   char            Minute;     
   char            Second;     
} TimeRecord;

typedef struct
{
   int32 dirSector;
   int32 CurrentCluster;
   int dirIdx;
} ListPos;

typedef struct
{
   char *name;
   char *shortName;
   BOOLEAN isDir;
   BOOLEAN isLong;
} LongFileName;

typedef char MMCResponse;

MMCResponse MMCInit();
MMCResponse ReadSector(int32 sector, char *hova);
MMCResponse WriteSector(int32 sector, char *honnan);

void      InitFAT();
char      FindDirEntry(char *fname, char f);

#ifdef ENABLE_FILELISTNG
   MMCResponse InitList(char *path);
   int8      ListFiles(char f);
   MMCResponse NextPage(char f);
   MMCResponse SetPage(char f, int32 page);
   MMCResponse CloseList(char f);
   void      FreeList();
#endif

MMCResponse fopen(char *fname, char mode);
MMCResponse fclose(char f);
MMCResponse fflush(char f);
char      cwd(char *fname, char f);
MMCResponse fputch(char be, char f);
MMCResponse fgetch(char *ki, char f);
MMCResponse fputstring(char *be, char f); // fputs is reserved in CCS C
int16      fread(char *buffer, int16 leng, char f);
MMCResponse fwrite(char *buffer, int16 leng, char f);
MMCResponse remove(char *fname);
MMCResponse getfsize(char *fname, int32 *fsiz); 