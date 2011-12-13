////Header funzioni LCD
void Init_Spi();
void Init_Lcd();
void lcd_cmd(int8 cmd);
void lcd_data(int8 cmd);
void lcd_clear();
void quadrato(int8 xin, int8 xfin, int8 yin, int8 yfin, int16 colore);
int fonts(char c);
void gotoxy(int8 xset,int8 yset);
void font_color(int16 colore);
void draw_pixel(int8 xset, int8 yset, int16 colore);

