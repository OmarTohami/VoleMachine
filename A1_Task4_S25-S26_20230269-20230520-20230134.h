#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

// File: VoleMachine.h
// Author: Omar Hani , Youana Joseph , Dima Khaled
// Section: S25,S26
// ID: 20230269 , 20230520 , 20230134
// TA: Bassant samer , Reem Ehab
// Date: 7 Nov 2024
// Purpose: This Is The Header For The Code

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>


using namespace std;

class ALU{
public:
    static string DecimalToHex(int decimalValue);
    static int HexToDecimal(const string& hexValue);
    static string HexToASCII(const string& hexValue);
};



class Register{
    vector<string> registerValues;

public:
    Register(int size);
    void SetValue(int index, const string& value);
    string GetValue(int index) const;
};


class Memory {
    vector<string> memoryValues;

public:
    Memory(int size);
    void SetValue(int index, const string& value);
    string GetValue(int index) const;
};


class CU {
    Register& registers;
    Memory& memory;

public:
    CU(Register& reg, Memory& mem);

    void LoadMemoryToRegister(const string& command);

    void ImmediateLoadToRegister(const string& command);

    void StoreRegisterToMemory(const string& command);

    void CopyRegister(const string& command);

    void AddRegistersAsIntegers(const string& command);

    void FloatingPointAddition(const string& command);

};


class Machine {
private:

    Memory memory; // Fixed size memory (256)
    Register registers; // Fixed size register (16)
    CU controlUnit; // Control Unit

public:
    Machine();

    int instructionCounter = 10;

    void LoadProgramFile(const string& filename);

    void ExecuteCommand(const string& command);

    void RunProgram();

    void PrintMemory();

    void PrintRegisters();

    string InputFileName();

    bool IsValidInstruction(const string& instruction);

    int InputChoice(int From, int To);

};

class MainUI{
private:
    Machine machine;
public:
    void DisplayMenu();

    int BeginProgram();
};

#endif