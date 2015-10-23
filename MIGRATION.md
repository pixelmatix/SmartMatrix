# Migrating from SmartMatrix 2.x to SmartMatrix 3.0
SmartMatrix 3.0 has separated out the single SmartMatrix class into a core class for refreshing the display, and separate layer classes for storing data.  The library is not backwards compatible with sketches created for SmartMatrix 2.x, but by following this document it should be relatively easy to update your sketches to get access to the new features.  You can have SmartMatrix3 installed in parallel with an existing SmartMatrix_32x32 or SmartMatrix_16x32 library without conflicts.

## Updating Normal SmartMatrix Sketch

### `#Include`
The Library is now named "SmartMatrix3" to avoid conflicts with the previous versions of the library.  Replace the `#include` at the top of the sketch.

```
#include <SmartMatrix_32x32.h>
```

replace with:

```
#include <SmartMatrix3.h>
```
At this point if you try to compile you'll get a lot of errors like this:
```
'class SmartMatrix' has no member named 'setScrollOffsetFromEdge'
```

or like this:
```
'MATRIX_WIDTH' was not declared in this scope
```

### Allocation
This single line needs to be replaced with a large section, as your sketch can now set a lot of options instead of being limited to the defaults of the library.

```
SmartMatrix matrix;
```

Replace with:

```
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);
```

The options are the defaults for a 32x32 pixel panel, similar defaults to the SmartMatrix_32x32 library.  If you are using a 16x32 panel, change `kMatrixHeight` to 16, and `kPanelType` to `SMARTMATRIX_HUB75_16ROW_MOD8SCAN`.  

At this point if you try to compile, you'll get a lot of errors like:

```
'class SmartMatrix3<36, 32, 32, 0u, 0u>' has no member named 'setScrollOffsetFromEdge'
```

### Setup()
The layers allocated at the top of the sketch need to be added before calling `matrix.begin()`.

```
matrix.begin()
```

replace with:

```
  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.addLayer(&indexedLayer); 
  matrix.begin();
```

If you don't need all the layers, delete the `addLayer` call for the ones you don't need, and delete the corresponding `SMARTMATRIX_ALLOCATE` macros at the top of the sketch.

### Background Layer
The methods for drawing to the background layer were moved from the SmartMatrix class to the background layer class, called `backgroundLayer` by default.  Do find-replace in your text editor, finding the text on the first line of each pair, and replacing with the text from the second line.

```
matrix.setBackgroundBrightness
backgroundLayer.setBrightness
```
```
matrix.swapBuffers
backgroundLayer.swapBuffers
```
```
matrix.drawPixel
backgroundLayer.drawPixel
```
```
matrix.fillScreen
backgroundLayer.fillScreen
```
```
matrix.setColorCorrection
backgroundLayer.enableColorCorrection
```
```
matrix.drawLine
backgroundLayer.drawLine
```
```
matrix.drawCircle
backgroundLayer.drawCircle
```
```
matrix.drawTriangle
backgroundLayer.drawTriangle
```
```
matrix.drawRectangle
backgroundLayer.drawRectangle
```
```
matrix.drawRoundRectangle
backgroundLayer.drawRoundRectangle
```
```
matrix.fillCircle
backgroundLayer.fillCircle
```
```
matrix.fillTriangle
backgroundLayer.fillTriangle
```
```
matrix.fillRectangle
backgroundLayer.fillRectangle
```
```
matrix.fillRoundRectangle
backgroundLayer.fillRoundRectangle
```
```
matrix.fillCircle
backgroundLayer.fillCircle
```
```
matrix.drawEllipse
backgroundLayer.drawEllipse
```
```
matrix.drawFastVLine
backgroundLayer.drawFastVLine
```
```
matrix.drawFastHLine
backgroundLayer.drawFastHLine
```
```
matrix.drawString
backgroundLayer.drawString
```
```
matrix.setFont
backgroundLayer.setFont
```
```
matrix.drawChar
backgroundLayer.drawChar
```
```
matrix.drawMonoBitmap
backgroundLayer.drawMonoBitmap
```
```
matrix.setBackgroundBrightness
backgroundLayer.setBrightness
```
```
matrix.readPixel
backgroundLayer.readPixel
```
```
matrix.backBuffer
backgroundLayer.backBuffer
```

### Scrolling layer (formerly "Foreground")
The methods for drawing to the scrolling text layer (formerly called "Foreground" when only two layers were possible) were moved from the SmartMatrix class to the scrolling layer class, called `scrollingLayer` by default.  Do find-replace in your text editor, finding the text on the first line of each pair, and replacing with the text from the second line.

```
matrix.getScrollStatus
scrollingLayer.getStatus
```
```
matrix.setScrollStartOffsetFromLeft
scrollingLayer.setStartOffsetFromLeft
```
```
matrix.updateScrollText
scrollingLayer.update
```
```
matrix.stopScrollText
scrollingLayer.stop
```
```
matrix.setScrollOffsetFromTop
scrollingLayer.setOffsetFromTop
```
```
matrix.setScrollOffsetFromEdge
scrollingLayer.setOffsetFromTop
```
```
matrix.scrollText
scrollingLayer.start
```
```
matrix.setScrollColor
scrollingLayer.setColor
```
```
matrix.setScrollMode
scrollingLayer.setMode
```
```
matrix.setScrollSpeed
scrollingLayer.setSpeed
```
```
matrix.setScrollFont
scrollingLayer.setFont
```

