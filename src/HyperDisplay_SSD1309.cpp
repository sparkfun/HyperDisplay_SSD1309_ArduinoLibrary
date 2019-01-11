#include "HyperDisplay_SSD1309.h"



SSD1309::SSD1309(uint16_t xSize, uint16_t ySize, SSD1309_Intfc_t interface) : hyperdisplay(xSize, ySize)
{
	_intfc = interface;
}


////////////////////////////////////////////////////////////
//				Display Interface Functions				  //
////////////////////////////////////////////////////////////
/*

SSD1309_Status_t writeBytes(uint8_t * pdata, bool DATAcmd, uint32_t numBytes) = 0;	// Pure virtual
SSD1309_Status_t selectDriver( void );												// Optional virtual
SSD1309_Status_t deselectDriver( void );											// Optional virtual

*/
SSD1309_Status_t SSD1309::selectDriver( void ){}		// Default implementation is 'empty' (I mean inconsequential) so that the compiler may optimize it away 
SSD1309_Status_t SSD1309::deselectDriver( void ){}	// Default implementation is 'empty' (I mean inconsequential) so that the compiler may optimize it away 										


////////////////////////////////////////////////////////////
//				    Internal Utility Funcs 				  //
////////////////////////////////////////////////////////////
/*

color_t 	getOffsetColor(color_t base, uint32_t numPixels);	// (required by hyperdisplay)

*/
color_t 	SSD1309::getOffsetColor(color_t base, uint32_t numPixels)
{
	// This function returns an offset pointer according to the number of pixels and the _colorMode of the object
	
	// TBD
	return base;
}


////////////////////////////////////////////////////////////
//				    Hyperdisplay API    				  //
////////////////////////////////////////////////////////////
/*

void hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0); 																							// Single pixel write. Required by hyperdisplay. Uses screen-relative coordinates
void hwxline(uint16_t x0, uint16_t y0, uint16_t len, color_t data, uint16_t colorCycleLength = 1, uint16_t startColorOffset = 0, bool goLeft = false); 																					// More efficient xline imp. Uses screen-relative coordinates
void hwyline(uint16_t x0, uint16_t y0, uint16_t len, color_t data, uint16_t colorCycleLength = 1, uint16_t startColorOffset = 0, bool goUp = false); 																					// More efficient yline imp. Uses screen-relative coordinates
void hwrectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t data, bool filled = false, uint16_t colorCycleLength = 1, uint16_t startColorOffset = 0, bool gradientVertical = false, bool reverseGradient = false); 	// More efficient rectangle imp in window-relative coordinates
void hwfillFromArray(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t numPixels, color_t data); 																															// More efficient fill from array implementation. Uses screen-relative coordinates

*/
void SSD1309::hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0)
{
	hd_hw_extent_t startCol = x0;		// The way it works now the coordinates of a 'hw_____' function are already in hardware coordinates
	hd_hw_extent_t startRow = y0;

	if(data == NULL){ return; }
	if((startCol < SSD1309_START_COL) || (startCol > SSD1309_STOP_COL)){ return; }
	if((startRow < SSD1309_START_ROW) || (startRow > SSD1309_STOP_ROW)){ return; }

	// startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset
	// color_t value = getOffsetColor(data, startColorOffset);
	// write_ram(value, startRow, startCol, startRow, startCol, 1);	// Write 1 pixel to the location (startCol, startRow)
}



