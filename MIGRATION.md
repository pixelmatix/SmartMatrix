
SmartMatrix3 has separted out the single SmartMatrix class into a core class for refreshing the display, and layers for storing data.  If you get an error like this, you need to use find+replace to update your code to use the new class and method names.

"error: 'class SmartMatrix3<48, 0u>' has no member named"

## backgroundLayer - search for first line of pair, replace with second
matrix.setBackgroundBrightness
backgroundLayer.setBrightness

matrix.swapBuffers
backgroundLayer.swapBuffers

matrix.drawPixel
backgroundLayer.drawPixel

matrix.fillScreen
backgroundLayer.fillScreen

matrix.enableColorCorrection
backgroundLayer.enableColorCorrection

matrix.drawLine
backgroundLayer.drawLine

matrix.drawCircle
backgroundLayer.drawCircle

matrix.drawTriangle
backgroundLayer.drawTriangle

matrix.drawRectangle
backgroundLayer.drawRectangle

matrix.drawRoundRectangle
backgroundLayer.drawRoundRectangle

matrix.fillCircle
backgroundLayer.fillCircle

matrix.fillTriangle
backgroundLayer.fillTriangle

matrix.fillRectangle
backgroundLayer.fillRectangle

matrix.fillRoundRectangle
backgroundLayer.fillRoundRectangle

matrix.fillCircle
backgroundLayer.fillCircle

matrix.drawEllipse
backgroundLayer.drawEllipse

matrix.drawFastVLine
backgroundLayer.drawFastVLine

matrix.drawFastHLine
backgroundLayer.drawFastHLine

matrix.drawString
backgroundLayer.drawString

matrix.setFont
backgroundLayer.setFont

matrix.drawChar
backgroundLayer.drawChar

matrix.drawMonoBitmap
backgroundLayer.drawMonoBitmap

matrix.setBackgroundBrightness
backgroundLayer.setBrightness

matrix.readPixel
backgroundLayer.readPixel


## scrolling layer (formerly foreground) - search for first line of pair, replace with second
matrix.getScrollStatus
scrollingLayer.getStatus

matrix.setScrollStartOffsetFromLeft
scrollingLayer.setStartOffsetFromLeft

matrix.updateScrollText
scrollingLayer.update

matrix.stopScrollText
scrollingLayer.stop

matrix.setScrollOffsetFromTop
scrollingLayer.setOffsetFromTop

matrix.scrollText
scrollingLayer.start

matrix.setScrollColor
scrollingLayer.setColor

matrix.setScrollMode
scrollingLayer.setMode

setScrollSpeed
scrollingLayer.setSpeed

matrix.setScrollFont
scrollingLayer.setFont



## indexed layer (formerly part of foreground) - more complicated

### search for first line of pair, replace with second
matrix.setForegroundFont
indexedLayer.setFont

matrix.displayForegroundDrawing
indexedLayer.swapBuffers

matrix.drawForegroundPixel
indexedLayer.drawPixel

### Update more carefully
fillScreen(index) replaces clearForeground - use 0 to fill with transparent (clear)
matrix.clearForeground()
indexedLayer.fillScreen(0)

new function to set color of layer (before was setForegroundColor which is now used by scrolling layer), use 1 for index
setIndexedColor(1, RGB)

order of arguments changed, swap `char` and `opaque`, optionally replace `true` with 1 for index
matrix.drawForegroundChar
indexedLayer.drawChar

order changed, swap `string` and `opaque`, optionally replace `true` with 1 for index
matrix.drawForegroundString
indexedLayer.drawString

order changed, swap `bitmap` and `opaque`, optionally replace `true` with 1 for index
matrix.drawForegroundMonoBitmap
indexedLayer.drawMonoBitmap
