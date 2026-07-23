#include <iostream>
#include <cstdint>
#include <iomanip>

int main(){
    uint8_t array[256];
    
    //setw(2): sets length of all hex values to 2
    //setfill('0'): uses 0 as filler character for single digit hex values
    //Must cast array[i] to int or else it would pring a character as uint is a typedef for unsigned char2
    for (int i = 0; i < 256; i++){
        array[i] = i;
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)array[i] << "\n";
    }
}