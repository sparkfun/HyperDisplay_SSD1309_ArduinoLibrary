#include "HyperDisplay_SSD1309.h"



SSD1309::SSD1309(uint16_t xSize, uint16_t ySize, SSD1309_Intfc_t interface) : hyperdisplay(xSize, ySize)
{
	_intfc = interface;
	_needsRefresh = false;
	_pgRefStart = 0;
	_pgRefEnd = 0;
	_colRefStart = 0;
	_colRefEnd = 0;
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
	return base;
	// return (color_t)(((SSD1309_Bite_t*)base + numPixels/8));
}

SSD1309_Status_t		SSD1309::refreshDisplay( void )
{
	if(!_needsRefresh){ return SSD1309_NoRefresh; }

	// This function is used to re-send the most up-to-date RAM information to just a few memory locations (rather than the whole screen every time...)
	// Compute where to start in the ram array and how many bytes to send out:
	size_t offset = (SSD1309_MAX_WIDTH * _pgRefStart) + _colRefStart;
	size_t numBytes = (_colRefEnd - _colRefStart + 1) + ((_pgRefEnd - _pgRefStart) * SSD1309_MAX_WIDTH);

	// Setup the write
	setMemoryAddressingMode( 0x00 ); // Horizontal Addressing Mode
	setColumnAddress((uint8_t)_colRefStart, (uint8_t)_colRefEnd);
	setPageAddress((uint8_t)_pgRefStart, (uint8_t)_pgRefEnd);

	// Transmit the bytes
	writeBytes((uint8_t*)(ramMirror + offset), true, numBytes);

	_needsRefresh = false;
	return SSD1309_Nominal;
}

SSD1309_Status_t 		SSD1309::updateRefreshZone( hd_hw_extent_t colStart, hd_hw_extent_t colEnd, hd_hw_extent_t rowStart, hd_hw_extent_t rowEnd )
{
	hd_hw_extent_t pgEnd = rowEnd / 8;			// Convert to pages
	hd_hw_extent_t pgStart = rowStart / 8;
	if(_needsRefresh){
		// A refresh is pending from some other operation:
		// Take the largest bounds
		_pgRefStart = min(_pgRefStart, pgStart);
		_pgRefEnd = max(_pgRefEnd, pgEnd);
		_colRefStart = min(_colRefStart, colStart);
		_colRefEnd = max(_colRefEnd, colEnd);
	}else{
		_pgRefStart = pgStart;
		_pgRefEnd = pgEnd;
		_colRefStart = colStart;
		_colRefEnd = colEnd;
	}
	_needsRefresh = true;
	return SSD1309_Nominal;
}


SSD1309_Status_t		SSD1309::setMirrorPixel(hd_hw_extent_t x, hd_hw_extent_t y)
{
	uint8_t temp = (*(ramMirror + ((SSD1309_MAX_WIDTH * (y/8)) + x))).bite;
	temp |= (0x01 << (y % 8));
	(*(ramMirror + ((SSD1309_MAX_WIDTH * (y/8)) + x))).bite = temp;
	return SSD1309_Nominal;
}

SSD1309_Status_t		SSD1309::clearMirrorPixel(hd_hw_extent_t x, hd_hw_extent_t y)
{
	uint8_t temp = (*(ramMirror + ((SSD1309_MAX_WIDTH * (y/8)) + x))).bite;
	temp &= (~(0x01 << (y % 8)));
	(*(ramMirror + ((SSD1309_MAX_WIDTH * (y/8)) + x))).bite = temp;
	return SSD1309_Nominal;
}



////////////////////////////////////////////////////////////
//				    Hyperdisplay API    				  //
////////////////////////////////////////////////////////////
/*
    // void hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0); 																																	
    // void hwxline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool goLeft = false); 																					
    // void hwyline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool goUp = false); 																						
    // void hwrectangle(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, bool filled = false, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool reverseGradient = false, bool gradientVertical = false); 
    // void hwfillFromArray(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, color_t data = NULL, hd_pixels_t numPixels = 0,  bool Vh = false );	
*/
void SSD1309::hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset)
{
	if(colorCycleLength == 0){ return; }

	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset because it could be greater than the cycle length
	color_t value = getOffsetColor(data, startColorOffset);

	SSD1309_Bite_t user = *((SSD1309_Bite_t*)value);
	if(user.b0){									// Check if the user's bit is set or not (this implies that the user should always set bit 0 of a 'bite' to the pixel value they want)
		// Need to set the pixel high	
		setMirrorPixel(x0, y0);
	}else{
		// Need to clear the pixel
		clearMirrorPixel(x0, y0);
	}
	updateRefreshZone( x0, x0, y0, y0);			// Tell where we need updates
	refreshDisplay();								// Perform updates
}

// void SSD1309::hwxline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset, bool goLeft)
// {
// 	if(colorCycleLength == 0){ return; }

