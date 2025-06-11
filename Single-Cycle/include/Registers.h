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
         // Setter and Getter for registers using register names
    void setRegister(const std::string& regName, int value);
    int getRegister(const std::string& regName) const;

    // Print all register values
    void printRegisters(std::ostream& out) const;

    protected:

    private:
        std::vector<int> registers;  // Stores register values
        std::map<std::string, int> registerMap; // Maps names to indices

    void initializeRegisterMap();
};

#endif // REGISTERS_H
