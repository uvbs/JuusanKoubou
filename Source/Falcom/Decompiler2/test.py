from ml import *
import Assembler

def main():
    print(Assembler.Instruction)
    print(Assembler.InstructionTable)
    print(Assembler.InstructionDescriptor)

    f = Assembler.Flags.StartBlock | Assembler.Flags.EndBlock
    print(f)

    inst = Assembler.Instruction()
    blk = Assembler.CodeBlock(None)
    tbl = Assembler.InstructionTable(None)

    blk.instructions[0]
    inst.branches[0]
    tbl.decriptors[0]

    print(f.isStartBlock)
    print(f.isEndBlock)

    console.pause('done')

if __name__ == '__main__':
    Try(main)