// 	startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 0);	// This line is needed to condition the user's input start color offset because it could be greater than the cycle length
// 	color_t value = getOffsetColor(data, startColorOffset);

// 	if(goLeft)
// 	{
// 		for(hd_hw_extent_t indi = 0; indi < len; indi++)
// 		{
// 			SSD1309_Bite_t user = *((SSD1309_Bite_t*)value);
// 			if(user.b0){ setMirrorPixel(x0 - indi, y0); }
// 			else{ clearMirrorPixel(x0 - indi, y0); }
// 			// updateRefreshZone( x0 - indi, x0, y0, y0);
// 			// startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 1);
// 			// value = getOffsetColor(data, startColorOffset);
// 		}
// 		updateRefreshZone( x0 - len + 1, x0, y0, y0);
// 	}
// 	else
// 	{
// 		for(hd_hw_extent_t indi = 0; indi < len; indi++)
// 		{
// 			SSD1309_Bite_t user = *((SSD1309_Bite_t*)value);
// 			if(user.b0){ setMirrorPixel(x0 + indi, y0); }
// 			else{ clearMirrorPixel(x0 + indi, y0); }
// 			// updateRefreshZone( x0, x0 + indi, y0, y0);
// 			// startColorOffset = getNewColorOffset(colorCycleLength, startColorOffset, 1);
// 			// value = getOffsetColor(data, startColorOffset);
// 		}
// 		updateRefreshZone( x0, x0 + len - 1, y0, y0);
// 	}
// 	refreshDisplay();								// Perform updates
// }	












// void SSD1309::hwyline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data, hd_colors_t colorCycleLength, hd_colors_t startColorOffset, bool goUp)
// {
// 	// To implement this consider writing 0xFF bytes into a single column
// }




// Functions that don't need color arguments, for simplicity.

void SSD1309::windowSet(wind_info_t* pwindow)
{
	wind_info_t* ptemp = NULL;
	SSD1309_Bite_t pix;
	pix.bite = 0xFF;

	if(pwindow != NULL){ ptemp = pCurrentWindow; pCurrentWindow = pwindow; } // Store current window and change to the desired window
	fillWindow((color_t)&pix);  
	if(ptemp != NULL){ pCurrentWindow = ptemp; } // Restore the old window, if needed
}

void SSD1309::windowClear(wind_info_t* pwindow)
{
	wind_info_t* ptemp = NULL;
	SSD1309_Bite_t pix;
	pix.bite = 0x00;

	if(pwindow != NULL){ ptemp = pCurrentWindow; pCurrentWindow = pwindow; } // Store current window and change to the desired window
	fillWindow((color_t)&pix);  
	if(ptemp != NULL){ pCurrentWindow = ptemp; } // Restore the old window, if needed
}



void SSD1309::pixelSet(hd_extent_t x0, hd_extent_t y0)
{
	SSD1309_Bite_t pix;
	pix.bite = 0xFF;
	pixel(x0, y0, (color_t)&pix);
}

void SSD1309::pixelClear(hd_extent_t x0, hd_extent_t y0)
{
	SSD1309_Bite_t pix;
	pix.bite = 0x00;
	pixel(x0, y0, (color_t)&pix);
}

void SSD1309::rectangleSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, bool filled)
{
	SSD1309_Bite_t pix;
	pix.bite = 0xFF;
	rectangle( x0, y0, x1, y1, filled,  (color_t)&pix); 
}

void SSD1309::rectangleClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, bool filled)
{
	SSD1309_Bite_t pix;
	pix.bite = 0x00;
	rectangle( x0, y0, x1, y1, filled,  (color_t)&pix);
}

#if HYPERDISPLAY_DRAWING_LEVEL > 0
    void SSD1309::lineSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, uint16_t width)
    {
    	SSD1309_Bite_t pix;
		pix.bite = 0xFF;
    	line( x0, y0, x1, y1, width, (color_t)&pix); 
    }

    void SSD1309::lineClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, uint16_t width)
    {
		SSD1309_Bite_t pix;
		pix.bite = 0x00;
    }

    void SSD1309::polygonSet(hd_extent_t x[], hd_extent_t y[], uint8_t numSides, uint16_t width)
    {
		SSD1309_Bite_t pix;
		pix.bite = 0xFF;
		polygon( x, y, numSides, width, (color_t)&pix);
    }

    void SSD1309::polygonClear(hd_extent_t x[], hd_extent_t y[], uint8_t numSides, uint16_t width)
    {
		SSD1309_Bite_t pix;
		pix.bite = 0x00;
		polygon( x, y, numSides, width, (color_t)&pix);
    }

    void SSD1309::circleSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t radius, bool filled)
    {
		SSD1309_Bite_t pix;
		pix.bite = 0xFF;
		circle( x0, y0, radius, filled, (color_t)&pix); 
    }

    void SSD1309::circleClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t radius, bool filled)
    {
		SSD1309_Bite_t pix;
		pix.bite = 0x00;
		circle( x0, y0, radius, filled, (color_t)&pix); 
    }