////////////////////////////////////////////////////////////
//				    SSD1309 Settings 				      //
////////////////////////////////////////////////////////////
/*

	SSD1309_Status_t setContrastControl(uint8_t val);
	SSD1309_Status_t overrideRam(bool on);
	SSD1309_Status_t setInversion(bool on);
	SSD1309_Status_t setPower(bool on); 
	SSD1309_Status_t nop( void );
	SSD1309_Status_t setCommandLock(bool on);

	// Scrolling Command Table
	SSD1309_Status_t contHScrollSetupRight(uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t contHScrollSetupLeft(uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t contVHScrollSetupRight(bool horiz, uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t contVHScrollSetupLeft(bool horiz, uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t setScroll(bool on);
	SSD1309_Status_t setVScrollArea(uint8_t TFA, uint8_t SA);
	SSD1309_Status_t contentScrollSetupRight(uint8_t PSA, uint8_t PEA, uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t contentScrollSetupLeft(uint8_t PSA, uint8_t PEA, uint8_t CSA, uint8_t CEA);

	// Addressing Setting Command Table
	SSD1309_Status_t setLowCSAinPAM( uint8_t low ); 			// Note: this command should be OR'd with the desired Page Address Mode Lower Nibble of Column Start Address when it is sent
	SSD1309_Status_t setHighCSAinPAM( uint8_t high );			// This command also OR'd with the high nibble...
	SSD1309_Status_t setMemoryAddressingMode( uint8_t mode );	// Only for horizontal or vertical addressing mode...
	SSD1309_Status_t setColumnAddress(uint8_t CSA, uint8_t CEA);
	SSD1309_Status_t setPageAddress(uint8_t PSA, uint8_t PEA);
	SSD1309_Status_t setPSAinPAM( uint8_t PSA ); 		// Note: OR this with 3 Least Significant bits that represent the page start address
	SSD1309_Status_t setDisplayStartLine( uint8_t DSL ); // Note: OR this with 6 Least Significant Bits that represent the display start line
	SSD1309_Status_t setSegmentMapping(bool flipped);
	SSD1309_Status_t setMultiplexRatio(	uint8_t MUXR);
	SSD1309_Status_t setCOMoutputDirection(bool flipped);
	SSD1309_Status_t setDisplayOffset(uint8_t OFST);
	SSD1309_Status_t setCOMpinsHWconfig(uint8_t CONF);
	SSD1309_Status_t setGPIO(uint8_t SET);

	// Timing and Driving Scheme Setting Command Table
	SSD1309_Status_t setDisplayClockDivideRatio(uint8_t FREQ);
	SSD1309_Status_t setPreChargePeriod(uint8_t PER);
	SSD1309_Status_t setVCOMHdeselectLevel(uint8_t LVL);

*/
SSD1309_Status_t SSD1309::setContrastControl(uint8_t val)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setContrastControl,
		val,
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::overrideRam(bool on)
{
	uint8_t buff[1] = {
		SSD1309_CMD_followRAMcontent
	};
	if(on){ buff[0] = SSD1309_CMD_allPixelsOn; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setInversion(bool on)
{
	uint8_t buff[1] = {
		SSD1309_CMD_inversionOff
	};
	if(on){ buff[0] = SSD1309_CMD_inversionOn; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setPower(bool on)
{
	uint8_t buff[1] = {
		SSD1309_CMD_pwrOff
	};
	if(on){ buff[0] = SSD1309_CMD_pwrOn; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::nop( void )
{
	uint8_t buff[1] = {
		SSD1309_CMD_nop
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setCommandLock(bool on)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setCommandLock,
		0x12
	};
	if(on){ buff[1] |= 0x04; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}


// Scrolling Command Table
SSD1309_Status_t SSD1309::contHScrollSetupRight(uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contHScrollSetupRight,
		0x00,
		(PSA & 0x07),
		(interval & 0x07),
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::contHScrollSetupLeft(uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contHScrollSetupLeft,
		0x00,
		(PSA & 0x07),
		(interval & 0x07),
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::contVHScrollSetupRight(bool horiz, uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contVHScrollSetupRight,
		0x00,
		(PSA & 0x07),
		(interval & 0x07),
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::contVHScrollSetupLeft(bool horiz, uint8_t PSA, uint8_t interval, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contVHScrollSetupLeft,
		0x00,
		(PSA & 0x07),
		(interval & 0x07),
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setScroll(bool on)
{
	uint8_t buff[1] = {
		SSD1309_CMD_deactivateScroll
	};
	if(on){ buff[0] = SSD1309_CMD_activateScroll; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setVScrollArea(uint8_t TFA, uint8_t SA)
{
	uint8_t buff[3] = {
		SSD1309_CMD_setVScrollArea,
		(TFA & 0x3F),
		(SA & 0x7F)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::contentScrollSetupRight(uint8_t PSA, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contentScrollSetupRight,
		0x00,
		(PSA & 0x07),
		0x01,
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::contentScrollSetupLeft(uint8_t PSA, uint8_t PEA, uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[8] = {
		SSD1309_CMD_contentScrollSetupLeft,
		0x00,
		(PSA & 0x07),
		0x01,
		(PEA & 0x07),
		0x00,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}


// Addressing Setting Command Table
SSD1309_Status_t SSD1309::setLowCSAinPAM( uint8_t low )
{	// Note: this command should be OR'd with the desired Page Address Mode Lower Nibble of Column Start Address when it is sent
	uint8_t buff[1] = {
		(SSD1309_CMD_setLowCSAinPAM | (low & 0x0F))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
 			
SSD1309_Status_t SSD1309::setHighCSAinPAM( uint8_t high )
{	// This command also OR'd with the high nibble...
	uint8_t buff[1] = {
		(SSD1309_CMD_setHighCSAinPAM | (high >> 4))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
			
SSD1309_Status_t SSD1309::setMemoryAddressingMode( uint8_t mode )
{
	uint8_t buff[2] = {
		SSD1309_CMD_setMemoryAddressingMode,
		(MODE & 0x03)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
	// Only for horizontal or vertical addressing mode...
SSD1309_Status_t SSD1309::setColumnAddress(uint8_t CSA, uint8_t CEA)
{
	uint8_t buff[3] = {
		SSD1309_CMD_setColumnAddress,
		CSA,
		CEA
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setPageAddress(uint8_t PSA, uint8_t PEA)
{
	uint8_t buff[3] = {
		SSD1309_CMD_setPageAddress,
		(PSA & 0x07),
		(PEA & 0x07)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setPSAinPAM( uint8_t PSA )
{	// Note: OR this with 3 Least Significant bits that represent the page start address
	uint8_t buff[1] = {
		(SSD1309_CMD_setPSAinPAM | (PSA & 0x07))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
 		
SSD1309_Status_t SSD1309::setDisplayStartLine( uint8_t DSL )
{	// Note: OR this with 6 Least Significant Bits that represent the display start line
	uint8_t buff[1] = {
		(SSD1309_CMD_setDisplayStartLine | (DSL & 0x3F))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
 
SSD1309_Status_t SSD1309::setSegmentMapping(bool flipped)
{
	uint8_t buff[1] = {
		SSD1309_CMD_setSegmentMapReset
	};
	if(flipped){ buff[0] = SSD1309_CMD_setSegmentMapFlipped; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setMultiplexRatio(	uint8_t MUXR)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setMultiplexRatio,
		(MUXR & 0x3F)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setCOMoutputDirection(bool flipped)
{
	uint8_t buff[1] = {
		SSD1309_CMD_setCOMoutputNormal
	};
	if( flipped ){ buff[0] = SSD1309_CMD_setCOMoutputFlipped; }
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setDisplayOffset(uint8_t OFST)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setDisplayOffset,
		(OFST & 0x3F)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setCOMpinsHWconfig(uint8_t CONF)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setCOMpinsHWconfig,
		(0x02 | (CONF & 0x30))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setGPIO(uint8_t SET)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setGPIO,
		(SET & 0x03)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}


// Timing and Driving Scheme Setting Command Table
SSD1309_Status_t SSD1309::setDisplayClockDivideRatio(uint8_t FREQ)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setDisplayClockDivideRatio,
		FREQ
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setPreChargePeriod(uint8_t PER)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setPreChargePeriod,
		PER
	};	
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}

SSD1309_Status_t SSD1309::setVCOMHdeselectLevel(uint8_t LVL)
{
	uint8_t buff[2] = {
		SSD1309_CMD_setVCOMHdeselectLevel,
		(LVL & 0x3C)
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}





























////////////////////////////////////////////////////////////
//		SSD1309_Arduino_SPI_OneWay Implementation		  //
////////////////////////////////////////////////////////////
SSD1309_Arduino_I2C::SSD1309_Arduino_I2C(uint16_t xSize, uint16_t ySize) : hyperdisplay(xSize, ySize), SSD1309(xSize, ySize, SSD1309_Intfc_I2C)
{

}

////////////////////////////////////////////////////////////
//				Display Interface Functions				  //
////////////////////////////////////////////////////////////
SSD1309_Status_t SSD1309_Arduino_I2C::writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes)
{


	return SSD1309_Nominal;
}








////////////////////////////////////////////////////////////
//				UG6464TDDBG01 Implementation			  //
////////////////////////////////////////////////////////////
char_info_t UG6464TDDBG01_Default_CharInfo;
wind_info_t UG6464TDDBG01_Default_Window;

UG2856KLBAG01_I2C::UG2856KLBAG01_I2C( void ) : hyperdisplay(UG2856KLBAG01_WIDTH, UG2856KLBAG01_HEIGHT), SSD1309_Arduino_I2C(UG2856KLBAG01_WIDTH, UG2856KLBAG01_HEIGHT)
{

}

SSD1309_Status_t UG2856KLBAG01_I2C::begin(TwoWire &wirePort, uint8_t saoPin, bool sa0Val)
{
	// Call the functions to setup the super classes
// Associate 
	_sa0 = saoPin;
	_i2c = &wirePort;
	_sa0val = sa0Val;

	// Set pinmodes
	if(_sa0 != SSD1309_ARD_I2C_UNUSED_PIN){ pinMode(_sa0, OUTPUT); }

	// Set pins to default positions
	if(_sa0 != SSD1309_ARD_I2C_UNUSED_PIN){ digitalWrite(_sa0, HIGH); }

	_i2c->begin();

	// Setup the default window
	setWindowDefaults(pCurrentWindow);

	// Power up the device
	startup();	
	defaultConfigure();

	return SSD1309_Nominal;
}

SSD1309_Status_t UG2856KLBAG01_I2C::defaultConfigure( void )
{
	// This is the suggested initialization routine from WiseChip (pg. 9 of the datasheet)
	

  	return SSD1309_Nominal;
}

void UG2856KLBAG01_I2C::startup( void )
{
	// Assume that VDD and VCC are stable when this function is called
	delay(20);
	digitalWrite(_rst, LOW);
	// delayMicroseconds(10);
	delay(5);
	digitalWrite(_rst, HIGH);
	delay(200);
	// Now you can do initialization
}

// void UG6464TDDBG01::getCharInfo(uint8_t val, char_info_t * pchar)
// {
// 	char_info_t * pcharinfo = &UG6464TDDBG01_Default_CharInfo;
// 	// Do stuff to fill out the default char info structure, then return a pointer to it
// 	pcharinfo->data = NULL;						// Color information for each pixel to be drawn
// 	pcharinfo->xLoc = NULL;						// X location wrt upper-left corner of char location(cursor) for each pixel
// 	pcharinfo->yLoc = NULL;						// Y location wrt upper-left corner of char location(cursor) for each pixel
//     pcharinfo->numPixels = 0;					// Number of pixels contained in the data, xLoc, and yLoc arrays
// 	pcharinfo->show = false;					// Whether or not to actually show the character
// 	pcharinfo->causedNewline = false;			// Whether or not the character triggered/triggers a new line
// }


void UG2856KLBAG01_I2C::clearDisplay( void )
{
	// Store the old window pointer: 
	wind_info_t * ptempWind = pCurrentWindow;

	// Make a new default window
	wind_info_t window;
	pCurrentWindow = &window;

	// Ensure the window is set up so that you can clear the whole screen
	setWindowDefaults(&window);

	// Make a local 'black' color 
	uint8_t temp_buff[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};				// This buffer is long enough to represent '0' no matter what color mode is selected
	
	// Fill the temporary window with black
	fillWindow((color_t)temp_buff);												// Pass the address of the buffer b/c we know it will be safe no matter what SSD1309 color mode is used

	// Restore the previous window
	pCurrentWindow = ptempWind;
}

void UG2856KLBAG01_I2C::setWindowDefaults(wind_info_t * pwindow)
{
	// // Fills out the default window structure with more or less reasonable defaults
	// pwindow->xMin = UG6464TDDBG01_START_COL;
	// pwindow->yMin = UG6464TDDBG01_START_ROW;
	// pwindow->xMax = UG6464TDDBG01_STOP_COL;
	// pwindow->yMax = UG6464TDDBG01_STOP_ROW;
	// pwindow->cursorX = 0;							// cursor values are in window coordinates
	// pwindow->cursorY = 0;
	// pwindow->xReset = 0;
	// pwindow->yReset = 0;
	
	// pwindow->lastCharacter.data = NULL;
	// pwindow->lastCharacter.xLoc = NULL;
	// pwindow->lastCharacter.yLoc = NULL;
	// pwindow->lastCharacter.xDim = 0;
	// pwindow->lastCharacter.yDim = 0;
	// pwindow->lastCharacter.numPixels = 0;
	// pwindow->lastCharacter.show = false;
	// pwindow->lastCharacter.causesNewline = false;
	
	// pwindow->data = NULL;				// No window data yet
	// setCurrentWindowColorSequence(NULL, 1, 0);	// Setup the default color (Which is NULL, so that you know it is not set yet)
}