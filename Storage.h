/**
 * @Author: emrecanozkok@gmail.com
 **/

#ifndef STORAGE_H
#define STORAGE_H

#include <SPIFFS.h>

class Storage {
  
  public:
    Storage();
    void write(String,String);
    String read(String);

    void writeFile(fs::FS&, const char*, const char*);
    String readFile(fs::FS&, const char*);
    
};

#endif