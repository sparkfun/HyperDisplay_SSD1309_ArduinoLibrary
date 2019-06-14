/*

A library to control SSD1309 OLED drivers, derived from the SparkFun HyperDisplay library

Created: August 2018
Modified: August 2018

Authors: Owen Lyke

*/

#ifndef HYPERDISPLAY_SSD1309_H
#define HYPERDISPLAY_SSD1309_H


////////////////////////////////////////////////////////////
//							Includes    				  //
////////////////////////////////////////////////////////////
#include "hyperdisplay.h"		// Inherit drawing functions from this library	
#include "Arduino.h"
#include "Wire.h"				// Arduino I2C support
#include "SPI.h"				// Arduino SPI support

////////////////////////////////////////////////////////////
//							Defines     				  //
////////////////////////////////////////////////////////////
#define SSD1309_MAX_WIDTH 	128
#define SSD1309_MAX_HEIGHT 	64
#define SSD1309_START_ROW 	0
#define SSD1309_START_COL 	0
#define SSD1309_START_PG 	0
#define SSD1309_STOP_ROW 	63
#define SSD1309_STOP_COL 	127
#define SSD1309_STOP_PG		7

#define SSD1309_RAM_BYTES (SSD1309_MAX_WIDTH*(SSD1309_MAX_HEIGHT/8))

// #define SSD1309_DEFAULT_WORKING_NUM_BYTES 2*64



template <typename T>
T nspoti(T i1){					// Nearest smaller power of two for integers
	T retval = i1;				// Be explicit about passing by value
	T bytesize = sizeof( T );	// Number of bytes (assuming 8 bits) that the type occupies
	retval--; 					// In case we start on a power of two
	if(retval == 0){ return 0; }
	if(bytesize > 0){
		retval |= (retval >> 1);
		retval |= (retval >> 2);
		retval |= (retval >> 4);
	}
	if(bytesize > 1){
		retval |= (retval >> 8);
	}
	if(bytesize > 3){
		retval |= (retval >> 16);
	}
	if(bytesize > 4){
		return 0;
	}
	retval++;				// This will carry a one into the next highest bit - we now have the nearest greater power of two (thanks http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float)
	retval = retval >> 1; 	// This divides by two to get the nearest lower power of two
	// retval += (retval == 0); // If you needed you could add this line to make sure that 0 is not returned
	return retval;
}



////////////////////////////////////////////////////////////
//							Typedefs    				  //
////////////////////////////////////////////////////////////
typedef enum{
	SSD1309_Nominal = 0,
	SSD1309_Error,
	SSD1309_NoRefresh,
}SSD1309_Status_t;

typedef enum{
	SSD1309_Intfc_4WSPI = 0,
	SSD1309_Intfc_3WSPI,
	SSD1309_Intfc_I2C,
	SSD1309_Intfc_8080,
	SSD1309_Intfc_6800,
}SSD1309_Intfc_t;		// This lists the possible interface modes of the SSD1309. The mode is selected by strapping pins BS[2:0] and the binary representations of these enum values corresponds to those strapping pins

typedef union{
	struct{
		uint8_t b0 : 1;	// Bit 0
		uint8_t b1 : 1; // Bit 1
		uint8_t b2 : 1; // Bit 2
		uint8_t b3 : 1; // Bit 3
		uint8_t b4 : 1; // Bit 4
		uint8_t b5 : 1; // Bit 5
		uint8_t b6 : 1; // Bit 6
		uint8_t b7 : 1; // Bit 7
	};
	uint8_t bite; // All bits arranged in order
}SSD1309_Bite_t;	// The basic (minimal) type that holds pixel information 

