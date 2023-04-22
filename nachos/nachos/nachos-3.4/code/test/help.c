#include "syscall.h"
#include "copyright.h"

int main() {
	PrintString("\t21127021 - Truong Van Chi\n");
	PrintString("\t21127121 - Phan Van Nguyen\n");
	PrintString("\t21127190 - Le Huu Trong\n");
	PrintString("\t21127290 - Nguyen Gia Hung\n");
	PrintString("\t21127373 - Le Thanh Khoi Nguyen\n");
    PrintString("========Tutorial of ascii and bubble sort programs========\n");
    PrintString("1. Ascii\n");
    PrintString("\tPrint a table ascii. A special character can't view true.\n");
    PrintString("\tCommand: ./userprog/nachos -rs 1023 -x ./test/ascii\n");
    PrintString("2. Bubble sort\n");
    PrintString("\tInput number elements of array\n");
    PrintString("\tInput all elements\n");
    PrintString("\tPrint a array after sort by bubble sort\n");
    PrintString("\tCommand: ./userprog/nachos -rs 1023 -x ./test/bubblesort\n");

    Halt();
}
