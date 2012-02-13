#include <18F2550.h>
#fuses HSPLL,WDT1,NOPROTECT,NODEBUG,NOBROWNOUT,USBDIV,PLL5,CPUDIV1,NOVREGEN,PUT,NOMCLR,NOLVP,BORV28,NOFCMEN,NOPBADEN
#use delay(clock=48000000)
#define CODE_START   0x1000
#build(reset=CODE_START, interrupt=CODE_START+0x08)
#org 0, CODE_START-1 {}

#include <N_LCD.h>
#include <N_LCD.c>

#include <STDLIB.H>
#include <MMC_SPI_FAT32.h>
#include <MMC_SPI_FAT32.c>
void update_menu(void);
void draw_bpm(char *fnome);
void draw_txt(char *fnome);

unsigned int loop,submenu=0,opzioni=5,fixyin=67,fixyfin=66;
int1 enable_slide=1,scende=1,mmc_on=0,read_file=0, file_bmp=0,file_txt=0,tema=0;
char menu_header[20]={"Menu"};
char menu[2][5][11]={
   {"Tetris","Arkanoid","Grafica 3D","Memoria","Imposta"},
   {"Tema si/no","Contrasto","USB loader","Info",""}
};
int riga[10]={14,24,34,44,54,64,74,84,94,104};
char logo[20];
int8 secondi=0, minuti=0, ore=0;
#int_TIMER1
void  TIMER1_isr(void)     //  tick
{
TMR1H=0x80;
secondi++; // incremento i secondi
 
      if (secondi==60)
         {
         secondi=0;
         minuti++;       
         }
      if (minuti==60)
         {
         minuti=0;
         ore++;
         }
      if (ore==24)
         {
         ore=0;
         }
TMR1IF=0;
      if (secondi==0){
         quadrato(100,130,1,11, BLUE);
         gotoxy(101,3);
         font_color(WHITE);
         printf(fonts,"%02d:%02d",ore,minuti);
         }
} 

