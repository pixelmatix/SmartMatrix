#ifndef _SMARTMATRIX_CIRCULARBUFFER_H_
#define _SMARTMATRIX_CIRCULARBUFFER_H_

// TODO: Consider INLINE for several functions - many small, only used in one place, in frequently used code

/* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         count;    /* new  */
} CircularBuffer_SM;

void cbInit(CircularBuffer_SM *cb, int size);

int cbIsFull(CircularBuffer_SM *cb);

int cbIsEmpty(CircularBuffer_SM *cb);

// returns index of next element to write
int cbGetNextWrite(CircularBuffer_SM *cb);

// mark next element as written
void cbWrite(CircularBuffer_SM *cb);

// returns index of next element to read
int cbGetNextRead(CircularBuffer_SM *cb);

// marks next element as read
void cbRead(CircularBuffer_SM *cb);


#endif // _SMARTMATRIX_CIRCULARBUFFER_H_
