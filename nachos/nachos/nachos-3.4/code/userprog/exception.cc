// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
char* User2System(int virtAddr,int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit +1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf,0,limit+1);
	//printf("\n Filename u2s:");
	for (i = 0 ; i < limit ;i++)
	{
		machine->ReadMem(virtAddr+i,1,&oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr,int len,char* buffer)
{
 	if (len < 0) return -1;
 	if (len == 0)return len;
 	int i = 0;
 	int oneChar = 0 ;
	do{
 		oneChar= (int) buffer[i];
 		machine->WriteMem(virtAddr+i,1,oneChar);
 		i ++;
 	}while(i < len && oneChar != 0);
 	return i;
} 

void IncreasePC()
{
	//int temp=machine->ReadRegister(PCReg);
	//machine->WriteRegister(PrevPCReg, temp);
	//temp=machine->ReadRegister(NextPCReg);
	//machine->WriteRegister(PCReg, temp);
	//machine->WriteRegister(NextPCReg, temp+4);
	int pcAfter=machine->ReadRegister(PCReg)+4;
	machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
						// are jumping into lala-land
    	machine->registers[PCReg] = machine->registers[NextPCReg];
    	machine->registers[NextPCReg] = pcAfter;

}


void ExceptionHandler(ExceptionType which)
{
 int type = machine->ReadRegister(2);

    switch(which)
    {
        case NoException:
              	return;
	case PageFaultException:
		DEBUG('a', "No valid translation found\n");
		printf("\n\nNo valid translation found\n");
   		interrupt->Halt();
		break;
	case ReadOnlyException:
		DEBUG('a', "Write attempted to page marked read-only\n");
		printf("\n\nWrite attempted to page marked read-only\n");
   		interrupt->Halt();
		break;	
	case BusErrorException:
		DEBUG('a', "Translation resulted in an invalid physical address\n");
		printf("\n\nTranslation resulted in an invalid physical address\n");
   		interrupt->Halt();
		break;
	case AddressErrorException:
		DEBUG('a', "Unaligned reference or one that was beyond the end of the address space\n");
		printf("\n\nUnaligned reference or one that was beyond the end of the address space\n");
   		interrupt->Halt();
		break;
	case OverflowException:
		DEBUG('a', "Integer overflow in add or sub.\n");
		printf("\n\nInteger overflow in add or sub.\n");
		interrupt->Halt();
		break;
	case IllegalInstrException:
		DEBUG('a', "Unimplemented or reserved instr.\n");
		printf("\n\nUnimplemented or reserved instr.\n");
		interrupt->Halt();
		break;
	case NumExceptionTypes:
		DEBUG('a', "Number exception types\n");
		printf("\n\nNumber exception types\n");
		interrupt->Halt();
		break;

        case SyscallException:
		switch(type)
		{
			case SC_Halt:
				//input: No
				//output: shutdown notification
				//Function: turn off system
				DEBUG('a', "Shutdown, initiated by user program.\n");
				printf("\nShutdown, initiated by user program.\n");
   				interrupt->Halt();
				break;
				//return;
	
			case SC_Create:
				int virtAddr;
				char* filename;
				DEBUG('a',"\n SC_Create call ...");
				DEBUG('a',"\n Reading virtual address of filename");
				virtAddr = machine->ReadRegister(4);
				DEBUG ('a',"\n Reading filename.");
				filename = User2System(virtAddr,MaxFileLength+1);
				if (filename == NULL)
				{
					printf("\n Not enough memory in system");
					DEBUG('a',"\n Not enough memory in system");
					machine->WriteRegister(2,-1);
					delete filename;
					IncreasePC();
					return;
				}
				DEBUG('a',"\n Finish reading filename.");
				if (!fileSystem->Create(filename,0))
				{
					printf("\n Error create file '%s'",filename);
					machine->WriteRegister(2,-1);
					delete filename;
					IncreasePC();
					return;
				}
				machine->WriteRegister(2,0);
				delete filename;
				IncreasePC();
				break;
				//return;

			case SC_ReadInt:
				char* buffer;
				int Max_Buffer;
				Max_Buffer = 255;
				buffer=new char[Max_Buffer + 1];
				int numbytes;
				numbytes = synchConsole->Read(buffer, Max_Buffer);// read buffer size maximum 255 character, return bytes
				//negative or positive number
				bool isNegative;
				isNegative = false;//set number is negative
				int indexFirst;
				indexFirst=0;
				int indexLast;
				indexLast=0;
				if(buffer[0] == '-')
				{
					isNegative = true;
					indexFirst=1;

				}
			
				//check buffer must be integer
				for(int i= indexFirst; i<numbytes; i++)
				{
					//update indexLast
					indexLast = i;
					if(buffer[i] == '.')//9.0000
					{
						for(int j = i + 1; j < numbytes; j++)
						{
							// invalid number 
							if(buffer[j] != '0')//9.00100
								{
									printf("\n\n The integer number is not valid");
									DEBUG('a', "\n The integer number is not valid");
									machine->WriteRegister(2, 0);
									IncreasePC();
									delete buffer;
									return;
								}
						}
						// update indexLast
						indexLast = i - 1;
						delete buffer;
						return;
	
					}
					else if(buffer[i] < '0' && buffer[i] > '9')//90238320jslks
					{
						printf("\n\n The integer number is not valid");
						DEBUG('a', "\n The integer number is not valid");
						machine->WriteRegister(2, 0);
						IncreasePC();
						delete buffer;
						return;

					}

				}
				//convert string buffer to integer
				int n;
				n=0;//result
				for(int i = indexFirst; i<= indexLast; i++)
				{
					n = n * 10 + (int)(buffer[i] - 48);//ascii '0'=48,...,'9'=57
				}
	
				//if n is negative number
				if(isNegative == true)
				{
					n = n * -1;
				}
				//success
				machine->WriteRegister(2, n);
				IncreasePC();
				delete buffer;
				break;

			case SC_PrintInt:
				int number;
				number = machine->ReadRegister(4);
				int count;
				count=0;
				if(number == 0)
				{
					synchConsole->Write("0", 1);
					IncreasePC();
					return;
				}
	
				if(number < 0)// number is negative
				{
					number = number * -1;
					int temp; 
					temp = number;
					while(temp)// count how many numbers
					{
						count++;
						temp /= 10;
					}
					char* buffer;
					int MAX_BUFFER;
					MAX_BUFFER= 255;
					buffer = new char[MAX_BUFFER + 1];
					for(int i = count; i >= 1; i--)//press type int to char
					{
						buffer[i] = (char)((number % 10) + 48);
						number /= 10;
					}
					buffer[0] = '-';
					buffer[count + 1] = 0;
					synchConsole->Write(buffer, count + 1);
					delete buffer;
					IncreasePC();
					//return;
				}
				else// number is positive
				{
					int temp;
					temp = number;
					while(temp)
					{
						count++;
						temp /= 10;
					}
					char* buffer;
					int MAX_BUFFER;
					MAX_BUFFER = 255;
					buffer = new char[MAX_BUFFER + 1];
					for(int i = count - 1; i >= 0; i--)
					{
						buffer[i] = (char)((number % 10) + 48);
						number /= 10;
					}
					buffer[count] = 0;
					synchConsole->Write(buffer, count);
					delete buffer;
					IncreasePC();
					//return;
				}
				break;
			case SC_ReadChar:
				int Max_byte;
				Max_byte = 255;
				char* buffer1;
				buffer1 = new char[255];
				int byte;
				byte = synchConsole->Read(buffer1, Max_byte);
	
				if(byte > 1)
				{
					printf("Chi duoc nhap duy nhat 1 ky tu!");
					DEBUG('a', "\nERROR: Chi duoc nhap duy nhat 1 ky tu!");
					machine->WriteRegister(2, 0);
				}
				else if(byte == 0)
				{
					printf("Ky tu rong!");
					DEBUG('a', "\nERROR: Ky tu rong!");
					machine->WriteRegister(2, 0);
				}
				else
				{
					char c = buffer1[0];
					machine->WriteRegister(2, c);
				}
	
				delete buffer1;
				IncreasePC();
				break;
				//return;
			case SC_PrintChar:
				char character;
				character = (char)machine->ReadRegister(4); //read register r4
				synchConsole->Write(&character, 1); // print character 1 byte
				IncreasePC();
				break;
				//return;
			case SC_ReadString:
				int virtAddress2;
				int length2;
				char* buffer2;
				virtAddress2= machine->ReadRegister(4);//address from register r4
				length2 = machine->ReadRegister(5); // length from register r5 
				buffer2 = User2System(virtAddress2, length2); // coppy User Space to System Space
				synchConsole->Read(buffer2, length2); //  use point SynchConsole->read for read string
				System2User(virtAddress2, length2, buffer2); // Copy string from System Space to User Space
				delete buffer2;
				IncreasePC();
				break;				
				//return;
			case SC_PrintString:
				int virtAddress3;
				char* buffer3;
				virtAddress3 = machine->ReadRegister(4); //address from register r4
				buffer3 = User2System(virtAddress3, 255); // coppy User Space to System Space with length 255 character
				int length3;
				length3 = 0;
				while (buffer3[length3] != 0) 
					length3++; // count length string
				synchConsole->Write(buffer3, length3 + 1); // use point SynchConsole->write for print string
				delete buffer3;
				IncreasePC(); 
				break;
				//return;
			case SC_Open:
				//OpenFileID Open(char *name, int type)
				int virtAddr5;
				virtAddr5 = machine->ReadRegister(4); // get address from register r4 
				int type;
				type = machine->ReadRegister(5); // get parameter from register r5 
				char* filename5;
				filename5 = User2System(virtAddr5, MaxFileLength); // coppy User Space to System Space with MaxFileLength
				//check open file
				int freeSlot;
 				freeSlot = fileSystem->FindFreeSlot();
				if (freeSlot != -1) //Only proccess when there are slot
				{
					if (type == 0 || type == 1) //When type = 0 or 1
					{
					
						if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename5, type)) != NULL) //Open file success
						{
							machine->WriteRegister(2, freeSlot); //return OpenFileID
						}
					}
					else if (type == 2) // type=2, proccess stdin 
					{
						machine->WriteRegister(2, 0); //return OpenFileID
					}
					else // type=3, process stdout  
					{
						machine->WriteRegister(2, 1); //return OpenFileID
					}
					delete[] filename;
					break;
				}
				machine->WriteRegister(2, -1); //if cann't open file then return -1
			
				delete[] filename5;
				break;
			case SC_CloseFile:
				int ID;
				ID = machine->ReadRegister(4); // get ID of file from register r4 
				if (ID >= 0 && ID <= 14) // only proccess when ID in [0, 14] 
				{
					if (fileSystem->openf[ID]) //open file success
					{
						delete fileSystem->openf[ID]; //delete storage the file 
						fileSystem->openf[ID] = NULL; //assign NULL for storage 
						machine->WriteRegister(2, 0);
						break;
					}
				}
				machine->WriteRegister(2, -1);
				break;
			case SC_Read:
				int virtAddr6;
				virtAddr6 = machine->ReadRegister(4); // get address of buffer parameter from register r4 
				int size;
				size = machine->ReadRegister(5); // get size from register r5 Lay charcount tu thanh ghi so 5
				int ID1;
				ID1 = machine->ReadRegister(6); //get ID of file from register r6  Lay id cua file tu thanh ghi so 6 
				int OldPos;
				int NewPos;
				char *buf;
				// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
				// check ID of file 
				if (ID1 < 0 || ID1 > 14)
				{
					printf("\nKhong the read vi id nam ngoai bang mo ta file.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// check file exists? 
				if (fileSystem->openf[ID1] == NULL)
				{
					printf("\nKhong the read vi file nay khong ton tai.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// if type = 3(stdout) then return -1
				if (fileSystem->openf[ID1]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
				{
					printf("\nKhong the read file stdout.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// If open file success then get OldPos
				OldPos = fileSystem->openf[ID1]->GetCurrentPos();
				buf = User2System(virtAddr6, size); // coppy User Space to System Space with buffer size 
				// if type=2 (stdin)
				if (fileSystem->openf[ID1]->type == 2)
				{
					// Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
					int length;
					length = synchConsole->Read(buf, size); 
					System2User(virtAddr6, length, buf); // coppy System Space to User Space with the buffer length is byte number fact
					machine->WriteRegister(2, length); // return bytes fact
					delete buf;
					IncreasePC();
					return;
				}
				// if read the file normal return bytes fact 
				if ((fileSystem->openf[ID1]->Read(buf, size)) > 0)
				{
					// bytes_fact = NewPos - OldPos
					NewPos = fileSystem->openf[ID]->GetCurrentPos();
					// coppy System Space to User Space with the buffer length is byte number fact
					System2User(virtAddr6, NewPos - OldPos, buf); 
					machine->WriteRegister(2, NewPos - OldPos);
				}
				// If concept of file is null then return -2
				else
				{
					machine->WriteRegister(2, -2);
				}
				delete buf;
				IncreasePC();
				return;
			case SC_Write:
				int virtAddr7;
				virtAddr7 = machine->ReadRegister(4); 
				int size1;
				size1 = machine->ReadRegister(5); 
				int id;
				id = machine->ReadRegister(6); 
				int OldPos1;
				int NewPos1;
				char *buf1;
				if (id < 0 || id > 14)
				{
					printf("\nKhong the write vi id nam ngoai bang mo ta file.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				if (fileSystem->openf[id] == NULL)
				{
					printf("\nKhong the write vi file nay khong ton tai.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// if file only read (type = 1) or file stdin (type = 2) then return -1
				if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
				{
					printf("\nKhong the write file stdin hoac file only read.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				OldPos1 = fileSystem->openf[id]->GetCurrentPos();
				buf1 = User2System(virtAddr7, size1);
				// if file read & write (type = 0) then return number byte fact
				if (fileSystem->openf[id]->type == 0)
				{
					if ((fileSystem->openf[id]->Write(buf1, size1)) > 0)
					{
						NewPos1 = fileSystem->openf[id]->GetCurrentPos();
						machine->WriteRegister(2, NewPos1 - OldPos1);
						delete buf1;
						IncreasePC();
						return;
					}
				}
				if (fileSystem->openf[id]->type == 3) // if file stdout (type = 3)
				{
					int i = 0;
					while (buf1[i] != 0 && buf1[i] != '\n')
					{
						synchConsole->Write(buf1 + i, 1); // use function Write of class SynchConsole 
						i++;
					}
					buf1[i] = '\n';
					synchConsole->Write(buf1 + i, 1); // Write character '\n'
					machine->WriteRegister(2, i - 1); // return bytes fact 
					delete buf1;
					IncreasePC();
					return;
				}
			case SC_Seek:
				int pos2;
				pos2 = machine->ReadRegister(4);
				int id1;
				id1 = machine->ReadRegister(5); 
				if (id1 < 0 || id1 > 14)
				{
					printf("\nKhong the seek vi id nam ngoai bang mo ta file.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				if (fileSystem->openf[id1] == NULL)
				{
					printf("\nKhong the seek vi file nay khong ton tai.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// check call Seek on console?
				if (id1 == 0 || id1 == 1)
				{
					printf("\nKhong the seek tren file console.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				// if pos = -1 then assign pos = Length 
				//pos2 = (pos2 == -1) ? fileSystem->openf[id1]->Length() : pos2;
				if(pos2==-1)
					pos2 = fileSystem->openf[id1]->Length();
				if (pos2 > fileSystem->openf[id1]->Length() || pos2 < 0) // check local pos true?
				{
					printf("\nKhong the seek file den vi tri nay.");
					machine->WriteRegister(2, -1);
				}
				else
				{
					// if true then return local fact in file 
					fileSystem->openf[id1]->Seek(pos2);
					machine->WriteRegister(2, pos2);
				}
				IncreasePC();
				return;
			case SC_Exec:
				// Input: vi tri int
				// Output: Fail return -1, Success: return id cua thread dang chay
				// SpaceId Exec(char *name);
				int virtAddr8;
				virtAddr8 = machine->ReadRegister(4);	
				char* name;
				name = User2System(virtAddr8, MaxFileLength + 1);
	
				if(name == NULL)
				{
					DEBUG('a', "\n Not enough memory in System");
					printf("\n Not enough memory in System");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				OpenFile *oFile;
				oFile = fileSystem->Open(name);
				if (oFile == NULL)
				{
					printf("\nExec:: Can't open this file.");
					machine->WriteRegister(2,-1);
					IncreasePC();
					return;
				}

				delete oFile;

				// Return child process id
				int Id;
				Id = pTab->ExecUpdate(name); 
				machine->WriteRegister(2,Id);

				delete[] name;	
				IncreasePC();
				return;
			case SC_Join:
				int id2; 
				id2= machine->ReadRegister(4);
			
				int res;
				res = pTab->JoinUpdate(id2);
			
				machine->WriteRegister(2, res);
				IncreasePC();
				return;
			
			case SC_Exit:
				int exitStatus;
				exitStatus = machine->ReadRegister(4);

				if(exitStatus != 0)
				{
					IncreasePC();
					return;
				
				}			
			
				int res1;
				res1 = pTab->ExitUpdate(exitStatus);
				//machine->WriteRegister(2, res);

				currentThread->FreeSpace();
				currentThread->Finish();
				IncreasePC();
				return; 
			case SC_CreateSemaphore:
				int virtAddr9;
 				virtAddr9= machine->ReadRegister(4);
				int semval;
				semval = machine->ReadRegister(5);

				char *name2;
				name2 = User2System(virtAddr, MaxFileLength + 1);
				if(name == NULL)
				{
					DEBUG('a', "\n Not enough memory in System");
					printf("\n Not enough memory in System");
					machine->WriteRegister(2, -1);
					delete[] name2;
					IncreasePC();
					return;
				}
			
				int res2;
				res2 = semTab->Create(name2, semval);

				if(res2 == -1)
				{
					DEBUG('a', "\n Khong the khoi tao semaphore");
					printf("\n Khong the khoi tao semaphore");
					machine->WriteRegister(2, -1);
					delete[] name2;
					IncreasePC();
					return;				
				}
			
				delete[] name2;
				machine->WriteRegister(2, res2);
				IncreasePC();
				return;
			case SC_Wait:			
				int virtAddr10;
				virtAddr10 = machine->ReadRegister(4);

				char *name3;
 				name3= User2System(virtAddr, MaxFileLength + 1);
				if(name == NULL)
				{
					DEBUG('a', "\n Not enough memory in System");
					printf("\n Not enough memory in System");
					machine->WriteRegister(2, -1);
					delete[] name3;
					IncreasePC();
					return;
				}
			
				int res3;
				res3 = semTab->Wait(name);

				if(res3 == -1)
				{
					DEBUG('a', "\n Khong ton tai ten semaphore nay!");
					printf("\n Khong ton tai ten semaphore nay!");
					machine->WriteRegister(2, -1);
					delete[] name3;
					IncreasePC();
					return;				
				}
			
				delete[] name3;
				machine->WriteRegister(2, res3);
				IncreasePC();
				return;
			case SC_Signal:		
				int virtAddr11;
 				virtAddr11= machine->ReadRegister(4);

				char *name4;
 				name4= User2System(virtAddr, MaxFileLength + 1);
				if(name == NULL)
				{
					DEBUG('a', "\n Not enough memory in System");
					printf("\n Not enough memory in System");
					machine->WriteRegister(2, -1);
					delete[] name4;
					IncreasePC();
					return;
				}
			
				int res4;
				res4 = semTab->Signal(name);

				if(res4 == -1)
				{
					DEBUG('a', "\n Khong ton tai ten semaphore nay!");
					printf("\n Khong ton tai ten semaphore nay!");
					machine->WriteRegister(2, -1);
					delete[] name4;
					IncreasePC();
					return;				
				}
				delete[] name4;
				machine->WriteRegister(2, res4);
				IncreasePC();
				return;
			case SC_Sum:
				// int Sum(int a, int b)
				int x;
				x = machine->ReadRegister(4);
				int y;
				y = machine->ReadRegister(5);
				int sum;
				sum = x + y;
				machine->WriteRegister(2, sum);
				IncreasePC();
				return;	
		}	
	}
	IncreasePC();
}


