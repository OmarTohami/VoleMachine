#include "A1_Task4_S25-S26_20230269-20230520-20230134.h"
#include <algorithm>
#include <cmath>
#include <bitset>
#include <iomanip>

// File: VoleMachine.cpp
// Purpose: 
// Author: Omar Hani , Youana Joseph , Dima Khaled
// Section: S25,S26
// ID: 20230269 , 20230520 , 20230134
// TA: Bassant samer , Reem Ehab
// Date: 7 Nov 2024
// Purpose: This program implements a simulator for the Vole machine and its language that is capable of simulating
// its operation and running the program.


// Function To Convert From Decimal To Hexa
string ALU::DecimalToHex(int decimalValue) {
    string hexValue = "";
    while (decimalValue != 0) {
        int remainder = decimalValue % 16;
        char hexChar = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
        hexValue += hexChar;
        decimalValue /= 16;
    }
    reverse(hexValue.begin(), hexValue.end());
    return hexValue.empty() ? "00" : hexValue;
}

// Function To Convert From Hexa To Decimal
int ALU::HexToDecimal(const string& hexValue) {
    int decimalValue = 0;
    int base = 1;
    for (int i = hexValue.size() - 1; i >= 0; i--) {
        if (isdigit(hexValue[i])) {
            decimalValue += (hexValue[i] - '0') * base;
        } else if (isupper(hexValue[i])) {
            decimalValue += (hexValue[i] - 'A' + 10) * base;
        }
        base *= 16;
    }
    return decimalValue;
}

// Function To Convert From Hexa To ASCII
string ALU::HexToASCII(const string& hexValue) {
    string ascii;
    for (size_t i = 0; i < hexValue.length(); i += 2) {
        std::string part = hexValue.substr(i, 2);
        char ch = static_cast<char>(stoul(part, nullptr, 16));
        ascii += ch;
    }
    return ascii;
}

// Register Class
Register::Register(int size) : registerValues(size, "00") {}

// Constructor For Register Class
void Register::SetValue(int index, const string& value) {
    registerValues[index] = value;
}

// Function To Get The Value Of A Register
string Register::GetValue(int index) const {
    return registerValues[index];
}


Memory::Memory(int size) : memoryValues(size, "00") {}

// Constructor For Memory Class
void Memory::SetValue(int index, const string& value) {
    memoryValues[index] = value;
}

// Function To Get The Value Of A Memory
string Memory::GetValue(int index) const {
    return memoryValues[index];
}


CU::CU(Register& reg, Memory& mem) : registers(reg), memory(mem) {}


void CU::LoadMemoryToRegister(const string& command) {
    string registerIndex(1, command[1]);
    registers.SetValue(ALU::HexToDecimal(registerIndex), memory.GetValue(ALU::HexToDecimal(command.substr(2, 4))));
}


void CU::ImmediateLoadToRegister(const string& command) {
    string registerIndex(1, command[1]);
    registers.SetValue(ALU::HexToDecimal(registerIndex), command.substr(2, 4));
}


void CU::StoreRegisterToMemory(const string& command) {
    string registerIndex(1, command[1]);
    if (command[2] != '0' || command[3] != '0') {
        memory.SetValue(ALU::HexToDecimal(command.substr(2, 4)), registers.GetValue(ALU::HexToDecimal(registerIndex)));
    } else {
        memory.SetValue(0, registers.GetValue(ALU::HexToDecimal(registerIndex)));

    }
}


void CU::CopyRegister(const string& command) {
    string sourceRegisterIndex(1, command[2]);
    string destinationRegisterIndex(1, command[3]);
    registers.SetValue(ALU::HexToDecimal(destinationRegisterIndex), registers.GetValue(ALU::HexToDecimal(sourceRegisterIndex)));
}


