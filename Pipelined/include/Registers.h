#ifndef REGISTERS_H
#define REGISTERS_H
#include <string>
#include <vector>
#include <map>

class Registers
{
    public:
        Registers();
        virtual ~Registers();

        // Setter and Getter for normal registers using register names
        void setRegister(const std::string& regName, int value);
        int getRegister(const std::string& regName) const;

        // Temporary Register Setters and Getters
void setIR(const std::string& instruction);
std::string getIR() const;


        void setMDR(int value);
        int getMDR() const;

        void setA(int value);
        int getA() const;

        void setB(int value);
        int getB() const;

        void setALUOut(int value);
        int getALUOut() const;

        // Print all register values
        void printRegisters(std::ostream& out) const;

    private:
        std::vector<int> registers;  // Stores normal register values
        std::map<std::string, int> registerMap; // Maps names to indices

        void initializeRegisterMap();

        // Temporary Registers for Multi-Cycle Execution
        std::string IR;      // Instruction Register
        int MDR;     // Memory Data Register
        int A, B;    // Register Read Data
        int ALUOut;  // ALU Output
};

#endif // REGISTERS_H
