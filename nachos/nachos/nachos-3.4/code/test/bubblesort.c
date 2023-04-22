#include "syscall.h"
#include "copyright.h"

int main() {
    int n;
    int numbers[100];
    int i, j, tmp;

    PrintString("Input number of array: ");
    n = ReadInt();

    //PrintInt(n);
    for(i = 0; i<n; i++) {
        PrintString("Input element ");
        PrintInt(i);
        PrintString(": ");
        numbers[i] = ReadInt();
    }

    //sort
    for (i = 0 ; i < n ; i++ ){
        for (j = n - 1; j > 0; j--){
            if (numbers[j] < numbers[j-1]) {
                tmp = numbers[j];
                numbers[j] = numbers[j -1];
                numbers[j - 1] = tmp;
            }
        }
    }

    PrintString("--------Result--------\n");
    for( j = 0; j<n; j++) {
        PrintInt(numbers[j]);
        PrintString(" ");
    }
    PrintString("\n");

    Halt();
}
