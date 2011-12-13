
#include <N_LCD.h>
#include <Hardware_IO.c>
#include <Times_fonts.c>

////Comandi LCD
#define NOP    0x00       // nop
#define SWRESET 0x01       // software reset
#define BSTROFF 0x02       // booster voltage OFF
#define BSTRON    0x03       // booster voltage ON
#define RDDIDIF 0x04       // read display identification
#define RDDST    0x09       // read display status
#define SLEEPIN 0x10       // sleep in
#define SLEEPOUT 0x11       // sleep out
#define PTLON    0x12       // partial display mode
#define NORON    0x13       // display normal mode
#define INVOFF    0x20       // inversion OFF
#define INVON    0x21       // inversion ON
#define DALO    0x22       // all pixel OFF
#define DAL    0x23       // all pixel ON
#define SETCON    0x25       // write contrast
#define DISPOFF 0x28       // display OFF
#define DISPON    0x29       // display ON
#define CASET    0x2A       // column address set
#define PASET    0x2B       // page address set
#define RAMWR    0x2C       // memory write
#define RGBSET    0x2D       // colour set
#define PTLAR    0x30       // partial area
#define VSCRDEF 0x33       // vertical scrolling definition
#define TEOFF    0x34       // test mode
#define TEON    0x35      // test mode
#define MADCTL    0x36       // memory access control
#define SEP    0x37       // vertical scrolling start address
#define IDMOFF    0x38       // idle mode OFF
#define IDMON    0x39       // idle mode ON
#define COLMOD    0x3A       // interface pixel format
#define SETVOP    0xB0       // set Vop
#define BRS    0xB4       // bottom row swap
#define TRS    0xB6       // top row swap
#define DISCTR    0xB9       // display control
#define DOR    0xBA       // data order
#define TCDFE    0xBD       // enable/disable DF temperature compensation
#define TCVOPE    0xBF       // enable/disable Vop temp comp
#define EC       0xC0       // internal or external oscillator
#define SETMUL    0xC2       // set multiplication factor
#define TCVOPAB 0xC3       // set TCVOP slopes A and B
#define TCVOPCD 0xC4       // set TCVOP slopes c and d
#define TCDF    0xC5       // set divider frequency
#define DF8COLOR 0xC6       // set divider frequency 8-color mode
#define SETBS    0xC7       // set bias system
#define RDTEMP    0xC8       // temperature read back
#define NLI    0xC9       // n-line inversion
#define RDID1    0xDA       // read ID1
#define RDID2    0xDB       // read ID2
#define RDID3    0xDC       // read ID3
#define  COLOR_12_BIT   0x03
#define  COLOR_16_BIT   0x05
////Colori

#DEFINE   WHITE RGB16(0xff,0xff,0xff)

#define BLACK RGB16(0x00,0x00,0x00)
#DEFINE  BLUE RGB16(0x00,0x00,0xff)
#define GREEN RGB16(0x00,0xff,0x00)
#define RED RGB16(0xff,0x00,0x00)
#define LIGHT_B RGB16(0xff,0xff,0xff)
#define BROWN RGB16(0xA5,0x2A,0x2A)
#define PINK RGB16(0xff,0xc0,0xcb)
#define YELLOW RGB16(0xff,0xff,0x00)

#define RGB16(R,G,B) (((R << 8) & 0xF800) | ((G << 3) & 0x7E0) | (B >> 3))

