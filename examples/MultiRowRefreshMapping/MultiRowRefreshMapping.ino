/*
 * This example can be used to determine the mapping of pixels onto a panel that refreshes multiple rows in parallel.
 * e.g. each RGB channel on a HUB75 P10 32x16 /2 panel fills pixels across four rows at once, and in a non-linear order
 * 
 * The example code moves a single pixel across the width of the display across one row at a time.  To help reverse-
 * engineer the mapping of a panel, we can "unwrap" the panel so a /2 panel looks like a 4-row high panel
 * (/2 means the panel is addressed with two addresses, times two RGB channels in a HUB75 pinout 2 x 2 = 4 rows)
 * or a /4 panel looks like an 8-row high panel (4 address x 2 parallel channels = 8 rows)
 * 
 * Example for a 32x16 /2 panel:
 * Under the "MODE_MAP_REVERSE_ENGINEERING" section, set:
 * kMatrixHeight = 4 (why? see above)
 * kMatrixWidth = (32x16)/kMatrixHeight = 128
 * kPanelType = SMARTMATRIX_HUB75_4ROW_MOD2SCAN
 * Set SKETCH_MODE to  MODE_MAP_REVERSE_ENGINEERING
 * Run the sketch, and record the output on the panel with video, so it's easy to scroll through the positions of pixels to analyze
 * You should see a red pixel move to 128 different locations on the panel, followed by a yellow pixel moving to 128 different locations, etc.
 * Use the recording of the red pixel positions to generate a map on graphing paper, the size of the actual panel
 * Starting with the first pixel that is shown, record "0" at the X/Y coordinates the pixel appeared on the panel
 * Continue until the last red pixel "127" is recorded
 * You can save some time by only recording the numbers of the first and last pixel in a row, and connecting them with an arrow
 * See example photo in this sketch directory
 * From the drawing, record the groupings of pixels from left-to-right, then top-to-bottom to generate a map for your panel, in this format:
 * {rowOffset, pixelOffset, numPixels/direction}
 * rowOffset = the row this group of pixels appears, offset from 0
 * pixelOffset = the order this pixel showed up when displayed by the sketch starting from 0(the number you wrote down in the graph paper)
 * numPixels/direction = the number of continuous pixels in this group, a positive number for left-to-right order, negative for right-to-left
 * The last row of the map is all zeros
 * Map corresponding to this example panel and the example photo:
 *   {0, 71,  -8},
 *   {0, 87,  -8},
 *   {0, 103, -8},
 *   {0, 119, -8},
 *   {2, 72,   8},
 *   {2, 88,   8},
 *   {2, 104,  8},
 *   {2, 120,  8},
 *   {4,  7,  -8},
 *   {4, 23,  -8},
 *   {4, 39,  -8},
 *   {4, 55,  -8},
 *   {6,  8,   8},
 *   {6, 24,   8},
 *   {6, 40,   8},
 *   {6, 56,   8},
 *   {0, 0, 0}   // last entry is all zeros
 * Add this map and panel definition to SmartMatrix Library
 *
 * How to add a map and new panel config to SmartMatrix Library
 * - Follow the format in PanelMaps.cpp, and add your map with a unique name
 * - Open SmartMatrixMultiplexedCommon.h, add a new definition at the top for your panel.  Give it the format
 * - SMARTMATRIX_HUB75_NUMROW_NUMCOL_MODNSCAN filling in NUMROW, NUMCOL, MODNSCAN 
 * - Add entries for your new panelType to the CONVERT_PANELTYPE_TO_... Definitions
 *   - CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT - height of your panel
 *   - CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET - HUB75 panels fill two rows in parallel, what's the spacing?  (normally half of panel height)
 *   - CONVERT_PANELTYPE_TO_MATRIXSCANMOD - This is just the MOD_N_SCAN value for your panel
 *   - CONVERT_PANELTYPE_TO_MATRIXPANELWIDTH - What's the width of your panel? (This doesn't have to be exact for non-multi-row-scan panels, 32 is used by default)
 *   - CONVERT_PANELTYPE_TO_MATRIXPHYSICALROWSPERREFRESHROW - how many physical rows get lit up with one address?  (how many rows are in the diagram you made above?)
 * - Open PanelMaps.h
 * - Add new case for your new panelType, returning your new panelMap
 * - Now test your new panelType with the MODE_MAP_TESTING mode of this sketch
 * 
 * Testing Example 32x16 /2 Panel:
 * Now this same sketch can be used to check if the mapping is being applied correctly to the panel
 * Change the width/height/paneltype to match the actual panel (not unwrapped)
 * kMatrixWidth = 32
 * kMatrixHeight = 16
 * kPanelType = SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN
 * Set SKETCH_MODE to  MODE_MAP_TESTING
 * Now when the sketch runs, you should see the pixel go from left to right, top to bottom, covering all the pixels on your panel in the correct order
 *
 * How do I know if I have a /2 or /4 panel?
 *   TBD - check the model number of your panel, look at the HUB75 silkscreen (can't always trust), run a test sketch, are all pixels displayed? (note AB-panel exception)
 */

//#include <SmartLEDShieldV4.h>  // uncomment this line for SmartLED Shield V4 (needs to be before #include <SmartMatrix3.h>)
#include <SmartMatrix3.h>

#define MODE_MAP_REVERSE_ENGINEERING  0
#define MODE_MAP_TESTING              1

#define SKETCH_MODE   MODE_MAP_REVERSE_ENGINEERING
//#define SKETCH_MODE   MODE_MAP_TESTING

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24

#if (SKETCH_MODE == MODE_MAP_REVERSE_ENGINEERING)
const uint8_t kMatrixWidth = 128;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 4;       // known working: 32, 64, 96, 128
const uint8_t kPanelType = SMARTMATRIX_HUB75_4ROW_MOD2SCAN;   // Use this to reverse engineer mapping for a MOD2 panel
//const uint8_t kPanelType = SMARTMATRIX_HUB75_8ROW_MOD4SCAN;   // Use this to reverse engineer mapping for a MOD4 panel
#endif

#if (SKETCH_MODE == MODE_MAP_TESTING)
const uint8_t kMatrixWidth = 32;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 16;       // known working: 32, 64, 96, 128
const uint8_t kPanelType = SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
#endif

const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

void setup() {
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  matrix.setBrightness(128);

  // do a (normally unnecessary) swapBuffers call to work around ESP32 bug where first swap is ignored
  backgroundLayer.swapBuffers();   
}

void loop() {
  for(int j=0; j<kMatrixHeight; j++) {
    rgb24 colorByHeight;

    colorByHeight.red = 0xff;
    if(j & 0x01)
      colorByHeight.green = 0xff;
    if(j & 0x02)
      colorByHeight.blue = 0xff;
      
    for(int i=0; i<kMatrixWidth; i++) {
      backgroundLayer.drawPixel(i,j,colorByHeight);
      backgroundLayer.swapBuffers();   
      delay(250);  

      backgroundLayer.fillScreen({0,0,0});
    }
  }
}