void main(void){ 
   Init_Spi();
   Init_Lcd();
   
   if(MMCInit() == MMC_TIME_OUT){   
   delay_ms(100);
   strcpy(logo,"PROVA/LOGO.BMP");
   lcd_cmd(DISPOFF);
   draw_bpm(logo);
   lcd_cmd(DISPON);
   delay_ms(3000);
   }
   
   update_menu();
   loop=4;
  TMR1H=0x80;
  TMR1L=0X00;
  GIE=1;
  PEIE=1;
  TMR1IE=1;
  T1CON=0b00001111;

  
   //set_tris_b(0x00);
   while (TRUE) {
      
      if(input_state(PB_DOWN)==0){scende=1; enable_slide=1; }
      if(input_state(PB_UP)==0){scende=0; enable_slide=1; }

      if(enable_slide==1){
         if(tema==0){
         quadrato(1,130,riga[loop]-2,riga[loop]-2, LIGHT_B);
         quadrato(1,130,riga[loop]+8,riga[loop]+8, LIGHT_B);
         }
         gotoxy(5,riga[loop]);
         font_color(BLACK);
         if(mmc_on==0) printf(fonts,menu[submenu][loop]);
            else{
               printf(fonts,FileList[loop].name);
               if(FileList[loop].isDir) printf(fonts,"/");//a "\" with row break
               }
         //printf(fonts,menu[submenu][loop]);
         if(scende==1){
            loop++;
            if(loop==opzioni)loop=0;
         }
         else{
            loop--;
            if(loop==255)loop=opzioni-1;
         }
         if(tema==0){
            quadrato(1,130,riga[loop]-2,riga[loop]-2, BLUE);
            quadrato(1,130,riga[loop]+8,riga[loop]+8, BLUE);
         }
         font_color(RED);
         gotoxy(5,riga[loop]);
         if(mmc_on==0) printf(fonts,menu[submenu][loop]);
         else{
            printf(fonts,FileList[loop].name);
            if(FileList[loop].isDir) printf(fonts,"/");//a "\" with row break
         }
         //printf(fonts,menu[submenu][loop]);
         enable_slide=0;
         delay_ms(150);
      }
      
      if((input_state(PB_OK)==0)&&(loop==1)&&(submenu==0)){
      unsigned int16 colormaggy=0x0f00,colorcount=0;
         delay_ms(100);
         quadrato(1,130,1,130, WHITE);
         for(colorcount=0; colorcount<0xffff; colorcount++){
         gotoxy(50,50);
         font_color(colormaggy);
         printf(fonts,"Ciao mondo");
         colormaggy=colormaggy+0x001f;
         delay_ms(1);  
         }
         update_menu();
      } 
      
      if((input_state(PB_OK)==0)&&(loop==4)&&(submenu==0)){
         delay_ms(100);
         menu_header="Menu->Imposta";
         opzioni=3;
         submenu=1;
         update_menu();
      } 
      
      if((input_state(PB_OK)==0)&&(loop==2)&&(submenu==1)){
         reset_cpu();
      }
      if((input_state(PB_OK)==0)&&(loop==0)&&(submenu==1)){
      if(tema==1){ tema=0;}
      else{ tema=1; }
      menu_header="Menu";
         opzioni=5;
         submenu=0;
         mmc_on=0;
         update_menu();
      }
      
      if((input_state(PB_LEFT)==0)&&(submenu!=0)){
         menu_header="Menu";
         opzioni=5;
         submenu=0;
         
         if (mmc_on==1) FreeList();
         mmc_on=0;
         update_menu();        
      }

      if((input_state(PB_OK)==0)&&((loop==3)||(mmc_on==1))&&((submenu==0)||(mmc_on==1))){
         char f,filename[20];
         int8 i;
         
         char slash[]="/";
         if(mmc_on==1){
         //strcpy(filename,"");
            if(FileList[loop].isDir) {
               strcat(filename,FileList[loop].name);
               strcat(filename,slash);           
            }
            else{
            strcat(filename,slash);
            strcat(filename,FileList[loop].shortName);
            read_file=1;            
            }
            if(loop==0) strcpy(filename,""); 
            FreeList();
         }
         strcpy(menu_header,"mmc:/");
         
         
        // quadrato(1,130,1,130, LIGHT_B);
         gotoxy(5,4);
         font_color(BLACK);
         if(read_file==0){
         if(MMCInit() == MMC_OK)                               
         {                                                     
           // printf(fonts, "Inizializzata!");
            delay_ms(1);
         
         
         //quadrato(1,130,1,130, LIGHT_B);
        // gotoxy(5,14);
         InitFAT();                                          
         if(mmc_on==0)strcpy(filename,""); 
         
         f = InitList(filename);
         strcat(menu_header,filename);
         if(f & MMC_ERROR){                                  
            printf(fonts,"No lista.");
            delay_ms(1000);
         }
         else{                                                         
            opzioni = ListFiles(f); 
            mmc_on=1;
            submenu=2;
            update_menu();             
            CloseList(f); // frees the file, but the list is 
           //FreeList();
         }
        // delay_ms(2000);
         //menu_header="MMC:";
         //opzioni=5;
        // submenu=0;
        // mmc_on=0;
         //update_menu();
      } 
     else{
            quadrato(1,130,1,130, LIGHT_B);
            printf(fonts, "Inserire mmc!");         
            delay_ms(1000);
            update_menu();
     }
     }
     if(read_file==1){
            
            ///Controlla l'estensione del file
          for(i=0; i<20; i++){
            if(filename[i]==0x2e){//.
               if(filename[i+1]==0x54){//T
                  if(filename[i+2]==0x58){//X
                     if(filename[i+3]==0x54){//T
                        file_txt=1;
                     }
                  }      
               }
            }
            if(filename[i]==0x2e){//.
               if(filename[i+1]==0x42){//B
                  if(filename[i+2]==0x4d){//M
                     if(filename[i+3]==0x50){//P
                        file_bmp=1;
                     }
                  }      
               }
            }
          }
            
            quadrato(1,130,1,130, LIGHT_B);                      
            if((file_txt==0)&&(file_bmp==0)){ 
               font_color(BLACK);
               gotoxy(5,24);
               printf(fonts, "File non supportato!");
            }
            
    if(file_txt==1){        
      draw_txt(filename);
      }
      if(file_bmp==1){
         draw_bpm(filename);
      }
      
      
      delay_ms(100);
         while(input_state(PB_OK)==1)
            ;
      //delay_ms(3000);
      menu_header="Menu";
      opzioni=5;
      submenu=0;
      mmc_on=0;
      update_menu();
      read_file=0;
     }
     }
   }
}

