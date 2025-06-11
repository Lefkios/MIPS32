#include "Registers.h"
#include <iostream>

using namespace std;

Registers::Registers() {
    // Initialize the 33 registers with 0
    registers.resize(33,0);

    registers[28] = 0x010008000; //$gp
    registers[29] = 0x7ffffffc; //$sp
    registers[32]= 0; //$zero

    // Initialize the mapping of names to indices
    initializeRegisterMap();
}

Registers::~Registers() {}

//map register names to their index
void Registers::initializeRegisterMap() {
    registerMap["$r0"]=0;
    registerMap["$at"]=1;
    registerMap["$v0"]=2;
    registerMap["$v1"]=3;
    registerMap["$a0"]=4;
    registerMap["$a1"]=5;
    registerMap["$a2"]=6;
    registerMap["$a3"]=7;
    registerMap["$t0"]=8;
    registerMap["$t1"]=9;
    registerMap["$t2"]=10;
    registerMap["$t3"]=11;
    registerMap["$t4"]=12;
    registerMap["$t5"]=13;
    registerMap["$t6"]=14;
    registerMap["$t7"]=15;
    registerMap["$s0"]=16;
    registerMap["$s1"]=17;
    registerMap["$s2"]=18;
    registerMap["$s3"]=19;
    registerMap["$s4"]=20;
    registerMap["$s5"]=21;
    registerMap["$s6"]=22;
    registerMap["$s7"]=23;
    registerMap["$t8"]=24;
    registerMap["$t9"]=25;
    registerMap["$k0"]=26;
    registerMap["$k1"]=27;
    registerMap["$gp"]=28;    //global area pointer
    registerMap["$sp"]=29;    //stack pointer
    registerMap["$fp"]=30;
    registerMap["$ra"]=31;
     registerMap["$zero"]=32;

}

//set a registers value using its name
void Registers::setRegister(const std::string& regName, int value) {
        registers[registerMap[regName]]=value;
        cout<<value;
}

//get a registers value using its name
int Registers::getRegister(const std::string& regName) const {
        return registers[registerMap.at(regName)];
}

void Registers::printRegisters(std::ostream& out) const {
    for (int i=0;i<31;i++) {
        out<<std::nouppercase<<std::hex<<registers[i]<<"\t"<<std::dec;
    }
    out<<hex<<registers[31]<<dec<<endl;
}


