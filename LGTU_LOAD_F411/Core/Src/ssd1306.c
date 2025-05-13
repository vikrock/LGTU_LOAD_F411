#include "ssd1306.h"
#include <stdlib.h>
#include <string.h>  // For memcpy
#include "encoder.h"


#if defined(SSD1306_USE_I2C)

void ssd1306_Reset(void) {
    /* for I2C - do nothing */
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

#elif defined(SSD1306_USE_SPI)

void ssd1306_Reset(void) {
    // CS = High (not selected)
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);

    // Reset the OLED
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_RESET); // command
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, (uint8_t *) &byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_SET); // data
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, buffer, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
#endif


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len) {
    SSD1306_Error_t ret = SSD1306_ERR;
    if (len <= SSD1306_BUFFER_SIZE) {
        memcpy(SSD1306_Buffer,buf,len);
        ret = SSD1306_OK;
    }
    return ret;
}

// Initialize the oled screen
void ssd1306_Init(void) {
    // Reset OLED
    ssd1306_Reset();

    // Wait for the screen to boot
    HAL_Delay(100);

    // Init OLED
    ssd1306_SetDisplayOn(0); //display off

    ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode
    ssd1306_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

    ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // Mirror vertically
#else
    ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
#endif

    ssd1306_WriteCommand(0x00); //---set low column address
    ssd1306_WriteCommand(0x10); //---set high column address

    ssd1306_WriteCommand(0x40); //--set start line address - CHECK

    ssd1306_SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // Mirror horizontally
#else
    ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); //--set inverse color
#else
    ssd1306_WriteCommand(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    ssd1306_WriteCommand(0xFF);
#else
    ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x1F); //
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x3F); //
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    ssd1306_WriteCommand(0xD3); //-set display offset - CHECK
    ssd1306_WriteCommand(0x00); //-not offset

    ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); //--set divide ratio

    ssd1306_WriteCommand(0xD9); //--set pre-charge period
    ssd1306_WriteCommand(0x22); //

    ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xDB); //--set vcomh
    ssd1306_WriteCommand(0x20); //0x20,0.77xVcc

    ssd1306_WriteCommand(0x8D); //--set DC-DC enable
    ssd1306_WriteCommand(0x14); //
    ssd1306_SetDisplayOn(1); //--turn on SSD1306 panel

    // Clear screen
    ssd1306_Fill(Black);
    
    // Flush buffer to screen
    ssd1306_UpdateScreen();
    
    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    
    SSD1306.Initialized = 1;
}

// Fill the whole screen with the given color
void ssd1306_Fill(SSD1306_COLOR color) {
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

// Write the screenbuffer with changed to the screen
void ssd1306_UpdateScreen(void) {
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
        ssd1306_WriteCommand(0xB0 + i); // Set the current RAM page address.
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
    }
}

//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }
   
    // Draw in the right color
    if(color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else { 
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color) {
    uint32_t i, b, j;
    
    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;
    
    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }
    
    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++) {
            if((b << j) & 0x8000)  {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            } else {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }
    
    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;
    
    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color) {
    // Write until null-byte
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }
        
        // Next char
        str++;
    }
    
    // Everything ok
    return *str;
}

// Position the cursor
void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