///Funzioni

void update_menu(void){

   if(tema==1){
      strcpy(logo,"IMAGE/KENIA.BMP");
      draw_bpm(logo);
      }
   else{   
   quadrato(1,130,12,118, LIGHT_B);
   quadrato(1,130,1,11, BLUE);
   quadrato(1,130,119,130, BLUE);
   }
   gotoxy(5,3);
   font_color(WHITE);
   printf(fonts,menu_header);
   font_color(BLACK);

   for(loop=0; loop<opzioni; loop++){
      gotoxy(5,riga[loop]);    
      if(mmc_on==0) printf(fonts,menu[submenu][loop]);
      else{
         printf(fonts,FileList[loop].name);
         if(FileList[loop].isDir) printf(fonts,"/");
         }
   }
   loop=opzioni-1;
   enable_slide=1;
   scende=1;
}

void draw_bpm(char *fnome){
int16 bmp_color,ii,r,g,area;
char f,i;
         int8 b,bmpx,bmpy;
         file_bmp=0;
         //lcd_cmd(DISPOFF);
         //quadrato(1,130,1,130, LIGHT_B);
         lcd_cmd(MADCTL);
         lcd_data(0b01000000);
         if(MMCInit() == MMC_OK){
            InitFAT();
            f = fopen(fnome,'r');
            bmpx=gFiles[f].IOpuffer[18];
            bmpy=gFiles[f].IOpuffer[22];
             area=((unsigned int16)bmpx*(unsigned int16)bmpy)+1;
            if((f & MMC_ERROR) == 0) {
               lcd_cmd(CASET);   ///la X
               lcd_data(67-(gFiles[f].IOpuffer[18]/2));
               lcd_data(66+(gFiles[f].IOpuffer[18]/2));
               lcd_cmd(PASET);      ////la Y
               lcd_data(fixyin-(gFiles[f].IOpuffer[22]/2));//67-
               lcd_data(fixyfin+(gFiles[f].IOpuffer[22]/2));//66+
               for(i=0; i<54; i++){ ///OK, verificato
                  fgetch(&r,f);              
               }
              
               //printf(fonts, "%x",r);
               lcd_cmd(RAMWR);
               for(ii=0; ii<area; ii++){
                  fgetch(&b,f);
                  fgetch(&g,f);
                  fgetch(&r,f);
                  bmp_color=RGB16(r,g,b);                
                  lcd_data(bmp_color>>8);
                  lcd_data(bmp_color);
               }
               fclose(f);
            }
            
            
         }
         else{
            quadrato(50,60,50,60, BLUE);
            
         }
         //lcd_cmd(DISPON);
         lcd_cmd(MADCTL);
         lcd_data(0b11100000);
         //printf(fonts, "Da implementare..%ld",area);
}

void draw_txt(char *fnome){
char f,c,tmp;
unsigned int8 i;
quadrato(1,130,1,12, BLUE);           
font_color(WHITE);
gotoxy(5,4);
printf(fonts, fnome);
font_color(BLACK);
gotoxy(5,24);
if(MMCInit() == MMC_OK){
         InitFAT();
         f = fopen(fnome,'r');
         if((f & MMC_ERROR) == 0) {       
            while(fgetch(&c,f) == MMC_OK) { 
               
               
               //if(fgetch(&c,f)!=MMC_OK) i=255;
               
               if((tmp==0x0d)&&(c==0x0a)) {y=y+10; x=5; gotoxy(x,y);}
               if((x==0x05)&&(c==0x20)) {c=0x0d;}
               
               if(c!=0x0d){
                  if(c!=0x0a){
                     if(c!=0x09){                        
                        printf(fonts, "%c",c); 
                        delay_ms(20);                          
                        }
                     }
                  }
                  if(c==0x09) {x=x+15; gotoxy(x,y);}
                  if(x>125){ x=5; y=y+10;}
                  tmp=c;
                  if(y>120){
                     while(input_state(PB_DOWN)==1)
                     ;
                     quadrato(1,130,13,130, WHITE);
                     gotoxy(5,24);
                     i=0;
                  }
               }
               fclose(f);
            //while(input_state(PB_OK)==1)
           // ;
         }
         else{
            quadrato(1,130,1,130, LIGHT_B);
            printf(fonts, fnome);
         }
      }
      file_txt=0;
}
