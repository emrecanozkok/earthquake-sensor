/*
Author: emrecanozkok@gmail.com
*/

#include "Display.h"


//LiquidCrystal_I2C lcd(0x27,16,2);

Display::Display(){
  this->lcd = new LiquidCrystal_I2C(0x27,16,2);
  this->lcd->init();
  return;
}
void Display::begin(){
 this->lcd->begin(16,2);
 this->lcd->backlight();
}

void Display::status(const char c[]){
  this->lcd->setCursor(0,0);
  this->lcd->print(c);
}
void Display::notify(const char c[]){
  this->lcd->setCursor(0,1);
  this->lcd->print(c);
}