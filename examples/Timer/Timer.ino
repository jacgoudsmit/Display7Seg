//***************************************************************************
// Timer example for the Display7Seg Library
//
// Shows the time in minutes.seconds.deciseconds on a 4-digit display
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


#include <Display7Seg.h>

// Declare the display instance
// The class uses a template for maximum efficiency
Display7Seg<
  4,                // Number of digits
  true              // True=The digits have decimal points
  //,true           // Value to write to digit pins to turn them on
  //,true           // Value to write to segment pins to turn them on
  > display;

void setup()
{
  // Define the pins that are used to control the digits and the
  // segments
  const byte digits[]   = {5, 4, 3, 2};
  const byte segments[] = {13, 12, 11, 10, 9, 8, 7, 6};

  // Initialize the display. This sets all the pins to digital
  // output mode and sets all pins to the "off" value as defined
  // by the construction template.
  // By default, the display is then enabled to make the first
  // digit from the state inside the object visible, but if you're
  // not ready, you can override that with an extra parameter to
  // blank the display.
  // When the display is blanked, the show() function will not
  // change any output pins. Blanking can be used if the pins are
  // also in use by other hardware and you don't want to disturb
  // that hardware.
  display.begin(digits, segments);
}

void loop()
{
  static unsigned prevm = millis();
  static unsigned counter = 0;
  unsigned m = millis();

  // Every 100 milliseconds, update the value on the display
  if (m - prevm > 100)
  {
    // Increase the counter
    counter++;

    // If the last three digits are "600" (i.e. 60.0 seconds),
    // add an additional 400 to the counter to increase the minutes.
    if (counter % 1000 == 600)
    {
      counter += 400;
    }

    // If the counter is too big to fit on a 4 digit display,
    // reset it to zero.
    if (counter == 9999)
    {
      counter = 0;
    }

    // Format the display based on the value
    // This changes the internal state stored in the display
    // object, so that the segments that need to be on are set to
    // one and the segments that need to be set to off are set to
    // zero. By default, the function uses decimal numbers but you
    // can override the radix by using e.g.:
    //   display.setValue<16>(counter, true, 2); // Hexadecimal
    // By default, the function uses an unsigned integer to do its
    // calculations but for large displays (more than 4 digits), you
    // may want to use an unsigned long because an unsigned 16-bit
    // integer can't hold values larger than 65535.
    //   display.setValue<10,unsigned long>(counter, true, 2); // >4 digits
    // If you don't want to show a decimal point, set the position
    // parameter to the number of digits in the display.
    display.setValue(
      counter,      // Value to encode into the state of the display
      false,        // True to show leading zeros (default=false)
      2);           // Pos of decimal point (default = none; 0=leftmost)

    // We also want to show a decimal point after the minutes, so
    // we set the most significant bit of the first digit's segment
    // bits without affecting the other bits that have just been set
    // by the setValue function.
    if (counter >= 1000)
    {
      display[0] |= B10000000;
    }

    // Store the current time stamp
    prevm = m;
  }

  // The show() function should be called on a regular basis to update
  // the display hardware from the internal state. Every time you call
  // the function, the segments of a single digit light up. If you don't
  // call the function often enough, you will see flickering.
  // You will probably want to call the function fast enough to update
  // the entire display at least about 25 to 30 times per second. That
  // means for a 4-digit display you should call the function at least
  // 100 times per second (25 * 4). In some situations, it may be a
  // good idea to call the show() function from a timer callback,
  // but in this sample sketch we just call it from the loop() function
  // which gets called plenty often enough.
  display.show();
}