typedef enum{
	// Fundamental Commands
	SSD1309_CMD_setContrastControl = 0x81,
	SSD1309_CMD_followRAMcontent = 0xA4,
	SSD1309_CMD_allPixelsOn = 0xA5,
	SSD1309_CMD_inversionOff = 0xA6,
	SSD1309_CMD_inversionOn = 0xA7,
	SSD1309_CMD_pwrOff = 0xAE, 
	SSD1309_CMD_pwrOn = 0xAF,
	SSD1309_CMD_nop = 0xE3,
	SSD1309_CMD_setCommandLock = 0xFD,

	// Scrolling Command Table
	SSD1309_CMD_contHScrollSetupRight = 0x26,
	SSD1309_CMD_contHScrollSetupLeft = 0x27,
	SSD1309_CMD_contVHScrollSetupRight = 0x29,
	SSD1309_CMD_contVHScrollSetupLeft = 0x2A,
	SSD1309_CMD_deactivateScroll = 0x2E,
	SSD1309_CMD_activateScroll = 0x2F,
	SSD1309_CMD_setVScrollArea = 0xA3,
	SSD1309_CMD_contentScrollSetupRight = 0x2C,
	SSD1309_CMD_contentScrollSetupLeft = 0x2D,

	// Addressing Setting Command Table
	SSD1309_CMD_setLowCSAinPAM = 0x00, 		// Note: this command should be OR'd with the desired Page Address Mode Lower Nibble of Column Start Address when it is sent
	SSD1309_CMD_setHighCSAinPAM = 0x10,		// This command also OR'd with the high nibble...
	SSD1309_CMD_setMemoryAddressingMode = 0x20, // Only for horizontal or vertical addressing mode...
	SSD1309_CMD_setColumnAddress = 0x21,
	SSD1309_CMD_setPageAddress = 0x22,
	SSD1309_CMD_setPSAinPAM = 0xB0, 		// Note: OR this with 3 Least Significant bits that represent the page start address
	SSD1309_CMD_setDisplayStartLine = 0x40, // Note: OR this with 6 Least Significant Bits that represent the display start line
	SSD1309_CMD_setSegmentMapReset = 0xA0,
	SSD1309_CMD_setSegmentMapFlipped = 0xA1,
	SSD1309_CMD_setMultiplexRatio = 0xA8,	
	SSD1309_CMD_setCOMoutputNormal = 0xC0,
	SSD1309_CMD_setCOMoutputFlipped = 0xC8,
	SSD1309_CMD_setDisplayOffset = 0xD3,
	SSD1309_CMD_setCOMpinsHWconfig = 0xDA,
	SSD1309_CMD_setGPIO	= 0xDC,

	// Timing and Driving Scheme Setting Command Table
	SSD1309_CMD_setDisplayClockDivideRatio = 0xD5,
	SSD1309_CMD_setPreChargePeriod = 0xD9,
	SSD1309_CMD_setVCOMHdeselectLevel = 0xDB,
}SSD1309_Cmd_t;

////////////////////////////////////////////////////////////
//					 Class Definition   				  //
////////////////////////////////////////////////////////////
class SSD1309 : virtual public hyperdisplay{
private:
protected:

	SSD1309(uint16_t xSize, uint16_t ySize, SSD1309_Intfc_t interface);

public: // temporary

	SSD1309_Bite_t 		ramMirror[(SSD1309_MAX_WIDTH*(SSD1309_MAX_HEIGHT/8))];		// Yes, a screen buffer exists for each display object. Normally this would not fly but a) only 128*64 bits are needed to fill the screen and b) knowledge of the memory contents is required to allow for single pixel access 

	SSD1309_Intfc_t		_intfc;		// The interface mode being used					

	bool				_needsRefresh;
	hd_hw_extent_t		_pgRefStart, _pgRefEnd;
	hd_hw_extent_t		_colRefStart, _colRefEnd;

	// This function defines how you talk to the driver, and it is very implementation-specific (e.g. depends on the interface mode, among others)
	virtual SSD1309_Status_t writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes) = 0;	// Pure virtual forces implementation in derived classes
	virtual SSD1309_Status_t selectDriver( void );												// Allows the user to implement a device-select function (does not force you to though)
	virtual SSD1309_Status_t deselectDriver( void );											// Allows the user to implement a device-deselect function

	// These are internally available utility functions
	color_t 				getOffsetColor(color_t base, uint32_t numPixels);	// (required by hyperdisplay)
	SSD1309_Status_t		refreshDisplay( void );
	SSD1309_Status_t 		updateRefreshZone( hd_hw_extent_t colStart, hd_hw_extent_t colEnd, hd_hw_extent_t rowStart, hd_hw_extent_t rowEnd );
	SSD1309_Status_t		setMirrorPixel(hd_hw_extent_t x, hd_hw_extent_t y);
	SSD1309_Status_t		clearMirrorPixel(hd_hw_extent_t x, hd_hw_extent_t y);

public:

	// Here is the main API implementation that allows this class to hook into the hyperdisplay library
    void hwpixel(hd_hw_extent_t x0, hd_hw_extent_t y0, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0); 																																	// Single pixel write. Required by hyperdisplay. Uses screen-relative coordinates
    // void hwxline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool goLeft = false); 																						// More efficient xline imp. Uses screen-relative coordinates
    // void hwyline(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t len, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool goUp = false); 																							// More efficient yline imp. Uses screen-relative coordinates
    // void hwrectangle(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, bool filled = false, color_t data = NULL, hd_colors_t colorCycleLength = 1, hd_colors_t startColorOffset = 0, bool reverseGradient = false, bool gradientVertical = false); 	// More efficient rectangle imp in window-relative coordinates
    // void hwfillFromArray(hd_hw_extent_t x0, hd_hw_extent_t y0, hd_hw_extent_t x1, hd_hw_extent_t y1, color_t data = NULL, hd_pixels_t numPixels = 0,  bool Vh = false );	 																																									// More efficient fill from array implementation. Uses screen-relative coordinates


    // Functions that don't need color arguments, for simplicity.
	
	void windowSet(wind_info_t* pwindow = NULL);
	void windowClear(wind_info_t* pwindow = NULL);
    void pixelSet(hd_extent_t x0, hd_extent_t y0);
    void pixelClear(hd_extent_t x0, hd_extent_t y0);
    void rectangleSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, bool filled = false); 
    void rectangleClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, bool filled = false); 
    #if HYPERDISPLAY_DRAWING_LEVEL > 0
        void lineSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, uint16_t width = 1); 
	    void lineClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t x1, hd_extent_t y1, uint16_t width = 1); 
	    void polygonSet(hd_extent_t x[], hd_extent_t y[], uint8_t numSides, uint16_t width = 1);
	    void polygonClear(hd_extent_t x[], hd_extent_t y[], uint8_t numSides, uint16_t width = 1);
	    void circleSet(hd_extent_t x0, hd_extent_t y0, hd_extent_t radius, bool filled = false); 
	    void circleClear(hd_extent_t x0, hd_extent_t y0, hd_extent_t radius, bool filled = false); 
    #endif /* HYPERDISPLAY_DRAWING_LEVEL > 0 */        

	// Here are all of the settings you can change on the SSD1309 - they use the writeBytes API 
	// Fundamental Commands
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
	SSD1309_Status_t setLowCSAinPAM( uint8_t CSA ); 			// Note: this command should be OR'd with the desired Page Address Mode Lower Nibble of Column Start Address when it is sent
	SSD1309_Status_t setHighCSAinPAM( uint8_t CSA );			// This command also OR'd with the high nibble...
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
};