// Draw line by Bresenhem's algorithm
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
  int32_t deltaX = abs(x2 - x1);
  int32_t deltaY = abs(y2 - y1);
  int32_t signX = ((x1 < x2) ? 1 : -1);
  int32_t signY = ((y1 < y2) ? 1 : -1);
  int32_t error = deltaX - deltaY;
  int32_t error2;
    
  ssd1306_DrawPixel(x2, y2, color);
    while((x1 != x2) || (y1 != y2))
    {
    ssd1306_DrawPixel(x1, y1, color);
    error2 = error * 2;
    if(error2 > -deltaY)
    {
      error -= deltaY;
      x1 += signX;
    }
    else
    {
    /*nothing to do*/
    }
        
    if(error2 < deltaX)
    {
      error += deltaX;
      y1 += signY;
    }
    else
    {
    /*nothing to do*/
    }
  }
  return;
}
//Draw polyline
void ssd1306_Polyline(const SSD1306_VERTEX *par_vertex, uint16_t par_size, SSD1306_COLOR color) {
  uint16_t i;
  if(par_vertex != 0){
    for(i = 1; i < par_size; i++){
      ssd1306_Line(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y, color);
    }
  }
  else
  {
    /*nothing to do*/
  }
  return;
}
/*Convert Degrees to Radians*/
static float ssd1306_DegToRad(float par_deg) {
    return par_deg * 3.14 / 180.0;
}
/*Normalize degree to [0;360]*/
static uint16_t ssd1306_NormalizeTo0_360(uint16_t par_deg) {
  uint16_t loc_angle;
  if(par_deg <= 360)
  {
    loc_angle = par_deg;
  }
  else
  {
    loc_angle = par_deg % 360;
    loc_angle = ((par_deg != 0)?par_deg:360);
  }
  return loc_angle;
}
/*DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color) {
    #define CIRCLE_APPROXIMATION_SEGMENTS 36
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint32_t count = 0;
    uint32_t loc_sweep = 0;
    float rad;
    
    loc_sweep = ssd1306_NormalizeTo0_360(sweep);
    
    count = (ssd1306_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;
    while(count < approx_segments)
    {
        rad = ssd1306_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sin(rad)*radius);
        yp1 = y + (int8_t)(cos(rad)*radius);    
        count++;
        if(count != approx_segments)
        {
            rad = ssd1306_DegToRad(count*approx_degree);
        }
        else
        {            
            rad = ssd1306_DegToRad(loc_sweep);
        }
        xp2 = x + (int8_t)(sin(rad)*radius);
        yp2 = y + (int8_t)(cos(rad)*radius);    
        ssd1306_Line(xp1,yp1,xp2,yp2,color);
    }
    
    return;
}
//Draw circle by Bresenhem's algorithm
void ssd1306_DrawCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,SSD1306_COLOR par_color) {
  int32_t x = -par_r;
  int32_t y = 0;
  int32_t err = 2 - 2 * par_r;
  int32_t e2;

  if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
    return;
  }

    do {
      ssd1306_DrawPixel(par_x - x, par_y + y, par_color);
      ssd1306_DrawPixel(par_x + x, par_y + y, par_color);
      ssd1306_DrawPixel(par_x + x, par_y - y, par_color);
      ssd1306_DrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if(-x == y && e2 <= x) {
              e2 = 0;
            }
            else
            {
              /*nothing to do*/
            }
        }
        else
        {
          /*nothing to do*/
        }
        if(e2 > x) {
          x++;
          err = err + (x * 2 + 1);
        }
        else
        {
          /*nothing to do*/
        }
    } while(x <= 0);

    return;
}

//Draw rectangle
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
  ssd1306_Line(x1,y1,x2,y1,color);
  ssd1306_Line(x2,y1,x2,y2,color);
  ssd1306_Line(x2,y2,x1,y2,color);
  ssd1306_Line(x1,y2,x1,y1,color);

  return;
}

//Draw bitmap - ported from the ADAFruit GFX library

void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1306_COLOR color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    for (uint8_t j = 0; j < h; j++, y++) {
        for (uint8_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            if (byte & 0x80)
                ssd1306_DrawPixel(x + i, y, color);
        }
    }
    return;
}

void ssd1306_SetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    ssd1306_WriteCommand(kSetContrastControlRegister);
    ssd1306_WriteCommand(value);
}

void ssd1306_SetDisplayOn(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SSD1306.DisplayOn = 0;
    }
    ssd1306_WriteCommand(value);
}

uint8_t ssd1306_GetDisplayOn() {
    return SSD1306.DisplayOn;
}
void start_screen(void)
{
	char tmp[5] = {'M', 'o', 'd', 'e', ':'};
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'T';
	tmp[1] = 'y';
	tmp[2] = 'p';
	tmp[3] = 'e';
	ssd1306_SetCursor(0, 21);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'I';
	tmp[1] = '=';
	tmp[2] = ' ';
	tmp[3] = ' ';
	tmp[4] = ' ';
	ssd1306_SetCursor(0, 42);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'U';
	ssd1306_SetCursor(66, 42);
	ssd1306_WriteString(tmp, Font_11x18, White);
	ssd1306_UpdateScreen();
}
void online_screen(void)
{
	char tmp0[3] = {'O', 'F', 'F'};
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(tmp0, Font_11x18, White);
	char tmp[2] = {'U', '='};
	ssd1306_SetCursor(66, 0);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'I';
	ssd1306_SetCursor(0, 21);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'u';
	ssd1306_SetCursor(66, 21);
	ssd1306_WriteString(tmp, Font_11x18, White);
	tmp[0] = 'Q';
	ssd1306_SetCursor(0, 42);
	ssd1306_WriteString(tmp, Font_11x18, White);
	ssd1306_UpdateScreen();
}
void upd_mode(uint8_t numb)
{
	char mode_load[5] = {'L', 'o', 'a', 'd', ' '};
	char mode_disch[5] = {'D', 'i', 's', 'c', 'h'};
	ssd1306_SetCursor(66, 0);
	if (numb == 1)
	{
		ssd1306_WriteString(mode_load, Font_11x18, White);
	}
	if (numb == 2)
	{
		ssd1306_WriteString(mode_disch, Font_11x18, White);
	}
	ssd1306_UpdateScreen();
}
void upd_type(uint8_t numb)
{
	char type_lipo[5] = {'L', 'i', '-', 'P', 'o'};
	char type_pbibp[5] = {'P', 'b', 'I', 'B', 'P'};
	char type_pbcar[5] = {'P', 'b', 'C', 'a', 'r'};
	char type_other[5] = {'O', 't', 'h', 'e', 'r'};
	ssd1306_SetCursor(66, 21);
	if (numb == 1)
	{
		ssd1306_WriteString(type_lipo, Font_11x18, White);
	}
	if (numb == 2)
	{
		ssd1306_WriteString(type_pbibp, Font_11x18, White);
	}
	if (numb == 3)
		{
			ssd1306_WriteString(type_pbcar, Font_11x18, White);
		}
	if (numb == 4)
		{
			ssd1306_WriteString(type_other, Font_11x18, White);
		}
	ssd1306_UpdateScreen();
}