void CU::AddRegistersAsIntegers(const string& command) {
    string destRegisterIndex(1, command[1]);
    string sourceRegisterIndex1(1, command[2]);
    string sourceRegisterIndex2(1, command[3]);

    // Retrieve the values from the source registers as integers
    int value1 = ALU::HexToDecimal(registers.GetValue(ALU::HexToDecimal(sourceRegisterIndex1)));
    int value2 = ALU::HexToDecimal(registers.GetValue(ALU::HexToDecimal(sourceRegisterIndex2)));

    // Helper function to get two's complement in binary representation
    auto toTwosComplementBinary = [](int number) -> string {
        if (number >= 0) {
            // For non-negative numbers, convert directly to binary
            return bitset<8>(number).to_string();
        } else {
            // For negative numbers, calculate two's complement
            int absValue = -number; // get the absolute value
            bitset<8> binary(absValue); // convert to binary (8 bits for simplicity)
            bitset<8> onesComplement = ~binary; // compute one's complement
            bitset<8> twosComplement = onesComplement.to_ulong() + 1; // add 1 to get two's complement
            return twosComplement.to_string();
        }
    };

    // Get the binary representations in two's complement format
    string binaryValue1 = toTwosComplementBinary(value1);
    string binaryValue2 = toTwosComplementBinary(value2);

    // Convert binary strings to integers and perform binary addition
    int sum = bitset<8>(binaryValue1).to_ulong() + bitset<8>(binaryValue2).to_ulong();

    // Convert the sum to binary, ensuring it's also in two's complement form if negative
    bitset<8> binarySum(sum);

    // Convert the binary sum to hexadecimal
    stringstream hexStream;
    hexStream << hex << uppercase << binarySum.to_ulong();
    string hexResult = hexStream.str();

    // Ensure the result is two characters (e.g., "0A" instead of "A") for consistency.
    registers.SetValue(ALU::HexToDecimal(destRegisterIndex), hexResult.size() == 1 ? "0" + hexResult : hexResult);
}


void CU::FloatingPointAddition(const string& command) {
    string destRegisterIndex(1, command[1]);
    string srcRegisterIndex1(1, command[2]);
    string srcRegisterIndex2(1, command[3]);

    int bias = 4;

    auto ConvertHexToCustomFloat = [&](const string& hexValue) -> pair<float, int> {
        int hexInt = ALU::HexToDecimal(hexValue);
        int sign = (hexInt >> 7) & 1;
        int exponent = (hexInt >> 4) & 0x07;
        int mantissa = hexInt & 0x0F;

        if (exponent == 0) {
            return {0.0f, 0};
        }

        int adjustedExponent = exponent - bias;
        float fractionalMantissa = 1.0f + mantissa / 16.0f;
        float result = (sign ? -1 : 1) * fractionalMantissa * pow(2, adjustedExponent);

        return {result, adjustedExponent}; // Return adjusted exponent
    };

    auto ConvertFloatToCustomHex = [&](float value) -> string {
        if (value == 0.0f) {
            return "00";
        }

        int sign = value < 0 ? 1 : 0;
        float absValue = fabs(value);
        int exponent = bias;

        // Normalize the value
        while (absValue >= 1.0f && exponent < 7) {
            absValue /= 2;
            exponent++;
        }
        while (absValue < 1.0f && exponent > 0) {
            absValue *= 2;
            exponent--;
        }

        absValue -= 1.0f;
        int mantissa = static_cast<int>(absValue * 16) & 0x0F;

        int finalResult = (sign << 7) | ((exponent & 0x07) << 4) | mantissa;
        return ALU::DecimalToHex(finalResult);
    };

    // Store values and exponents to avoid redundant calculations
    auto [value1, exp1] = ConvertHexToCustomFloat(registers.GetValue(ALU::HexToDecimal(srcRegisterIndex1)));
    auto [value2, exp2] = ConvertHexToCustomFloat(registers.GetValue(ALU::HexToDecimal(srcRegisterIndex2)));

    if (value1 == 0.0f) {
        registers.SetValue(ALU::HexToDecimal(destRegisterIndex), ConvertFloatToCustomHex(value2));
        return;
    }
    if (value2 == 0.0f) {
        registers.SetValue(ALU::HexToDecimal(destRegisterIndex), ConvertFloatToCustomHex(value1));
        return;
    }

    // Align exponents
    while (exp1 > exp2) {
        value2 /= 2;
        exp2++;
    }
    while (exp2 > exp1) {
        value1 /= 2;
        exp1++;
    }

    // Now add the mantissas
    float floatResult = value1 + value2;

    if (floatResult != 0.0f) {
        int resultSign = floatResult < 0 ? 1 : 0;
        float absResult = fabs(floatResult);
        int resultExponent = bias;

        // Normalize the result
        while (absResult >= 1.0f && resultExponent < 7) {
            absResult /= 2;
            resultExponent++;
        }
        while (absResult < 1.0f && resultExponent > 0) {
            absResult *= 2;
            resultExponent--;
        }

        absResult -= 1.0f;
        int resultMantissa = static_cast<int>(absResult * 16) & 0x0F;

        // Final result construction
        int finalResult = (resultSign << 7) | ((resultExponent & 0x07) << 4) | resultMantissa;

        // Ensure hex result is in 2 characters (leading zero if necessary)
        string hexResult = ALU::DecimalToHex(finalResult);
        registers.SetValue(ALU::HexToDecimal(destRegisterIndex), hexResult.size() == 1 ? "0" + hexResult : hexResult);
    } else {
        registers.SetValue(ALU::HexToDecimal(destRegisterIndex), "00");
    }
}


