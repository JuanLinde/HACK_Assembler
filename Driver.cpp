#include <unordered_map>
#include <string>
#include <fstream>
#include <set>
#include <math.h>
using namespace std;

void populateSet(set<int>&);
void populateSymbolsTable(unordered_map<string, int>& ,set<int>&);
string toBinary(string);

/*
	Functionality: This program receives a document with HACK assembly instructions and creates
	               a document with their machine language equivalent instructions.

	General Algorithm:

					1. While there are lines to be read from assembly file:
					2.  If the line contains an instruction:
					3.   Extract the instruction from the line
					4.   Divide it into its logical components
					5.   Translate its logical components to their corresponding binary code
					6.   Concatenate the translation
					7.   Output the translation to machine code file

	Author: Juan Linde
		    
*/
int main()
{
	/*
		Preamble:
		         1. Open a connection to the assembly code
				 2. Create and populate symbol table with predefined symbols
				 3. Create and populate empty register table
	*/
	ifstream assemblyCode("testInput.txt");
	set<int> emptyRegisters;
	populateSet(emptyRegisters);
	unordered_map<string, int> symbolsTable; 
	populateSymbolsTable(symbolsTable,emptyRegisters);


}

/*
	Functionality: Receives a reference to the set of empty registers (set) and populates the set
	               with the numbers of all available registers (24576).

	Algorithm: 
				1. Count from 0 to 24577 (exclusive), for each reg:
				2.  insert register into the set
*/
void populateSet(set<int>& set)
{
	for (int reg = 0; reg < 24577; reg++)
	{
		set.insert(reg);
	}
}
/*
	Functionality: Receives the symbols table (um) and populates it with all the predefined symbols

				   Side effect: updates the empty registers (er) set by erasing each register that
				                goes into the symbols table.

	Algorithm:

			      1. Count from 0 to 16 (exclusive), for each count i:
				  2.  key   = Ri
				  3.  value = i
				  4.  insert (key,value) into map
				  5.  update empty register set
				  6. insert ("SCREEN", 16384) into map
				  7. insert ("KBD", 24574) into map

*/
void populateSymbolsTable(unordered_map<string,int>& um, set<int>& er)
{
	for (int i = 0; i < 16; i++)
	{
		string key = "R" + to_string(i);
		int value = i;
		um.emplace(key, value);
		er.erase(value);
	}
	um.emplace("SCREEN", 16384);
	um.emplace("KBD", 24576);
	er.erase(16384);
	er.erase(24576);
}
/*
	Function: receives a string, value, with a positive number in decimal system and translates
			  the value to a 15-bit binary number stored in a string, valueInBinary.

	Algorithm:
			   1. Initialize valueInBinary variable with 15 zeros
			   2. Initialize numToTranslate variable with input value
			   3. While numToTranslate is not equal to zero:
			   4.     initialize position variable, position, to log_2(n)
			   5.     Change the corresponding position in output variable to 1
			   6.     Calculate new number to translate -> n = n - 2^position
			   8. Output valueInBinary
*/
string toBinary(string value)
{

	string valueInBinary = "000000000000000";
	int numToTranslate = stoi(value);

	bool thereIsRemainder = (numToTranslate != 0);
	while (thereIsRemainder)
	{
		int position = log2(numToTranslate);
		valueInBinary[valueInBinary.length() - 1 - position] = '1';
		numToTranslate -= pow(2, position);
		thereIsRemainder = (numToTranslate != 0);
	}
	return valueInBinary;
}