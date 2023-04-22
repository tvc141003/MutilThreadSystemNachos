#include "syscall.h"
#include "copyright.h"

int main() {
    int i;
    char c;
    for(i = 0; i<256; i++) {
        c = i;
        PrintInt(i);
        PrintString(": ");
        PrintChar(c);
        PrintString(",\t");
        if (i % 16 == 0) PrintString("\n");
    }

    Halt();
}
