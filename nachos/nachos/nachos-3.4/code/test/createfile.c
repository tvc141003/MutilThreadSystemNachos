#include "syscall.h"
#include "copyright.h"
#define maxlen 32
int main()
{
	int len;
	char filename[maxlen +1];
/*Create a file*/
	PrintString("Input file name: ");
	ReadString(filename, maxlen);

	if (Create(filename) == -1)
	{
	// xuất thông báo lỗi tạo tập tin
	//printf("xuất thông báo lỗi tạo tập tin");

		PrintString("Unsuccess!");
	}
	else
	{
	// xuất thông báo tạo tập tin thành công
	//printf("xuất thông báo tạo tập tin thành công");
		PrintString("Success!");
	}

	Halt();
}