Machine::Machine() : memory(256), registers(16), controlUnit(registers, memory) {}

string Machine::InputFileName() {
    string FileName = "";
    cout << "Enter file name with extension:\n";
    cin >> FileName;
    return FileName;
}

void Machine::LoadProgramFile(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cout << "Failed to open the file. Program terminated." << endl;
        return;
    }

    string line;
    int instructionCounter = 0;

    // Read file line by line
    while (getline(inputFile, line)) {
        istringstream instructionStream(line);
        string opcode;

        // Process each opcode in the line
        while (instructionStream >> opcode) {
            for (char &ch : opcode) {
                if (isalpha(ch)) {
                    ch = toupper(ch);
                }
            }
            if (!IsValidInstruction(opcode)) {
                cout << "Invalid instruction: " << opcode << endl;
                inputFile.close(); // Close the file before returning
                return; // Stop loading if an invalid instruction is found
            }
            memory.SetValue(instructionCounter, opcode.substr(0, 2));
            memory.SetValue(instructionCounter + 1, opcode.size() > 2 ? opcode.substr(2, 2) : "00");
            instructionCounter += 2;
        }
    }
    inputFile.close();
    cout << "Program loaded successfully from the file\n";
}


void Machine::ExecuteCommand(const string& command) {
    switch (command[0]) {
        case '1': // Load memory to register
            controlUnit.LoadMemoryToRegister(command);
            break;
        case '2': // Immediate load to register
            controlUnit.ImmediateLoadToRegister(command);
            break;
        case '3': // Store register to memory or print
            controlUnit.StoreRegisterToMemory(command);
            break;
        case '4': // Copy register
            controlUnit.CopyRegister(command);
            break;
        case '5': // Add registers as integers
            controlUnit.AddRegistersAsIntegers(command);
            break;
        case '6': // Floating-point addition
            controlUnit.FloatingPointAddition(command);
            break;
    }
}

// Helper function to validate the instruction
bool Machine::IsValidInstruction(const string& instruction) {
    if (instruction.length() < 4) {
        return false; // Must be exactly 4 characters
    }
    for (char c : instruction) {
        if (!isxdigit(c)) {
            return false; // Must be a valid hex digit
        }
    }
    return true; // Valid instruction
}


void Machine::RunProgram() {
    instructionCounter = 0; // Initialize program counter
    int Counter = 1;
    while (instructionCounter < 256) {
        string instruction = memory.GetValue(instructionCounter) + memory.GetValue(instructionCounter + 1);

        // Stop if the instruction is empty or if we have a halt ('C') opcode
        if (instruction == "0000" || instruction[0] == 'C'  || instruction[0] == 'c') {

            cout << "Excuting Instruction " << Counter << " : " << instruction << " at program Counter " << " (" << ALU::DecimalToHex(instructionCounter) << "): " <<  endl;
            break;
        }

        // Print instruction with formatted output
        cout << "Excuting Instruction " << Counter << " : " << instruction << " at program Counter " << " (" << ALU::DecimalToHex(instructionCounter) << "): " <<  endl;

        // Handle branch instructions
        if (instruction[0] == 'B') {
            string branchRegisterIndex(1, instruction[1]);
            if (ALU::HexToDecimal(registers.GetValue(ALU::HexToDecimal(branchRegisterIndex))) == ALU::HexToDecimal(registers.GetValue(0))) {
                instructionCounter = ALU::HexToDecimal(instruction.substr(2, 2)); // Jump to branch address
            } else {
                instructionCounter += 2; // Move to the next instruction
            }
        } else {
            ExecuteCommand(instruction); // Execute non-branch commands
            instructionCounter += 2; // Move to the next instruction
        }
        Counter++;

        // Out-of-bounds check
        if (instructionCounter < 0 || instructionCounter >= 256) {
            cout << "Error: Program counter out of bounds. Halting execution." << endl;
            break;
        }
    }
}