uint8_t float_to_str(float value, char *buf, size_t buf_size, uint8_t precision) {

    char format[5];
/*    format[0] = '%';
    format[1] = '.';
    format[2] = '0' + precision;
    format[3] = 'f';
    format[4] = '\0';
   */
    	format[0] = '%';
        format[1] = '4';
        format[2] = '.';
        format[3] = '0'+precision;
        format[4] = 'f';
    int result = snprintf(buf, buf_size, format, value);
    return (uint8_t)(result + 1);

}


void upd_chisl(float32_t chisl, uint8_t position)
{
	char string[4] = {0,};
	float_to_str(chisl, string, 5, 2);
	//float_to_string(string, chisl);
	if (position == 0)
	{
		ssd1306_SetCursor(18, 0);
	}
	if (position == 1)
	{
		ssd1306_SetCursor(84, 0);
	}
	if (position == 2)
	{
		ssd1306_SetCursor(18, 21);
	}
	if (position == 3)
	{
		ssd1306_SetCursor(84, 21);
	}
	if (position == 4)
	{
		ssd1306_SetCursor(18, 42);
	}
	if (position == 5)
	{
		ssd1306_SetCursor(84, 42);
	}
	ssd1306_WriteString(string, Font_11x18, White);
	ssd1306_UpdateScreen();
}
// Функция для отрисовки подчеркивания
void draw_underline(uint8_t menu_item) {
	static uint8_t prev_menu_item = 0; // Статическая переменная для храненеия предыдущего значения menu_item
	if (menu_item >= 1 && menu_item <= 4) {
		if (menu_item != prev_menu_item) {  // Проверка изменения menu_item
		uint8_t x1, y1, x2, y2; // переменные для координат линии подчеркивания
		x1 = 66;
		y1 = 19;
		x2 = 66 + 60; // ширина подчеркивания
		y2 = 19;
		ssd1306_Line(x1, y1, x2, y2, Black); // Стираем подчеркивание
		        x1 = 66;
				y1 = 39;
				x2 = 66 + 60; // ширина подчеркивания
				y2 = 39;
				ssd1306_Line(x1, y1, x2, y2, Black); // Стираем подчеркивание
				        x1 = 66;
						y1 = 59;
						x2 = 66 + 60; // ширина подчеркивания
						y2 = 59;
						ssd1306_Line(x1, y1, x2, y2, Black); // Стираем подчеркивание
						        x1 = 0;
								y1 = 59;
								x2 = 0 + 60; // ширина подчеркивания
								y2 = 59;
								ssd1306_Line(x1, y1, x2, y2, Black); // Стираем подчеркивание

		switch (menu_item) { // В зависимости от пункта меню подчеркиваем значение
		case 1:
			x1 = 66; // Координата начала подчеркивания
			y1 = 19;
			x2 = 66 + 60; // Ширина подчеркивания
			y2 = 19;
			ssd1306_Line(x1 , y1, x2, y2, White); // Рисуем подчеркивание
			break;
			case 2:
				x1 = 66; // Координата начала подчеркивания
				y1 = 39;
				x2 = 66 + 60; // Ширина подчеркивания
				y2 = 39;
				ssd1306_Line(x1 , y1, x2, y2, White); // Рисуем подчеркивание
				break;
			    case 3:
					x1 = 66; // Координата начала подчеркивания
					y1 = 59;
					x2 = 66 + 60; // Ширина подчеркивания
					y2 = 59;
					ssd1306_Line(x1 , y1, x2, y2, White); // Рисуем подчеркивание
					break;
			        case 4:
			    		x1 = 0; // Координата начала подчеркивания
			    		y1 = 59;
			    		x2 = 0 + 60; // Ширина подчеркивания
			    		y2 = 59;
			    		ssd1306_Line(x1 , y1, x2, y2, White); // Рисуем подчеркивание
			    		break;
		}
		ssd1306_UpdateScreen(); // Обновляем экран
		prev_menu_item = menu_item; // Обновляем предыдущее значение
	}
	}
}

