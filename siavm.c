/* Developed by Jacob Gidley, CSI 404, Professor Phipps
*
* This program is a VM for the SIA architecture
* It will run a SIA program from an assembled machine code file as inout
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#define MEM_SIZE 10240	// VM has 10Kb allocated to it
#define REG_NUM	16

// Function prototypes
void loadFile(char *fileName);
void fetch();
void dispatch();
void execute();
void store();
void run();

// Declare global variables (registers)
int registers[REG_NUM];
int op1 = 0,	// OP1
	op2 = 0,	// OP2
	result = 0,	// Result register
	pc = 0;		// PC counter
	
unsigned char memory[MEM_SIZE]; // VM memory
unsigned char opcode; 			// Holds the current instruction opcode
unsigned char haltFlag = 0; 	// Will be set when the current instruction read is HALT
char curInstBuffer[4];			// Current instruction buffer


/* Reads the file and populates memory starting at the beginning */
void loadFile(char *fileName)
{
	FILE *fp; // Pointer to input file
	
	// Open input file
	if ((fp = fopen(fileName,"r")) == NULL)
	{
		fprintf(stderr, "Error: In function 'loadFile': Failed to open %s.\n", fileName);
		perror("Error");
		exit(1);
	}

	// Store the whole contents of input file into memory
	fread(memory, 1, MEM_SIZE, fp);
	if (ferror(fp))
	{
		fprintf(stderr, "Error: In fucntion 'loadFile': Failed to read from %s\n", fileName);
		perror("Error");
		exit(1);
	}
	
	// Close the file
	fclose(fp);
}

/* Fetches the next instruction from memory and stores it in a separate buffer */
void fetch()
{
	int i, 
		j = pc; // j = PC counter so that we start in memory[] from where we are in the SIA program i.e. if we are on the first instruction, then j = PC = 0.
		
	for(i = 0; i < 4; i++)
	{
		curInstBuffer[i] = memory[j];
		j++;
	}
}

/* Decode the current fetched instruction, find opcode, break into micro ops (if needed), increment PC counter */
void dispatch()
{
	// Get opcode from fetch buffer
	opcode = (curInstBuffer[0] >> 4) & 0x0F; // Store the instruction's opcode
	
	// Handle 3R opcodes except halt (add = 1, and = 2, divide = 3, multiply = 4, subtract = 5, or = 6)
	if (opcode > 0 && opcode <= 6) 
	{
		op1 = registers[curInstBuffer[0] & 0x0F];
		op2 = registers[(curInstBuffer[1] >> 4) & 0x0F];
	}
	
	// Handles sft opcode
	if (opcode == 7)
	{
		op1 = registers[curInstBuffer[0] & 0x0F];
		op2 = curInstBuffer[1] & 0x1F; // Get 5 bit shift value
	}
	
	// Handles interrupt opcode
	if (opcode == 8)
	{
		op1 = ((curInstBuffer[0] & 0x0F) << 8) | (curInstBuffer[1] & 0xFF);
	}
	
	// Handles ai opcode
	if (opcode == 9)
	{
		op1 = registers[curInstBuffer[0] & 0x0F];
		op2 = curInstBuffer[1];
	}
	
	// Handles branch opcodes
	if (opcode == 10|| opcode == 11)
	{
		// Get register values to compare in execute
		op1 = registers[curInstBuffer[0] & 0x0F];
		op2 = registers[(curInstBuffer[1] >> 4) & 0x0F];
	}
	
	// Handles jump opcode 
	if (opcode == 12)
	{
		// Get unsigned (absolute value) of jump address
		op1 = (((curInstBuffer[0] & 0x0F) << 24) | ((curInstBuffer[1] & 0xFF) << 16) | ((curInstBuffer[2] & 0xFF) << 8) | (curInstBuffer[3] & 0xFF));
	}
	
	// Handles iterateover opcode
	if (opcode == 13)
	{
		//op1 = curInstBuffer[1] & 0xFF; // Next pointer amount (unsigned)
		op1 = registers[curInstBuffer[0] & 0x0F]; // pointer register value
		op2 = ((curInstBuffer[2] << 8) | curInstBuffer[3]) & 0xFFFF;	// delta offset amount (unsigned)
	}
	
	// Handles load and store opcodes
	if (opcode == 14 || opcode == 15)
	{
		op1 = registers[curInstBuffer[0] & 0x0F]; // first register's value
		op2 = registers[(curInstBuffer[1] >> 4) & 0x0F]; // second register's value	
	}
	
	// Check how much to increase PC counter by (Only 4 instructions are 4 bytes, opcodes are between 10 - 13)
	if (opcode >= 10 && opcode <= 13)
	{
		pc += 4; // Increment pc counter by 4 bytes
	}
	else
	{
		pc += 2; // Increment pc counter by 2 bytes
	}
}