///Colore & posizione fonts
int8 x=0,y=0;
unsigned int16 color1;

   
   void Init_Spi(){
   
   output_b(0x00);
   set_tris_b(0b10000000);
   output_c(0x00);
   set_tris_c(0x00);
   output_a(0x00);
   set_tris_a(0xfe);
  // output_high(LCD_RST);
   //output_high(LCD_CS);
   //output_high(PIN_B6);
   //output_high(PIN_B5);
   //output_high(PIN_B4);
   
   SSPCON1 = 0b00010000;
   
   SSPEN=0;
   /////////////// da completare
   }
   
   void Init_Lcd(){
  // int i;
/*   static int8 RGB12ColorMap[48] = {
   // number of bytes in the table excluding this one
   
   // red map: an input 4 bit rrrr color is mapped to an output 5 bit rrrrr color
   0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F,
   
   // green map: an input 4 bit gggg color is mapped to an output 6 bit gggggg color
   0x00,0x07,0x0B,0x0F,0x13,0x17,0x1B,0x1F,0x23,0x27,0x2B,0x2F,0x33,0x37,0x3B,0x3F,
   
   // blue map: an input 4 bit bbbb color is mapped to an output 5 bit bbbbb color
   0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1F};*/
   
   output_low(LCD_RST);
   Delay_MS(30);
   output_high(LCD_RST);
   Delay_MS(30);
   lcd_cmd(SLEEPOUT);
   lcd_cmd(MADCTL);
   lcd_data(0b11100000);
   lcd_cmd(SETCON);
   lcd_data(0x30);
   delay_ms(4);
   
   
   lcd_cmd(COLMOD);
   lcd_data(COLOR_16_BIT);
   quadrato(1,130,1,130,WHITE);
   lcd_cmd(DISPON);
   //lcd_cmd(RGBSET);
   
  // for(i=0; i<48; i++){
  // lcd_data(RGB12ColorMap[i]);
  // }
   
   /////DA coompletare
   }
   
   ///Invia Comando
   void lcd_cmd( int8 cmd){

   //invia 9° bit del comando
   output_low(LCD_CS);
   output_low(LCD_SCK);
   output_low(LCD_SDO);   //Comando
   output_high(LCD_SCK);
   ///Attiva SPI in hardware
   SSPEN=1;   
   SSPBUF = cmd;
   while(!BF);
   delay_cycles(5);
   
   output_high(LCD_CS);
   SSPEN=0;
   }
      ///Invia dati
   void lcd_data( int8 cmd){


   //invia 9° bit del comando
   output_low(LCD_CS);
   output_low(LCD_SCK);
   output_high(LCD_SDO);   //Data
   output_high(LCD_SCK);
   ///Attiva SPI in hardware
   SSPEN=1;   
   SSPBUF = cmd;
   while(!BF);
   delay_cycles(5);
   
   output_high(LCD_CS);
   SSPEN=0;
   }
   
   void lcd_clear(){
   int primi4, mezzo4, ultimi4;
   
   unsigned int16 i;
   lcd_cmd(PASET);
   lcd_data(1);
   lcd_data(130);
   lcd_cmd(CASET);
   lcd_data(1);
   lcd_data(130);
   lcd_cmd(RAMWR);
   
   primi4 = (WHITE >> 4) & 0xff;
   mezzo4 = (((WHITE & 0x0f) << 4) | ((WHITE >> 8) & 0x0f));
   ultimi4 = (WHITE & 0xff);
  // for(i=0; i<8450; i++){ 
      for(i=0; i<8450; i++){ 
      lcd_data(primi4);
      lcd_data(mezzo4);
      lcd_data(ultimi4);
      }
   }
   
   
   void quadrato(int8 xin, int8 xfin, int8 yin, int8 yfin, int16 colore){
   //int primi4, mezzo4, ultimi4; 
   unsigned int16 i;
   unsigned int16 count;
   
   
   lcd_cmd(CASET);   ///la X
   lcd_data(xin);
   lcd_data(xfin);
   lcd_cmd(PASET);      ////la Y
   lcd_data(yin);
   lcd_data(yfin);
   lcd_cmd(RAMWR);
   //fill=RGB16(colore,colore,colore);
   
   //primi4 = (colore >> 4) & 0xff;
   //mezzo4 = (((colore & 0x0f) << 4) | ((colore >> 8) & 0x0f));
   //ultimi4 = (colore & 0xff);
   
  // for(i=0; i<8450; i++){ 
  count= (((unsigned int16)(xfin-xin+1)*(unsigned int16)(yfin-yin+1)))+1;
      for(i=0; i<count; i++){ 
      //lcd_data(primi4);
      //lcd_data(mezzo4);
      //lcd_data(ultimi4);
      lcd_data(colore>>8);
      lcd_data(colore);
      
      }
   }
   
   //PRintf caratteri
   int fonts(char c){
   unsigned int16 index;
   int8 i,b,data;
   
   if(c==0x0D)c=" ";
   if(c==0x0A){y=y+10; x=0; c=" ";} 
   c-=32;
   index = (unsigned int16)c*5;
   
   for(i=0; i<5; i++){
   
      data= lcd_font[index++];
      for(b=0; b<8; b++){
         if(data & 0x01) {
            lcd_cmd(RAMWR);
            lcd_data(color1>>8);
            lcd_data(color1);            
            //lcd_data(color3);
            gotoxy(x,++y);
         }
         else{
            gotoxy(x,++y);
         }
      data >>= 1;   
      }
      y=y-8;
      gotoxy(++x,y);
   }
   gotoxy(++x,y);
   if(x>131){ x=5; y=y+10;}
   if(y>120) y=24;
   return 0;
   }
   
   void gotoxy(int8 xset,int8 yset){
   lcd_cmd(CASET);   ///la X
   lcd_data(xset);
   lcd_data(xset);
   lcd_cmd(PASET);      ////la Y
   lcd_data(yset);
   lcd_data(yset);
   x=xset;
   y=yset;
   }
   
   void font_color(int16 colore){
   //color1 = (colore >> 4) & 0xff;
   //color2 = (((colore & 0x0f) << 4) | ((colore >> 8) & 0x0f));
   //color3 = (colore & 0xff);
   color1=colore;
   }
   
   void draw_pixel(int8 xset, int8 yset, int16 colore){
   //int primi4, mezzo4, ultimi4; 
   lcd_cmd(CASET);   ///la X
   lcd_data(xset);
   lcd_data(xset);
   lcd_cmd(PASET);      ////la Y
   lcd_data(yset);
   lcd_data(yset);
  // primi4 = (colore >> 4) & 0xff;
  // mezzo4 = (((colore & 0x0f) << 4) | ((colore >> 8) & 0x0f));
  // ultimi4 = (colore & 0xff);
   lcd_cmd(RAMWR);
   // lcd_data(primi4);
    //lcd_data(mezzo4);
    lcd_data(colore);
    lcd_data(colore>>8);
   }
