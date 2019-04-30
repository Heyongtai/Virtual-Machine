/* Developer: Jacob Gidley, CSI 404, Professor Phipps
* This program will assemble SIA assembly language code from a text file into binary
* and output the machine code into a designated file.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *ltrim(char *s) 
{
	while (*s == ' ' || *s == '\t') 
		s++;
		
	return s;
}

char getRegister(char *text) 
{
	if (*text == 'r' || *text=='R') 
		text++;
		
	return atoi(text);
}

int assembleLine(char *text, unsigned char* bytes) 
{
	text = ltrim(text);
	char *keyWord = strtok(text," "); // Get the instruction's name
	
	/* add */
	if (strcmp("add",keyWord) == 0)
	{
		bytes[0] = 0x10;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* subtract */
	if (strcmp("subtract",keyWord) == 0)
	{
		bytes[0] = 0x50;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* multiply */
	if (strcmp("multiply",keyWord) == 0)
	{
		bytes[0] = 0x40;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* divide */
	if (strcmp("divide",keyWord) == 0)
	{
		bytes[0] = 0x30;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* and */
	if (strcmp("and",keyWord) == 0)
	{
		bytes[0] = 0x20;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* or */
	if (strcmp("or",keyWord) == 0)
	{
		bytes[0] = 0x60;
		bytes[0] |= getRegister(strtok(NULL," "));
		bytes[1] = getRegister(strtok(NULL," ")) << 4 | getRegister(strtok(NULL," "));
		
		return 2;
	}
	
	/* halt */
	if (strncmp("halt", keyWord, 4) == 0) // strncmp does not compare with the cartridge return
	{	
		// There is no need to do anything here but return the # of bytes because bytes[] is currently all 0s
		return 2;
	}
	
	/* branch if equal */
	if (strcmp("branchifequal",keyWord) == 0) 
	{
		int x, y;
		
		bytes[0] = 0xA0;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		bytes[1] = getRegister(strtok(NULL," ")) << 4; // _ _ _ _  0000
		
		x = atoi(strtok(NULL," ")); // Get value of address offset 
		
		y = x >> 16; // Get top 4 bits of address offset, works for numbers that are 20 bits long
		y &= 0x0F; // AND the top 4 bits with 15 (1111), gets rid of any leading 1s for negative offsets
		
		bytes[1] |= y; // OR the bits of the 2nd register and the top 4 bits of the address offset 
		
		bytes[2] = (x >> 8) & 0xFF; // Get first 8 bits of the address offset 

		bytes[3] = x & 0xFF; // Store the last 8 bits of the offset
		
		return 4;
	}
	
	/* branch if less */
	if (strcmp("branchifless",keyWord) == 0) 
	{
		int x, y;
		
		bytes[0] = 0xB0;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		bytes[1] = getRegister(strtok(NULL," ")) << 4; // _ _ _ _  0000
		
		x = atoi(strtok(NULL," ")); // Get value of address offset 
		
		y = x >> 16; // Get top 4 bits of address offset, works for numbers that are 20 bits long
		y &= 0x0F; // AND the top 4 bits with 15 (1111), gets rid of any leading 1s for negative offsets
		
		bytes[1] |= y; // OR the bits of the 2nd register and the top 4 bits of the address offset 
		
		bytes[2] = (x >> 8) & 0xFF; // Get first 8 bits of the address offset 

		bytes[3] = x & 0xFF; // Store the last 8 bits of the offset
		
		return 4;
	}
	
	/* leftshift */
	if (strcmp("leftshift",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0x70;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		x = atoi(strtok(NULL," ")); // Get value of shift amount
		
		bytes[1] = x;
		
		return 2;
	}
	
	/* rightshift */
	if (strcmp("rightshift",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0x70;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		x = atoi(strtok(NULL," ")); // Get value of shift amount
		
		bytes[1] = 0x20 | x; // OR shift amount with 0010 0000 in order to set the shift direction bit
		
		return 2;
	}
	
	/* jump */
	if (strcmp("jump",keyWord) == 0)
	{
		int x, y;
		
		bytes[0] = 0xC0;
		
		x = atoi(strtok(NULL," ")); // Get value of address offset 
		
		y = x >> 24; // Get the top 4/12 bits to store into the last 4 bits of bytes[0]
		bytes[0] |= y;
		
		y = x >> 16; // Get the top 8/12 bits to store into bytes[1]
		bytes[1] = y;
		
		y = x >> 8; // Get 8/16 of the bits of the lower address offset
		bytes[2] = y;
		
		bytes[3] = x; // Get other 8/16 bits from the lower address offset
		
		return 4;
	}
	
	/* load */
	if (strcmp("load",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0xE0;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		bytes[1] = getRegister(strtok(NULL," ")) << 4; // _ _ _ _  0000
		
		x = atoi(strtok(NULL," "));
		
		bytes[1] |= x & 0x0F; // Store the 4 bits of address offset with any leading 1s removed
		
		return 2;
	}
	
	/* store */
	if (strcmp("store",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0xF0;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		bytes[1] = getRegister(strtok(NULL," ")) << 4; // _ _ _ _  0000
		
		x = atoi(strtok(NULL," ")); 
		
		bytes[1] |= x & 0x0F; // Store the 4 bits of address offset with any leading 1s removed
		
		return 2;
	}
	
	/* addimmediate */
	if (strcmp("addimmediate",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0x90;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		x = atoi(strtok(NULL," "));
		
		bytes[1] = x; // Store the 8 bits of immediate value with any leading 1s removed
		
		return 2;
	}
	
	/* iterate over */
	if (strcmp("iterateover",keyWord) == 0)
	{
		int x, y;
		
		bytes[0] = 0xD0;
		bytes[0] |= getRegister(strtok(NULL," "));
		
		x = atoi(strtok(NULL," ")); // Pointer offset
		y = atoi(strtok(NULL," ")); // Address offset
		
		bytes[1] = x; // Store 8 bits of pointer offset
		
		bytes[2] = y >> 8; // Store the first 8 bits of address offset
		
		bytes[3] = y; // Store last 8 bits of address offset
		
		return 4;
	}
	
	/* interrupt */
	if (strcmp("interrupt",keyWord) == 0)
	{
		int x;
		
		bytes[0] = 0x80;
		
		x = atoi(strtok(NULL," ")); // interrupt value
		
		bytes[0] |= x >> 8; // Store first 4/12 bits of the interrupt value
		
		bytes[1] = x; // Store the last 8/12 bits of the interrupt value
		
		return 2;
	}
	
	// Return 0 if instruction does not exist
	printf("Error: \"%s\" is not a recognized SIA instruction\n", keyWord);
	return 0;
}

int main(int argc, char **argv) 
{
	FILE *src = fopen(argv[1],"r");
	FILE *dst = fopen(argv[2],"wb");
	
	while (!feof(src)) 
	{
		unsigned char bytes[4];
		char line[1000];
		
		printf ("About to read\n");
		
		if (NULL != fgets(line, 1000, src)) 
		{
			printf ("Read: %s\n",line);
			int byteCount = assembleLine(line,bytes); // Get the amount of bytes to write
			
			if (byteCount != 0) 
			{
				fwrite(bytes,byteCount,1,dst);
			}
			
			memset(bytes, 0, sizeof(bytes)); // Clear the bytes array of previous values (Safer this way!)
		}
	}
	fclose(src);
	fclose(dst);
	return 0;
}