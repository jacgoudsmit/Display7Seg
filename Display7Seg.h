//***************************************************************************
// Display7Seg library
//
// Copyright (C) 2019 Jac Goudsmit
//
// Licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
// 
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
//***************************************************************************


#ifndef DISPLAY7SEG_H
#define DISPLAY7SEG_H

#include <Arduino.h>

// Implement the _countof macro if the C++ compiler doesn't know it yet
#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif


//***************************************************************************
// Multi-Digit 7-Segment Display Driver
//
// This class allows the control of a multi-digit seven-segment display.
// Displays with a decimal point for each digit are also supported.
//
// The class assumes that the display consists of a number of digits, each
// with 7 or 8 segments (the decimal points are the 8th segment, if that
// feature is in use). Each segment is an LED that's connected between one
// of the common segment lines and one of the common digit lines. The
// Arduino is connected to the seven or eight segment lines, and the digit
// lines for each digit (there should be current-limiting resistors to
// prevent the LEDs from being destroyed!).
//
// So the LEDs are in a matrix between digit-pins and segment-pins. In order
// to show a different pattern on each digit, it's necessary to show the
// patterns for each digit individually, and switch rapidly between the
// digits (in theory it's also possible to cycle over the segments instead of
// the digits but that's not how we do it).
//
// The class keeps an internal state of all the segments that should be lit
// for the entire display. Each digit is represented by one byte in an
// internal array, and each segment in a digit is represented by a bit: the
// least significant bit is the "a" segment and the most significant bit
// is the decimal point segment (if used).
//
// The functions in the class are roughly divided into two purposes:
// - The "show" functions read the state and update the output pins
// - The "get" and "set" functions manipulate the internal state, and
//   also influence the way the display is updated (e.g. blanking).
//
// After constructing an instance of the class and calling begin() to
// initialize the output pins, your program should call the show() function
// on a regular basis to show the next digit on the display. It can be
// called from a timer callback function or from the loop() function if you
// allow the loop() function to be called often enough. How often you call
// the show() function is not critical, but for the best result, the entire
// display should be updated at least 25 times per second. That means for
// a 4-digit display the function should be called at least 100 times per
// second (4 * 25).
//
// The state of the segments can be changed anytime, either directly (by
// storing the bit pattern that represents the segments for a particular
// digit) or through helper functions that format the display from other
// data such as a number.
//
// To use a display, create an instance that specifies the number of digits
// and (optionally), whether the display has decimal points, and what the
// output pin values should be to turn the digits and segments on.
// From the setup() function in your program, call the begin() function to
// initialize the pins that are used to control the digits and segments.
// In your loop() function, call show() to light up the next digit.
// You may also call the show() from a timer callback function but
// this is not implemented in the library.
template <
  unsigned numdigits,                   // Number of digits; 1 or higher
  bool havedp = false,                  // True if display has decimal point
  bool digiton = true,                  // Pin value to turn a digit on
  bool segmenton = true>                // Pin value to turn a segment on
class Display7Seg
{
  // Pin numbers initialized at begin() time
  byte              _digitpins[numdigits];
                                        // Pin numbers for each digit
  byte              _segmentpins[havedp ? 8 : 7];
                                        // Pin numbers for each segment