// функция для моргания подчеркивания
void draw_blinking_underline(uint8_t menu_item) {
	static uint32_t time_underline = 0; // статическая переменная для хранения времени
	uint32_t current_time = HAL_GetTick(); // текущее время тиков
    uint8_t x1, y1, x2, y2; // Переменные для координат
    switch (menu_item) { // В зависимости от пункта меню реализуем моргалку
    case 1:
    	x1 = 66;
    	y1 = 19;
    	x2 = 66 + 60; // Ширина подчеркивания
    	y2 = 19;
    	// Если короткое нажатие активно и прошло 500мс
    	if (short_press == 1 && (current_time - time_underline >= 600)) {
    		time_underline = current_time; // обновляем время
    		ssd1306_Line(x1, y1, x2, y2, White); // рисуем подчеркивание
    	} else if (short_press == 1) {
    		ssd1306_Line(x1, y1, x2, y2, Black); // стираем подчеркивание
    	}
    	else if (short_press == 0){ // Если активное нажатие не активно вызываем функцию статичного подчеркивание
    		draw_underline(menu_item);
    	}
    	break;
    	case 2:
    		x1 = 66;
            y1 = 39;
            x2 = 66 + 60; // Ширина подчеркивания
            y2 = 39;
            // Если короткое нажатие активно и прошло 500мс
            if (short_press == 1 && (current_time - time_underline >= 600)) {
            	time_underline = current_time;  // обновляем время
                ssd1306_Line(x1, y1, x2, y2, White); // рисуем подчеркивание
            }  else if (short_press == 1) {
            	ssd1306_Line(x1, y1, x2, y2, Black); // стираем подчеркивание
            } else if (short_press == 0){ // Если активное нажатие не активно вызываем функцию статичного подчеркивание
            	draw_underline(menu_item);
            }
            break;
    	    case 3:
    	    	x1 = 66;
    	    	y1 = 59;
    	    	x2 = 66 + 60; // Ширина подчеркивания
    	    	y2 = 59;
    	    	// Если короткое нажатие активно и прошло 500мс
    	    	if (short_press == 1 && (current_time - time_underline >= 600)) {
    	    		time_underline = current_time; // обновляем время
    	    		ssd1306_Line(x1, y1, x2, y2, White); // рисуем подчеркивание
    	    	} else if (short_press == 1) {
    	    		ssd1306_Line(x1, y1, x2, y2, Black); // стираем подчеркивание
    	    	}
    	    	else if (short_press == 0){ // Если активное нажатие не активно вызываем функцию статичного подчеркивание
    	    		draw_underline(menu_item);
    	    	}
    	    	break;
    	        case 4:
    	    	x1 = 0;
    	    	y1 = 59;
    	    	x2 = 0 + 60; // Ширина подчеркивания
    	    	y2 = 59;
    	    	// Если короткое нажатие активно и прошло 500мс
    	    	if (short_press == 1 && (current_time - time_underline >= 600)) {
    	    		time_underline = current_time; // обновляем время
    	    		ssd1306_Line(x1, y1, x2, y2, White); // рисуем подчеркивание
    	    	} else if (short_press == 1) {
    	    		ssd1306_Line(x1, y1, x2, y2, Black); // стираем подчеркивание
    	    	}
    	    	else if (short_press == 0){ // Если активное нажатие не активно вызываем функцию статичного подчеркивание
    	    		draw_underline(menu_item);
    	    	}
    	    	break;
    }
    ssd1306_UpdateScreen(); // Обновляем экран
    }

void change_screen (uint8_t long_press) {
	    static uint8_t prev_long_press = 0;
	    if (long_press != prev_long_press) {
	        prev_long_press = long_press; // Обновляем предыдущее значение
	        ssd1306_Fill(Black); // Очищаем экран
	        if (long_press == 0) {
	            start_screen(); // рисуем стартовый экран
	            upd_mode(mode_item); // выводим значение сохраненного режима
	            upd_type(type_item);  // выводим значения сохраненного типа нагрузки
	        } else if (long_press == 1) {
	            online_screen(); // рисуем второй экран
	        }
	    }
	}
