/**
 * @Author: emrecanozkok@gmail.com
 **/

#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>


class Display {
  private:
    LiquidCrystal_I2C * lcd;
  public:
    Display();
    void begin();
    void status(const char*);
    void notify(const char*);
};

#endif