### Indexed Layer (formerly part of Foreground)
It's unlikely you used these methods as they were not documented or included in examples.

It's more complicated than using find and replace to update these as the order and number of some of the arguments changed.  Use the class definition in Layer_Indexed.h as a reference.

**Find/Replace**

```
matrix.setForegroundFont
indexedLayer.setFont
```
```
matrix.displayForegroundDrawing
indexedLayer.swapBuffers
```
```
matrix.drawForegroundPixel
indexedLayer.drawPixel
```

**Update more carefully, see notes**

fillScreen(index) replaces clearForeground() - use 0 to fill with transparent (equivalent to clearing the screen):
```
matrix.clearForeground()
indexedLayer.fillScreen(0)
```

new function to set color of layer (before was setForegroundColor which is now used by scrolling layer), use 1 for index:
```
setIndexedColor(1, RGB)
```

order of arguments changed, swap `char` and `opaque`, optionally replace `true` with 1 for index:

```
matrix.drawForegroundChar
indexedLayer.drawChar
```

order changed, swap `string` and `opaque`, optionally replace `true` with 1 for index:
```
matrix.drawForegroundString
indexedLayer.drawString
```

order changed, swap `bitmap` and `opaque`, optionally replace `true` with 1 for index:
```
matrix.drawForegroundMonoBitmap
indexedLayer.drawMonoBitmap
```

## Troubleshooting
**Code Compiles and runs but the screen is blank**

Make sure the calls to matrix.addLayers() before matrix.begin() are added


## Update FastLED Sketch
FastLED's `SMART_MATRIX` controller was based on SmartMatrix 2.x, and is incompatible with SmartMatrix 3.0.  You can get similar features by using FastLED's helper functions but drawing to the SmartMatrix background layer directly instead of through FastLED.  [Let us know](http://community.pixelmatix.com) if you want to see support for the FastLED `SMART_MATRIX` controller in the future.

### `#Include`
Start by updating the #include to `<SmartMatrix3.h>`

If you try to compile you'll now get errors like
```
FastLED_ControllerMigration.ino:18:23: error: 'MATRIX_WIDTH' was not declared in this scope
```

### Allocation
Add the Allocation code to your sketch, following FastLED_Functions as an example of where it goes:
```
#define COLOR_DEPTH 24                  // This sketch and FastLED uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
```



If your sketch has a `#define` for `kMatrixWidth` and `kMatrixHeight`, remove those definitions as they are now a constant in the allocation section.

### Setup
```
  LEDS.addLeds<SMART_MATRIX>(leds,NUM_LEDS);
```

replace with:
```
  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.begin();
```

At a minimum you just need `backgroundLayer` for sketches using FastLED.  The FastLED_Functions example adds scrolling text on top of the FastLED graphics.  If you don't need scrolling text, delete the `addLayer` call for scrollingLayer, and delete the corresponding `SMARTMATRIX_ALLOCATE_SCROLLING_LAYER` macro at the top of the sketch.

### Replace `LEDS` Methods
Find any code refering to `LEDS` and replace it.

**setBrightness**
```
LEDS.setBrightness
```

If you want to control the brightness of the background layer only (keeping the scrolling text layer bright), replace with:
```
backgroundLayer.setBrightness
```

If you want to control the brightness of the entire panel, replace with:
```
matrix.setBrightness
```

**LEDS.show**
```
LEDS.show();
```

replace with:
```
backgroundLayer.swapBuffers();
```

**LEDS.countFPS**
```
LEDS.countFPS();
```

delete, or replace with:
```
matrix.countFPS();
```

### Use SmartMatrix Methods Directly
For any code that starts with `pSmartMatrix->`, search for the same function beginning with "matrix." in the sections above and replace.  e.g. 
```
pSmartMatrix->setScrollMode
```

replace with:
```
scrollingLayer.setMode
```

### Update leds
Find the `CRGB leds` array and delete it (backgroundLayer will hold the pixel data now)
```
CRGB leds[kMatrixWidth * kMatrixHeight];
```

Add this line at the top of `loop()` to get a pointer to the current draw buffer in the background layer:

```
  rgb24 *buffer = backgroundLayer.backBuffer();
```

Now `buffer` can be used like the `leds` array.  For example:

```
      leds[XY(i,j)] = CHSV(noise[j][i],255,noise[i][j]);
```

replace with:
```
      buffer[XY(i,j)] = CHSV(noise[j][i],255,noise[i][j]);
```

This won't work quite yet, you'll get an error like:
```
no known conversion for argument 1 from 'CHSV' to 'const rgb24&'
```

There's no automatic conversion from FastLED's `CHSV` to `rgb24`, so help the compiler out by wrapping any `CHSV` values in `CRGB()`.
```
      buffer[XY(i,j)] = CRGB(CHSV(noise[j][i],255,noise[i][j]));
```

Keep in mind that you need to get an updated pointer from `backBuffer()` after every call to `swapBuffers()`.  If you call `swapBuffers()` more than once per pass through `loop()`, make sure to update the `buffer` pointer by calling `backBuffer()` after each swap, not just at the top of `loop()`.
