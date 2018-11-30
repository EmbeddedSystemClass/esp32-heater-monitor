/**
 * Simple Array Class
 *
 *  Created on: Sep 27, 2018
 *      Author: iPAS <ipas.th@gmail.com>
 *
 * Inspired by https://gist.github.com/daeltar/90951
 */

#ifndef __SIMPLE_ARRAY_H__
#define __SIMPLE_ARRAY_H__

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

template <class T, class SZ>
class SimpleArray {

  private:
    SZ bufferLength;
    SZ bufferSize;
    T * buffer;

  public:
    SimpleArray(SZ initSize);
    ~SimpleArray();

    SZ length(void);
    SZ size(void);

    T & operator [] (SZ index);
    T * getBuffer(void);

    SZ getBytes(void * buf, SZ size);  // 'size' is the maximum size of 'buf'.

    int  compare(SimpleArray<T, SZ> & array);
    bool operator == (SimpleArray<T, SZ> & array);
    bool operator != (SimpleArray<T, SZ> & array);

    SZ append(T item);
    SZ append(SimpleArray<T, SZ> & array);
    SZ append(void * buf, SZ count);
    void operator += (T item);
    void operator += (SimpleArray<T, SZ> & array);
    void operator = (T item);
    void operator = (SimpleArray<T, SZ> & array);

    SZ remove(SZ index, SZ count);
    void clear();
};

// To solve 'undefined' problem
// https://stackoverflow.com/questions/14914129/c-undefined-reference-to-template-class-method
template class SimpleArray<uint8_t, uint8_t>;
template class SimpleArray<uint8_t, int>;

#endif  // __SIMPLE_ARRAY_H__