  // Current state
  volatile byte     _display[numdigits];
                                        // LEDs represented by bits
                                        // lsb=a, msb=dp, 0=off, 1=on
  volatile byte     _curdigit;
                                        // Digit that's currently visible
                                        // Out of range=blank

public:
  //-------------------------------------------------------------------------
  // Constructor
  Display7Seg()
  {
    // Turn all segments in all digits off in the state
    for (unsigned i = 0; i < _countof(_display); i++)
    {
      _display[i] = 0;
    }
  }

public:
  //-------------------------------------------------------------------------
  // Call this from setup() to initialize the pin numbers
  void begin(
    const byte digitpins[],             // Digit pin numbers (left to right)
    const byte segmentpins[],           // Segment pin numbers (a to g, dp)
    bool blank = false)                 // True = blank display
  {
    // Copy the array of digit pins, set the corresponding pins to
    // output mode, and switch the pins off.
    for (unsigned i = 0; i < _countof(_digitpins); i++)
    {
      byte p = digitpins[i];

      _digitpins[i] = p;

      pinMode(p, OUTPUT);
      digitalWrite(p, !digiton);
    }

    // Copy the array of segment pins, set the corresponding pins to
    // output mode, and switch the pins off.
    for (unsigned i = 0; i < _countof(_segmentpins); i++)
    {
      byte p = segmentpins[i];

      _segmentpins[i] = p;

      pinMode(p, OUTPUT);
      digitalWrite(p, !segmenton);
    }

    setBlank(blank);
  }

protected:
  //-------------------------------------------------------------------------
  // Set the output pins to show the state of the given digit
  //
  // The display is blanked if the input parameter is out of range.
  //
  // The internal state of the given digit is transfered to the display.
  void showDigit(
    unsigned newdigit)                  // Digit to show; 0=leftmost
  {
    // If the display is already blanked, don't change the output pins
    if (_curdigit < _countof(_digitpins))
    {
      // Turn the current digit off if we're changing to a different digit.
      if (newdigit != _curdigit)
      {
        digitalWrite(_digitpins[_curdigit], !digiton);
      }

      // Change the segment outputs according to the state of the new digit.
      // If the display is being blanked (new digit is out of range), set
      // turn all the segment output pins off.
      byte b = (newdigit < _countof(_digitpins)) ? _display[newdigit] : 0;
      for (unsigned i = 0; i < _countof(_segmentpins); i++, b >>= 1)
      {
        digitalWrite(_segmentpins[i], (b & 1) ? segmenton : !segmenton);
      }
    }

    // Turn the new digit on
    if (newdigit < _countof(_digitpins))
    {
      digitalWrite(_digitpins[newdigit], digiton);
    }

    // Set the current digit to the new value
    _curdigit = newdigit;
  }

public:
  //-------------------------------------------------------------------------
  // Set the output pins to show the state of the next digit.
  //
  // This is intended to be called repeatedly, from the loop() function, or
  // from a timer callback function.
  //
  // To prevent flickering, you should call the function often enough to
  // show each digit at least about 25 times per second. So for a 4 digit
  // display this should be called at least 100 times (4 * 25) per second.
  void show()
  {
    // Don't do anything if the display is blank
    if (_curdigit < _countof(_digitpins))
    {
      unsigned nextdigit = _curdigit + 1;
      
      // If we're currently displaying the last digit, start at the top
      if (nextdigit >= _countof(_digitpins))
      {
        nextdigit = 0;
      }

      showDigit(nextdigit);
    }
  }

public:
  //-------------------------------------------------------------------------
  // Allow direct access to the display
  //
  // This makes it possible to access the bits for each digit directly.
  // The function guards against index values that are out of range; if an
  // out of range index is used, data will be read from and written to a
  // static dummy variable.
  //
  // Example:
  //
  //    Simple7Seg<4> display;
  //    ...
  //    // Show "0" on the leftmost display digit
  //    display[0] = B00111111; // "0"
  volatile byte &                       // Returns reference to byte in state
  operator[](
    unsigned index)                     // Digit number; 0=leftmost
  {
    // Scratch pad variable used when index is out of range
    static byte dummy;

    // Access the state if the index is in range, otherwise access the dummy.
    return (index < _countof(_display)) ? _display[index] : dummy;
  }

public:
  //-------------------------------------------------------------------------
  // Set display blanking on or off
  //
  // The display is updated immediately. The public functions will not
  // change any output pins while the display is blanked, but if a "show"
  // override is used on any of the set-functions, blanking is disabled.
  void setBlank(
    bool onoff)                         // True = blank, False = show
  {
    showDigit(onoff ? _countof(_display) : 0);
  }

public:
  //-------------------------------------------------------------------------
  // Check if display is blanked
  bool                                  // Returns True if display blanked
  isBlank()
  {
    return _curdigit >= _countof(_display);
  }

public:
  //-------------------------------------------------------------------------
  // Set the segments of a digit to a given value
  //
  // Optionally, update the display immediately. If an update is requested
  // while the display is disabled, the display is re-enabled.
  //
  // If the digit index is out of range, nothing happens.
  void setSegments(
    unsigned index,                     // Digit number; 0=lefmost
    byte bitmap,                        // Bit pattern to store
    bool show = false)                  // True to update display
  {
    if (index < _countof(_display))
    {
      _display[index] = bitmap;

      if (show)
      {
        showDigit(index);
      }
    }
  }

public:
  //-------------------------------------------------------------------------
  // Set a digit to a decimal or hexadecimal value
  //
  // Optionally, update the display immediately. If an update is requested
  // while the display is disabled, the display is re-enabled.
  //
  // If any values are out of range, nothing happens.
  void setNumber(
    unsigned index,                     // Digit number; 0=leftmost
    byte num,                           // (Hexa)decimal number to display
    bool dp = false,                    // Decimal point on or off
    bool show = false)                  // True to update display
  {
    if ((index < _countof(_digitpins)) && (num <= 16))
    {
      static const byte table[16] = {
        B00111111, // 0
        B00000110,
        B01011011,
        B01001111,
        B01100110,
        B01101101,
        B01111101,
        B00000111,
        B01111111,
        B01101111,

        B01110111, // A
        B01111100,
        B00111001,
        B01011110,
        B01111001,
        B01110001,
      };

      setSegments(index, table[num] | (dp ? B10000000 : 0), show);
    }
  }

public:
  //-------------------------------------------------------------------------
  // Set the entire display state to show a numeric value
  template <unsigned radix = 10, typename T = unsigned>
  bool                                  // Returns false on overflow
  setValue(
    T value,                            // Value to show
    bool leadingzero = false,           // True to show leading zeros
    unsigned dppos = _countof(_display)) // Position of decimal point
  {
    for (unsigned i = _countof(_display); i-- > 0;)
    {
      // Calculate the last digit
      byte v = value % radix;

      // Display the calculated digit unless it's 0 and the value is
      // 0 and no leading zeros are requested and we're not on the last
      // position.
      if ((v) || (value) || (leadingzero) || (i >= dppos) || (i == _countof(_display) - 1))
      {
        setNumber(i, v, dppos == i);
      }
      else
      {
        setSegments(i, dppos == i ? B10000000 : 0);
      }

      value /= radix;
    }

    return !value;
  }
};

#endif // DISPLAY7SEG_H