void Machine::PrintMemory() {
    cout << "                 MAIN MEMORY IN HEX" << endl;

    // Print the header row with hexadecimal column numbers (0-F)
    for (int j = 0; j < 16; ++j) {
        string address = (j < 10) ? to_string(j) : string(1, 'A' + (j - 10));
        cout << setw(2) << setfill(' ') << address << " ";
    }
    cout << endl;

    // Print memory values in hexadecimal
    for (int i = 0; i < 16; ++i) {
        string address = (i < 10) ? to_string(i) : string(1, 'A' + (i - 10));
        cout << address << " ";

        for (int j = 0; j < 16; ++j) {
            cout << setw(2) << setfill('0') << std::hex << memory.GetValue(i * 16 + j) << " ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "                 MAIN MEMORY IN ASCII" << endl;

    // Print the header row for ASCII
    for (int j = 0; j < 16; ++j) {
        string address = (j < 10) ? to_string(j) : string(1, 'A' + (j - 10));
        cout << setw(2) << setfill(' ') << address << " ";
    }
    cout << endl;

    // Print memory values in ASCII format
    for (int i = 0; i < 16; ++i) {
        string address = (i < 10) ? to_string(i) : string(1, 'A' + (i - 10));
        cout << address << " ";

        for (int j = 0; j < 16; ++j) {
            // Get value as a string in hex format
            string valueStr = memory.GetValue(i * 16 + j);

            // Convert hex string to an integer
            int value;
            std::stringstream ss;
            ss << std::hex << valueStr;
            ss >> value;

            // Convert to ASCII, handling non-printable characters
            char asciiChar = (value >= 32 && value <= 126) ? static_cast<char>(value) : '.'; // '.' for non-printable
            cout << setw(2) << setfill(' ') << asciiChar << " ";
        }
        cout << endl;
    }

    cout << endl;
}



void Machine::PrintRegisters() {
    // Print registers in hexadecimal format
    cout << "Registers In HEX:" << endl;
    for (int i = 0; i < 16; ++i) {
        string regName = (i < 10) ? to_string(i) : string(1, 'A' + (i - 10));
        cout << "Register " << regName << ": " << setw(2) << setfill('0') << registers.GetValue(i) << endl;
    }
    cout << endl;

    // Print registers in ASCII format
    cout << "Registers In ASCII:" << endl;
    for (int i = 0; i < 16; ++i) {
        string valueStr = registers.GetValue(i);

        // Convert hex string to ASCII using HexToASCII
        string asciiValue = ALU::HexToASCII(valueStr);

        // Handle non-printable characters
        string asciiDisplay;
        for (char ch : asciiValue) {
            if (ch >= 32 && ch <= 126) {
                asciiDisplay += ch; // Add printable characters
            } else {
                asciiDisplay += ' '; // Replace non-printable characters with '?'
            }
        }

        string regName = (i < 10) ? to_string(i) : string(1, 'A' + (i - 10));
        cout << "Register " << regName << ": " << asciiDisplay << endl;
    }
    cout << endl;

    // Print the formatted program counter
    string HexCounter = ALU::DecimalToHex(instructionCounter);
    if (instructionCounter < 16) {
        HexCounter = "0" + HexCounter;
    }
    cout << "Program Counter = " << HexCounter << endl << endl;
}


void MainUI::DisplayMenu(){
    cout << "Menu:\n";
    cout << "1. Load Program from a file\n";
    cout << "2. Execute Program\n";
    cout << "3. Display status\n";
    cout << "4. Exit\n";
}


int Machine::InputChoice(int From, int To) {
    int Choice;
    while (true) {
        cout << "Please enter your choice:" << endl;
        cin >> Choice;
        if (cin.good() && cin.peek() == '\n' && Choice >= From && Choice <= To) {
            // Input is valid and within range
            return Choice;
        } else {
            // Clear error state if invalid, ignore remaining characters in input
            cin.clear();
            cin.ignore(256, '\n');
            cout << "Invalid Input. ";
        }
    }
}


int MainUI::BeginProgram() {
    while (true){
        DisplayMenu();
        int Choice = machine.InputChoice(1, 4);
        switch (Choice) {
            case 1:
                machine.LoadProgramFile(machine.InputFileName());
                break;
            case 2:
                machine.RunProgram();
                break;
            case 3:
                machine.PrintMemory();
                machine.PrintRegisters();
                break;
            case 4:
                cout << "Program Terminated." << endl;
                return 0;
            default:
                cout << "Invalid Choice. Please Try Again." << endl;
                break;
        }
    }
}


