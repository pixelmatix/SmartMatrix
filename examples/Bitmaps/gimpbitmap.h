#ifndef _GIMPBITMAP_
#define _GIMPBITMAP_

// this structre can hold up to a 32x32 pixel RGB bitmap
typedef struct  {
  unsigned int   width;
  unsigned int   height;
  unsigned int   bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char  pixel_data[32*32*3 + 1];
} gimp32x32bitmap;

#endif
