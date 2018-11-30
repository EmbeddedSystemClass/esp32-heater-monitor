/**
 * Simple.cpp
 *
 *  Created on: Sep 27, 2018
 *       Email: ipas.th@gmail.com
 *      Author: ipas
 */

#include "SimpleArray.h"

/**
 * Constructors / Destructor
 */
template <class T, class SZ>
SimpleArray<T, SZ>::SimpleArray(SZ initSize) {
    bufferLength = 0;
    bufferSize   = initSize;
    buffer       = new T[bufferSize];
}

template <class T, class SZ>
SimpleArray<T, SZ>::~SimpleArray() {
    delete[] buffer;
}

/**
 * Accessing / Properties
 */
template <class T, class SZ>
SZ SimpleArray<T, SZ>::length(void) {
    return bufferLength;
}

template <class T, class SZ>
SZ SimpleArray<T, SZ>::size() {
    return bufferSize;
}

template <class T, class SZ>
T & SimpleArray<T, SZ>::operator [] (SZ index) {
    return buffer[index];
}

template <class T, class SZ>
T * SimpleArray<T, SZ>::getBuffer(void) {
    return buffer;
}

template <class T, class SZ>
SZ SimpleArray<T, SZ>::getBytes(void * buf, SZ size) {
    SZ i;
    for (i = 0; i < bufferLength && i < size; i++) {
        ((T *)buf)[i] = buffer[i];
    }
    return i;  // actual read bytes
}

/**
 * Comparators
 */
template <class T, class SZ>
int SimpleArray<T, SZ>::compare(SimpleArray<T, SZ> & array) {
    SZ i;
    for (i = 0; i < bufferLength && i < array.length(); i++) {
        if (buffer[i] != array[i])
            break;
    }
    if (bufferLength >= array.length())
        return bufferLength - i;  // How many items left, NOT be compared nor equal
    else
        return array.length() - i;
}

template <class T, class SZ>
bool SimpleArray<T, SZ>::operator == (SimpleArray<T, SZ> & array) {
    return ((bufferLength == array.length()) && (compare(array) == 0));
}

template <class T, class SZ>
bool SimpleArray<T, SZ>::operator != (SimpleArray<T, SZ> & array) {
    return !(*this == array);
}

/**
 * Manipulators
 */
template <class T, class SZ>
SZ SimpleArray<T, SZ>::append(T item) {
    if (bufferLength < bufferSize) {
        buffer[bufferLength++] = item;
        return 1;
    }
    return 0;
}

template <class T, class SZ>
SZ SimpleArray<T, SZ>::append(SimpleArray<T, SZ> & array) {
    SZ i;
    for (i = 0; i < array.length() && i < bufferSize; i++) {
        if (append(array[i]) < 1)
            break;
    }
    return i;
}

template <class T, class SZ>
SZ SimpleArray<T, SZ>::append(void * buf, SZ count) {
    SZ i;
    for (i = 0; i < count && i < bufferSize; i++) {
        if (append(((T *)buf)[i]) < 1)
            break;
    }
    return i;
}

template <class T, class SZ>
void SimpleArray<T, SZ>::operator += (T item) {
    append(item);
}

template <class T, class SZ>
void SimpleArray<T, SZ>::operator += (SimpleArray<T, SZ> & array) {
    append(array);
}

template <class T, class SZ>
void SimpleArray<T, SZ>::operator = (T item) {
    clear();
    append(item);
}

template <class T, class SZ>
void SimpleArray<T, SZ>::operator = (SimpleArray<T, SZ> & array) {
    clear();
    append(array);
}

template <class T, class SZ>
SZ SimpleArray<T, SZ>::remove(SZ index, SZ count) {
    if (index >= bufferLength)
        return 0;
    if (index+count >= bufferLength)
        count = bufferLength - index;

    T * tmp = new T[bufferSize];
    SZ i, j = 0;
    for (i = 0; i < index; i++, j++) {
        tmp[j] = buffer[i];
    }
    for (i = index+count; i < bufferLength; i++, j++) {
        tmp[j] = buffer[i];
    }
    delete[] buffer;
    buffer = tmp;
    bufferLength = j;
    return count;
}

template <class T, class SZ>
void SimpleArray<T, SZ>::clear() {
    bufferLength = 0;
}
