#ifndef _CIRCULARBUFFER_H_
#define _CIRCULARBUFFER_H_

// TODO: Consider INLINE for several functions - many small, only used in one place, in frequently used code

/* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         count;    /* new  */
} CircularBuffer;

void cbInit(CircularBuffer *cb, int size);

int cbIsFull(CircularBuffer *cb);

int cbIsEmpty(CircularBuffer *cb);

// returns index of next element to write
int cbGetNextWrite(CircularBuffer *cb);

// mark next element as written
void cbWrite(CircularBuffer *cb);

// returns index of next element to read
int cbGetNextRead(CircularBuffer *cb);

// marks next element as read
void cbRead(CircularBuffer *cb);


#endif // _CIRCULARBUFFER_H_