#endif /* HYPERDISPLAY_DRAWING_LEVEL > 0 */       



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
SSD1309_Status_t SSD1309::setLowCSAinPAM( uint8_t CSA )
{	// Note: this command should be OR'd with the desired Page Address Mode Lower Nibble of Column Start Address when it is sent
	uint8_t buff[1] = {
		(SSD1309_CMD_setLowCSAinPAM | (CSA & 0x0F))
	};
	selectDriver();
	SSD1309_Status_t retval = writeBytes(buff, false, sizeof(buff));
	deselectDriver();
	return retval;
}
 			
SSD1309_Status_t SSD1309::setHighCSAinPAM( uint8_t CSA )
{	// This command also OR'd with the high nibble...
	uint8_t buff[1] = {
		(SSD1309_CMD_setHighCSAinPAM | (CSA >> 4))
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
		(mode & 0x03)
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
	// Find nearest (lower) power of two from the I2C buffer length
	_i2cXferLen = nspoti <uint16_t> (SSD1309_I2C_BUFFER_LENGTH - 2); // We subtract two because one is for the device address (worst case) and the second is for the control byte. 
	_rst = SSD1309_ARD_UNUSED_PIN;
}

////////////////////////////////////////////////////////////
//				Display Interface Functions				  //
////////////////////////////////////////////////////////////
SSD1309_Status_t SSD1309_Arduino_I2C::writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes)
{
	uint8_t addr = SSD1309_BASE_ADDR;
	if(_sa0val){ addr |= 0x01; }

	uint16_t retval = (uint16_t)SSD1309_Nominal;

	if(DATAcmd){
		// Data is sent all in a row, using only one device address and control byte header
		uint32_t offset = 0;
		for(size_t indi = 0; indi < (numBytes / _i2cXferLen); indi++){
			_i2c->beginTransmission(addr);
			_i2c->write(0x40);
			_i2c->write(pdata, _i2cXferLen);
			offset += _i2cXferLen;
			if(_i2c->endTransmission()){
				retval |= (uint16_t)SSD1309_Error;
			}
		}
		_i2c->beginTransmission(addr);
		_i2c->write(0x40);
		_i2c->write((pdata + offset), (numBytes % _i2cXferLen));
		if(_i2c->endTransmission()){
			retval |= (uint16_t)SSD1309_Error;
		}
	}else{
		// Commands are sent one byte at a time, each with the device address and 0x00 control byte first
		for(size_t indi = 0; indi < numBytes; indi++){
			_i2c->beginTransmission(addr);
			_i2c->write(0x00);
			_i2c->write(*(pdata+indi));
			if(_i2c->endTransmission()){
				retval |= (uint16_t)SSD1309_Error;
			}
		}
	}
	return (SSD1309_Status_t)retval;
}


















////////////////////////////////////////////////////////////
//		SSD1309_Arduino_SPI_OneWay Implementation		  //
////////////////////////////////////////////////////////////
SSD1309_Arduino_SPI::SSD1309_Arduino_SPI(uint16_t xSize, uint16_t ySize) : hyperdisplay(xSize, ySize), SSD1309(xSize, ySize, SSD1309_Intfc_I2C)
{
	_rst = SSD1309_ARD_UNUSED_PIN;
}

////////////////////////////////////////////////////////////
//				Display Interface Functions				  //
////////////////////////////////////////////////////////////
SSD1309_Status_t SSD1309_Arduino_SPI::writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes)
{
	uint8_t addr = SSD1309_BASE_ADDR;	// Not used in SPI mode but still set for good practice

	SSD1309_Status_t retval = SSD1309_Nominal;

	_spi->beginTransaction(SPISettings(UG2856KLBAG01_SPI_FREQ_MAX, UG2856KLBAG01_SPI_ORDER, UG2856KLBAG01_SPI_MODE));

	if(DATAcmd){
		// In SPI mode data is indicated by the state of the _dc line so no control bytes are needed
		digitalWrite(_dc, HIGH);
		for(size_t indi = 0; indi < numBytes; indi++){
			selectDriver();
			_spi->transfer(*(pdata + indi));
			deselectDriver();
		}
	}else{
		// In SPI mode commands are indicated by the _dc line state, so no control bytes are needed
		digitalWrite(_dc, LOW);
		for(size_t indi = 0; indi < numBytes; indi++){
			selectDriver();
			_spi->transfer(*(pdata + indi));
			deselectDriver();
		}
	}
	_spi->endTransaction();
	digitalWrite(_dc, HIGH);
	return SSD1309_Nominal;
}

SSD1309_Status_t SSD1309_Arduino_SPI::selectDriver( void ){
	digitalWrite(_cs, LOW);
}
SSD1309_Status_t SSD1309_Arduino_SPI::deselectDriver( void ){
	digitalWrite(_cs, HIGH);
}