////////////////////////////////////////////////////////////
//				Examples of Derived Classes    			  //
////////////////////////////////////////////////////////////

/*

These classes are included as a demonstration of how to use the
SSD1309 superclass to derive more specific implementations. The
SSD1309 class imposes limits based on the maximum dimensions 
that the driver can handle (128 x 64) and allows for several
different data bus interfaces.

Let's consider defining an Arduino I2C class for the 1309 below:

To create a class for this display we first created a derived 
class for Arduino I2C that is derived from SSD1309. This 
allowed for definitin of the control pins and an I2C port, as
well as to implement the required (by SSD1309) hooks:
- writeBytes
- selectDrver
- deselectDriver

The Arduino I2C class has now defined the communication
type but has not constrained the display size and location. 
Using a final derived class (based on the WiseChip UG-2856KLBAG01 screen) 
these parameters were built-in to the begin function. The begin function 
also calls functions to set up the required parameters from the 
two main superclasses (hyperdisplay and SSD1309). Additionally
in this final class we implemented the pure-virtual function
'getCharInfo.' This completes the class so that it is no longer
of an abstract type - this means that you can actually
instantiate an object of this class. 

*/


// Here are a few implementation-specific classes that can be used on the appropirate system
#define SSD1309_ARD_UNUSED_PIN 0xFF


////////////////////////////////////////////////////////////
//					Arduino I2C Class   	 			  //
////////////////////////////////////////////////////////////

#define SSD1309_BASE_ADDR 0b0111100 // Or this with 0x01 if _sa0val true to get the 7-bit I2C address

#ifdef BUFFER_LENGTH										// For some reason Arduino chose BUFFER_LENGTH to mean the I2C buffer length in Wire.h, so  if that is defined we will *assume* thats what it means
	#define SSD1309_I2C_BUFFER_LENGTH BUFFER_LENGTH
#else
	#define SSD1309_I2C_BUFFER_LENGTH 32					// In case BUFFER_LENGTH is not defined we will fall back to 32
#endif

class SSD1309_Arduino_I2C: public SSD1309{									// General for use with Arduino using I2C with arbitrary display size
private:
protected:

	SSD1309_Arduino_I2C(uint16_t xSize, uint16_t ySize);	// Interface is removed from parameters because it is constrained to I2C in this derived class

	size_t 		_i2cXferLen;

	uint8_t 	_rst, _sa0;		// Pin definitions
	bool 		_sa0val;
	TwoWire * 	_i2c;			// Which I2C port to use

public:
	SSD1309_Status_t writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes);
	// SSD1309_Status_t selectDriver( void );
	// SSD1309_Status_t deselectDriver( void );
};










////////////////////////////////////////////////////////////
//					Arduino SPI Class   	 			  //
////////////////////////////////////////////////////////////
#define UG2856KLBAG01_SPI_FREQ_MAX 	5000000	// once tested up to 10 MHz
#define UG2856KLBAG01_SPI_MODE 		SPI_MODE3
#define UG2856KLBAG01_SPI_ORDER		MSBFIRST

class SSD1309_Arduino_SPI: public SSD1309{									// General for use with Arduino using I2C with arbitrary display size
private:
protected:

	SSD1309_Arduino_SPI(uint16_t xSize, uint16_t ySize);	// Interface is removed from parameters because it is constrained to I2C in this derived class

	uint8_t 	_rst, _cs, _dc;		// Pin definitions
	SPIClass * 	_spi;				// Which I2C port to use

public:
	SSD1309_Status_t writeBytes(uint8_t * pdata, bool DATAcmd, size_t numBytes);
	SSD1309_Status_t selectDriver( void );
	SSD1309_Status_t deselectDriver( void );
};


#endif /* HYPERDISPLAY_SSD1309_H */
