//lcd Pinout
#DEFINE LCD_CS PIN_B3
#DEFINE LCD_SDO PIN_C7
#DEFINE LCD_RST PIN_B2
#DEFINE LCD_SCK PIN_B1

//Pad Pinout
#DEFINE PB_UP PIN_A3 
#DEFINE PB_DOWN PIN_A1 
#DEFINE PB_LEFT PIN_A4 
#DEFINE PB_RIGHT PIN_A2 
#DEFINE PB_OK PIN_A5 

///IR
#DEFINE IR PIN_A0

///////////////Registri ///////////////////// pagina 66

///Porta SPI
#byte SSPSTAT = 0xFC7
#byte SSPCON1 = 0xFC6
#byte SSPBUF = 0xFC9
#bit SSPEN = SSPCON1.5
#bit BF = SSPSTAT.0

//Timer1
#byte TMR1H = 0xFCF
#byte TMR1L = 0xFCE
#byte PIR1 = 0xF9E
#bit TMR1IF = PIR1.0
#byte INTCON = 0xFF2
#byte PIE1 = 0xF9D
#byte T1CON = 0xFCD

#bit GIE = INTCON.7
#bit PEIE = INTCON.6
#bit TMR1IE = PIE1.0
