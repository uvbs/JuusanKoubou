from ml import *
import Assembler

def main():
    print(Assembler.Instruction.Instruction)
    print(Assembler.InstructionTable.InstructionTable)
    print(Assembler.Instruction.InstructionDescriptor)

    f = Assembler.Instruction.Flags.StartBlock | Assembler.Instruction.Flags.EndBlock

    print(f.isStartBlock)
    print(f.isEndBlock)

    console.pause('done')

if __name__ == '__main__':
    Try(main)