/* Executes the instruction that was fetched */
void execute()
{
	// Execute different instructions depending on opcode
	switch(opcode)
	{
		case 0:
		{
			haltFlag = 1;	// Halt has been read, set flag
			break;
		}
		case 1:		// add
		{
			result = op1 + op2;
			break;
		}
		case 2:		// and
		{
			result = op1 & op2;
			break;
		}
		case 3:		// divide
		{
			if (op2 == 0)
			{
				fprintf(stderr, "Error: In function 'execute': Divide by zero\n");
				exit(1);
			}
			else
			{
				result = op1 / op2;
			}
			break;
		}
		case 4:		// multiply
		{
			result = op1 * op2;
			break;
		}
		case 5:		// subtract
		{
			result = op1 - op2;
			break;
		}
		case 6:		// or
		{
			result = op1 | op2;
			break;
		}
		case 7:		// leftshift and rightshift
		{
			// Check if rightshift bit is set
			if ((curInstBuffer[1] & 0x20) == 0x20)
			{
				result = op1 >> op2; // rightshift
			}
			else
			{
				result = op1 << op2; // leftshift
			}
			break;
		}
		case 8:		// interrupt
		{
			// Check the interrupt code
			switch (op1)
			{
				int i;
				case 0:		// Print registers
				{
					for (i = 0; i < REG_NUM; i++)
					{
						printf("R[%d] = %d\n", i, registers[i]);
					}
					
					printf("OP1 = %d, OP2 = %d\n", op1, op2);
					printf("Result = %d\n", result);
					printf("PC = %d\n", pc);
					break;
				}
				case 1:		// Print memory
				{
					for (i = 0; i < 120; i++)
					{
						printf("Memory[%d] = %d | ", i, memory[i]);
						
						 // Display 5 elements of memory per line
						if (i % 5 == 0 && i != 0)
						{
							printf("\n");
						}
					}
					printf("\n");
					break;
				}
			}
		}
		case 9:		// addimmediate
		{
			result = op1 + op2;
			break;
		}
		case 10:	// branchifequal
		{
			// Check to see if argument register values are equal
			if (op1 == op2)
			{
				// Adjust PC value according to given offset
				if((curInstBuffer[1] & 0x0F) > 7) // Check if offset is negative
				{
					pc += ((((curInstBuffer[1] & 0x0F) - 16) << 16) | ((curInstBuffer[2] & 0xFF) << 8) | (curInstBuffer[3] & 0xFF) - 4); // -4 to account for PC increase in dispatch()
				}
				else
				{
					pc += ((((curInstBuffer[1] & 0x0F) << 16) | ((curInstBuffer[2] & 0xFF) << 8) | (curInstBuffer[3] & 0xFF)) - 4); // -4 to account for PC increase in dispatch()
				}
			}
			break;
		}
		case 11:	// branchifless
		{
			// Check to see if 1st argument register is equal than 2nd argument register
			if (op1 < op2)
			{
				// Adjust PC value according to given offset
				if((curInstBuffer[1] & 0x0F) > 7) // Check if offset is negative
				{
					pc += ((((curInstBuffer[1] & 0x0F) - 16) << 16) | ((curInstBuffer[2] & 0xFF) << 8) | (curInstBuffer[3] & 0xFF) - 4); // -4 to account for PC increase in dispatch()
				}
				else
				{
					pc += ((((curInstBuffer[1] & 0x0F) << 16) | ((curInstBuffer[2] & 0xFF) << 8) | (curInstBuffer[3] & 0xFF)) - 4); // -4 to account for PC increase in dispatch()
				}
			}
			break;
		}
		case 12:	// jump
		{
			pc = op1;	// jump to address in memory, set PC
			break;
		}
		case 13:	// iterateover
		{
			
			op1 += curInstBuffer[1] & 0xFF; // offset is added (unsigned) to the contents of the pointer register
			
			// Find the value of the next-pointer's address in memory
			result = ((memory[op1] << 24) | (memory[op1 + 1] << 16) | (memory[op1 + 2] << 8) | memory[op1 + 3]); 
			
			// Check if memory at pointer value is null(0), if not decrement PC by (delta + 4) bytes
			if (result != 0)
			{
				pc -= (op2 + 4); // decrement PC, the +4 is to account for earlier PC increase in dispatch
			}
			break;
		}
		case 14:	// Load
		{
			// Get memory address to write to
			if (curInstBuffer[1] & 0x0F > 8) // if offset is negative
			{
				result = op2 + ((curInstBuffer[1] & 0x0F) - 16); // convert offset to its intended negative value 
			}
			else
			{
				result = op2 + (curInstBuffer[1] & 0x0F);
			}
			break;
		}
		case 15:	// store
		{
			// Get memory address to write to
			if ((curInstBuffer[1] & 0x0F) > 8) // if offset is negative
			{
				result = op2 + ((curInstBuffer[1] & 0x0F) - 16); // convert offset to its intended negative value 	
			}
			else
			{
				result = op2 + (curInstBuffer[1] & 0x0F);
			}
			break;
		}
	}
}

