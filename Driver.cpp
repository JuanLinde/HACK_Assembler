#include <unordered_map>
#include <string>
#include <fstream>
#include <set>
#include <math.h>
#include <unordered_set>
using namespace std;

void populateSet(set<int>&);
void populateSymbolsTable(unordered_map<string, int>&, unordered_set<string>&, string);
bool checkForInstruction(string);
bool checkForLabelDef(string);
string getLabelFrom(string);
void updateEmptyRegisters(set<int>&, const unordered_map<string, int>&, 
	                      const unordered_set<string>&);
void translateToMachineLanguage(set<int>&, unordered_map<string, int>&, 
	                            const unordered_set<string>&, string);
string getInstructionFrom(string);
string toBinary(string);
string getDestinationFrom(string);
string getComputationFrom(string);
string getJumpInstructionFrom(string);

/*
	Functionality: This program receives a document with HACK assembly instructions and creates
				   a document with their machine language equivalent instructions.

	General Algorithm:

	1. Create a table with all the empty registers
	2. Populate symbols table 
	3. Update empty register set
	4. Go through assembly code translating each instruction  

	Author: Juan Linde
			
*/
int main(int argc, char* argv[])
{
	/*
		1. Populate empty register set
		2. Populate symbols table
		3. Update empty register set
		4. Translate to machine language
	*/
	set<int> emptyRegisters;
	unordered_map<string, int> symbolsTable;
	unordered_set<string> labels;
	string assemblyFileName = argv[1];

	populateSet(emptyRegisters);
	populateSymbolsTable(symbolsTable, labels, assemblyFileName);
	updateEmptyRegisters(emptyRegisters, symbolsTable,labels);
	translateToMachineLanguage(emptyRegisters,symbolsTable,labels,assemblyFileName);
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
	Functionality: Receives the symbol table (st) and populates it with the predefined symbols
	               and the label definitions in the assembly code

	General Algorithm:

	1. Populate with predefined symbols
	2. Populate with label definitions

	Complexity: O(n^2)

*/
void populateSymbolsTable(unordered_map<string,int>& st, unordered_set<string>& l, string afn)
{
	/*
		Populate with predefined symbols:
		1. Count from 0 to 16, for each i:
		2.   Create the string for the name of each register
		3.   Insert the name, register number pair into the table
		4. Insert the name and register number for the screen label
		5. Insert the name and register number for the keyboard label.
	*/
	for (int i = 0; i < 16; i++)
	{
		string key = "R" + to_string(i);
		int value = i;
		st.emplace(key, value);
	}
	st.emplace("SP", 0);
	st.emplace("LCL", 1);
	st.emplace("ARG", 2);
	st.emplace("THIS", 3);
	st.emplace("THAT", 4);
	st.emplace("SCREEN", 16384);
	st.emplace("KBD", 24576);

	/*
		Populate table with label definitions:
		1. Open connection to assembly code
		2. While there are lines to be read:
		3.   If line has instruction:
		4.     update instruction count
		5.   else if line has label definition:
		6.     insert label and register number into symbols table
		7. Close connection to assembly code
	*/
	ifstream assemblyCode(afn);
	string currLine = "";
	int instructionCount = 0;
	std::getline(assemblyCode, currLine);
	while (assemblyCode)
	{
		bool lineHasInstruction = checkForInstruction(currLine);
		bool lineHasLabelDefinition = checkForLabelDef(currLine);

		if (lineHasInstruction)
		{
			instructionCount += 1;
		}
		else if (lineHasLabelDefinition)
		{
			string labelDef = getLabelFrom(currLine);
			int regNum = instructionCount;
			st.emplace(labelDef, regNum);
			l.insert(labelDef);
		}
		std::getline(assemblyCode, currLine);
	}
	assemblyCode.close();
}
/*
	Functionality: Receives a line and outputs true or false depending if the line contains an
				   HACK assembly language instruction.

	Algorithm:

				1. count from 0 to length of line - 1, for each character
				2.   If character is a space, continue
				3.   else if character is beginning of comment or label definition, return false
				4.   else, return true
				5. return false  

*/
bool checkForInstruction(string line)
{
	for (int character = 0; character < line.length(); character++)
	{
		bool charIsSpace = (line[character] == ' ');
		bool charIsBegComment = (line[character] == '/');
		bool charIsBegLabelDef = (line[character] == '(');
		
		if (charIsSpace) continue;
		else if (charIsBegComment || charIsBegLabelDef) return false;
		else return true;
	}
	return false; // Takes care of the case of an empty line
}
/*
	Functionality: Receives a line of assembly code (line) and outputs true or false depending
	               if the line contains a label definition

	General algorithm:

	1. If line contains a label definition, return true
	2. else return false

	Complexity: 
*/
bool checkForLabelDef(string line)
{
	/*
		1. Count from 0 to number of characters, for each character
		2.   if the character is a space, continue
		3.   else if the character is the beginning of a comment, return false
		4.   else if the character is the beginning of a label definition, return true
		5.   else if the character is another character return false
		6. Return false
	*/
	for (int i = 0; i < line.length(); i++)
	{
		char currChar = line[i];
		bool charIsSpace = (currChar == ' ');
		bool charIsBegComm = (currChar == '/');
		bool charIsBegLabelDef = (currChar == '(');

		if (charIsSpace) continue;
		else if (charIsBegComm) return false;
		else if (charIsBegLabelDef) return true;
		else return false;
	}
	return false; // This is the case where line is empty
}
/*
	Functionality: Receives a line of assembly code (line) that has a label definition and returns
	               a string with just the label definition.

	General algorithm:
	1. Look through the line
	2. Extract the label definition only
	3. Return the result

	Complexity: O(n) where n is the number of characters in the line
*/
string getLabelFrom(string line)
{
	/*
		Look through the line and extract only the label definition:
		1. Count from 0 to the number of characters in the line (exclusive), for each character
		2.   If the character is a space, continue
		3.   Else if the character is the beginning of a comment, break
		4.   Else, append the character to the label definition to be returned
		5. Return the label definition
	*/
	string labelDef = "";
	for (int i = 0; i < line.length(); i++)
	{
		char currChar = line[i];
		bool currCharIsSpace = (currChar == ' ');
		bool currCharIsBegComm = (currChar == '/');
		bool currCharIsBegLab = (currChar == '(');
		bool currCharIsEndLab = (currChar == ')');

		if (currCharIsSpace || currCharIsBegLab) continue;
		else if (currCharIsBegComm || currCharIsEndLab) break;
		else labelDef += currChar;
	}
	return labelDef;
}
/*
	Functionality: Receives a set of ints that represents the empty registers (er) and the mapping
	               of labels to register number (st), compares them, and erases the used registers 
				   from the empty registers set.
	
	General Algorithm:
	
	1. Iterate through symbols table
	2. If a register is in symbols table and in empty register set, delete them from 
	   empty register set

	Complexity:

	1. Iterating through an unordered_map is O(log n)
	2. Counting in a set is O(log n)
	3. Erasing in a set is O(log n)
	4. So, in total it is O(log^3 n), worst case

*/
void updateEmptyRegisters(set<int>& er, const unordered_map<string,int>& st, 
	                      const unordered_set<string>& l)
{
	/*
		1. Iterate through each elemen in the symbols table
		2.   Determine if the register of an element is in the empty register set
		     and is not a label definition since label definitions do not occupy registers
			 in data memory.
		3.   If it is, delete it because it is being used
	*/
	for (const auto& pair : st)
	{
		string name = pair.first;
		int regNum = pair.second;
		bool regIsInEmptyRegistersSet = (er.count(regNum) > 0);
		bool nameIsLabel = (l.count(name) > 0);

		if (regIsInEmptyRegistersSet && !nameIsLabel)
		{
			er.erase(regNum);
		}
	}
}
/*
	Functionality: Receives an set that tracks the empty registers (er) and an unordered map with 
	               the symbols table and uses both to translate a file of HACK assembly code to
				   HACK machine language and output the result to a file.

	General Algorithm:

	1. While there are lines to read from assembly code
	2.   If the line contains an instruction
	3.     Extract the instruction
	4.     Determine type of instruction
	5.     Divide instruction into fields
	5.     Translate each field
	6.     Concatenate results
	7.     Output results to machine code file

	Complexity:

	1. The outer loop is O(n)
	2.   Checking if there is an instruction is O(n)
	3.   Extracting the instruction is O(n)
	4.   Determining the type of the instruction is O(1)
	5.   Extracting value is O(n)
	6.   toBinary(value) is O(1)
	7.   map.count() is O(log n)
	8.   Updating empty register set is O(log n)
*/
void translateToMachineLanguage(set<int>& er, unordered_map<string, int>& st, 
	                            const unordered_set<string>& l, string afn)
{
	ifstream assemblyCode(afn);
	int dotPos = afn.find('.');
	string outputFileName = afn.substr(0,dotPos) + ".hack";
	ofstream machineCode(outputFileName);

	string currLine = "";
	std::getline(assemblyCode, currLine);
	while (assemblyCode)
	{
		bool lineHasInstruction = checkForInstruction(currLine);
		if (lineHasInstruction)
		{
			string instruction = getInstructionFrom(currLine);
			bool instructionIsA = (instruction[0] == '@');
			/*
			    Algorithm in more detail:

				1. If it is an A instruction:
				2.   If value after @ is a number:
				3.     Translate the number to binary
				4.     Concatenate op code
				5.     Output machine language code
				6.   else value after @ is a name:
				7.     If name is already in symbols table:
				8.       Extract its register number
				9.       Translate it to binary
				10.      Concatenate op code
				11.      Output machine language code
				12.    else name is not in symbols table:
				13.      Look for first empty register
				14.      Add name and register number to symbols table
				15.      Update the empty registers tracker
				16.      Translate register number to binary
				17.      Concatenate with Op Code
				18.      Output machine language code
			*/
			if (instructionIsA)
			{
				string value = instruction.substr(1);
				int firstSymbol = int(value[0]) - 48; // Avoids reading char as ASCII code
				bool valueIsNumber = (firstSymbol >= 0 && firstSymbol <= 9); 

				if (valueIsNumber)
				{
					string opCode = "0";
					string machineInstruction = opCode + toBinary(value);
					machineCode << machineInstruction << endl;
				}
				else
				{
					bool symbolAlreadyInTable = (st.count(value) > 0);

					if (symbolAlreadyInTable)
					{
						string regNum = to_string(st.at(value));
						string opCode = "0";
						string machineInstruction = opCode + toBinary(regNum);
						machineCode << machineInstruction << endl;
					}
					else
					{
						int firstEmptyRegister = *(er.begin());
						string regNum = to_string(firstEmptyRegister);
						st.emplace(value, stoi(regNum));
						updateEmptyRegisters(er,st,l);
						string opCode = "0";
						string machineInstruction = opCode + toBinary(regNum);
						machineCode << machineInstruction << endl;
					}
				}
			}
			else {
				// Assembly language specification
				unordered_map<string,string> compToBinary =
				{ {"0","0101010"},{"1","0111111"},{"-1","0111010"},{"D","0001100"},
				  {"A","0110000"},{"M","1110000"},{"!D","0001101"},{"!A","0110001"},
				  {"!M","1110001"},{"-D","0001111"},{"-A","0110011"},{"-M","1110011"},
				  {"D+1","0011111"},{"A+1","0110111"},{"M+1","1110111"},
				  {"D-1","0001110"},{"A-1","0110010"},{"M-1","1110010"},
				  {"D+A","0000010"},{"D+M","1000010"},{"D-A","0010011"},
				  {"D-M","1010011"},{"A-D","0000111"},{"M-D","1000111"},
				  {"D&A","0000000"},{"D&M","1000000"},{"D|A","0010101"},
				  {"D|M","1010101"} };
				unordered_map<string,string> destToBinary =
				{ {"null","000"},{"M","001"},{"D","010"},{"MD","011"},{"A","100"},
				  {"AM","101"},{"AD","110"},{"AMD","111"} };
				unordered_map<string, string> jmpToBinary =
				{ {"null","000"},{"JGT","001"},{"JEQ","010"},{"JGE","011"},
				  {"JLT","100"},{"JNE","101"},{"JLE","110"},{"JMP","111"} };
				string dest = getDestinationFrom(instruction);
				string comp = getComputationFrom(instruction);
				string jmp = getJumpInstructionFrom(instruction);
				string opCode = "111";
				string machineInstruction = opCode + compToBinary.at(comp) +
					                        destToBinary.at(dest) + jmpToBinary.at(jmp);
				machineCode << machineInstruction << endl;
			}
		}
		std::getline(assemblyCode, currLine);
	}
	assemblyCode.close();
	machineCode.close();
}
/*
	Functionality: Receives a string (line) with an instruction in assembly code and extracts the
				   instruction from it.

	Algorithm:

	1. Initialize instruction variable 
	2. Count from 0 to the number of characters in the string (exclusive), for each character
	3.   If the character signals the beginning of a comment, break.
	4.   Else if the character is a space, continue
	5.   Else, update the instruction variable with the current character
	6. Return the instruction variable

	Complexity: O(n) where n is the length of the input string.
*/
string getInstructionFrom(string line)
{
	string instruction = "";
	for (int i = 0; i < line.length(); i++)
	{
		char currChar = line[i];
		bool charIsBegOfComment = (currChar == '/');
		bool charIsSpace = (currChar == ' ');

		if (charIsBegOfComment) break;
		else if (charIsSpace) continue;
		else
		{
			instruction += currChar;
		}
	}
	return instruction;
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
/*
	Functionality: Receives a string with a HACK assembly C instruction (i) and outputs
	               a string with just the destination part of it.

	Algorithm:
	1. If there is an equal sign:
	2.   extract destination
	3.   return it
	4. If there is not an equal sign:
	5.   return "null"

	Complexity:
	1. Searching the string for an equal sign is O(n)
	2. Getting substring is O(n)
	3. O(n) in general
*/
string getDestinationFrom(string i)
{
	size_t equalPos = i.find('=');
	bool thereIsEqualSign = (equalPos != string::npos);

	if (thereIsEqualSign)
	{
		string dest = i.substr(0, equalPos);
		return dest;
	}
	return "null";

}
/*
	Functionality: Receives a HACK assembly C instruction (i) and outputs a string with
	               only the computation part

	Algorithm:
	1. Determine if instruction has equal sign and/or semicolon
	2. Extract computation based on case
	3. Return it

	Complexity:

*/
string getComputationFrom(string i)
{
	size_t equalPos = i.find('=');
	size_t semicolonPos = i.find(';');
	bool thereIsEqualSign = (equalPos != string::npos);
	bool thereIsSemicolon = (semicolonPos != string::npos);

	if (thereIsEqualSign)
	{
		if (thereIsSemicolon)
		{
			string comp = i.substr(equalPos + 1, semicolonPos - equalPos - 1);
			return comp;
		}
		else
		{
			string comp = i.substr(equalPos + 1);
			return comp;
		}
	}
	else
	{
		if (thereIsSemicolon)
		{
			string comp = i.substr(0, semicolonPos);
			return comp;
		}
		else
		{
			string comp = i.substr(0);
			return comp;
		}
	}
}
/*
	Functionality: Receives a string with a HACK assembly language instruction (i) and
	               returns a string with just the jump instruction. If there is no jump 
				   instruction, the string "null" is returned.

	Algorithm:
	
	1. If there is a jump instruction:
	2.   Extract the jump instruction
	3.   Return it
	4. If there is not a jump instruction:
	5.   Return "null"
*/
string getJumpInstructionFrom(string i)
{
	size_t semicolonPos = i.find(';');
	bool thereIsSemicolon = (semicolonPos != string::npos);

	if (thereIsSemicolon)
	{
		string jumpInst = i.substr(semicolonPos + 1);
		return jumpInst;
	}
	else
	{
		return "null";
	}
}