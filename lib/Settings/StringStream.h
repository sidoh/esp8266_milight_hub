/*
 * Adapated from https://gist.github.com/cmaglie/5883185
 */

#ifndef _STRING_STREAM_H_INCLUDED_
#define _STRING_STREAM_H_INCLUDED_

#include <Stream.h>

class StringStream : public Stream
{
public:
    StringStream(String &s) : string(s), position(0) { }

    // Stream methods
    virtual int available() { return string.length() - position; }
    virtual int read() { return position < string.length() ? string[position++] : -1; }
    virtual int peek() { return position < string.length() ? string[position] : -1; }
    virtual void flush() { };
    // Print methods
    virtual size_t write(uint8_t c) { string += (char)c; return 1; };

private:
    String &string;
    unsigned int length;
    unsigned int position;
};

#endif // _STRING_STREAM_H_INCLUDED_