/* Stores the value of the result register */
void store()
{
	// 3R instructions
	if (opcode > 0 && opcode <= 6)
	{
		registers[curInstBuffer[1] & 0x0F] = result;
	}
	
	// addimmediate and shift instructions
	if (opcode == 9 || opcode == 7)
	{
		registers[curInstBuffer[0] & 0x0F] = result;
	}
	
	// iterateover instruction
	if (opcode == 13)
	{
		registers[curInstBuffer[0] & 0x0F] = result; // Store the value of the next pointer address in memory
	}
	
	// load instruction
	if (opcode == 14)
	{
		registers[curInstBuffer[0] & 0x0F] = (memory[result] << 24);		// load 1st 8 bits of int value
		registers[curInstBuffer[0] & 0x0F] |= (memory[result + 1] << 16);	// load 2nd 8 bits of int value
		registers[curInstBuffer[0] & 0x0F] |= (memory[result + 2] << 8);	// load 3rd 8 bits of int value
		registers[curInstBuffer[0] & 0x0F] |= memory[result + 3];			// load 4th 8 bits of int value
	}
	
	// store instruction
	if (opcode == 15)
	{
		// split up int value into 4 signed chars
		memory[result] = (op1 >> 24) & 0x00FF;		// Store 1st 8 bits of int value
		memory[result + 1] = (op1 >> 16) & 0x00FF;	// Store 2nd 8 bits of int value
		memory[result + 2] = (op1 >> 8) & 0x00FF;	// Store 3rd 8 bits of int value
		memory[result + 3] = op1 & 0x00FF;			// Store 4th 8 bits of int value
	}
}

// Run loop
void run()
{
	// keep looping until a halt instruction is read
	while(haltFlag != 1) 
	{
		fetch();
		dispatch();
		execute();
		store();
	}
}

/* Main function that initiates the run loop */ 
int main (int argc, char *argv[])
{
	// Check for arg count
	if (argc != 2)
	{
		printf("Error: Wrong number of command line arguments specified!\n");
		exit(1);
	}
	
	loadFile(argv[1]); // Load file into memory
	
	run();	// Call the run loop
	
    return 0;
}