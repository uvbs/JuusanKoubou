from Common import *
from Assembler import Flags, Instruction, InstructionTable, InstructionDescriptor, OperandDescriptor
from Assembler import *
from .types import *

__all__ = (
    'ScenaOpTable',
)

NoOperand = InstructionDescriptor.NoOperand

class ED6FCInstructionTable(InstructionTable):
    def readOpCode(self, fs: fileio.FileStream) -> int:
        return fs.ReadByte()

    def writeOpCode(self, fs: fileio.FileStream, inst: 'Instruction'):
        fs.WriteByte(inst.opcode)

for i in ED6FCOperandType:
    globals()[i.name] = i

def inst(opcode: int, mnemonic: str, operandfmts: str = None, flags: Flags = Flags.Empty, handler: InstructionHandler = None) -> InstructionDescriptor:
    if operandfmts is NoOperand:
        operands = NoOperand
    else:
        operands = ED6FCOperandDescriptor.fromFormatString(operandfmts)

    return InstructionDescriptor(opcode = opcode, mnemonic = mnemonic, operands = operands, flags = flags, handler = handler)

def Handler_Jc():
    pass

def Handler_Switch():
    pass

ScenaOpTable = ED6FCInstructionTable([
    inst(0x00, 'ExitThread'),
    inst(0x01, 'Return',        NoOperand,          Flags.EndBlock),
    inst(0x02,  'Jc',           NoOperand,          Flags.StartBlock,    Handler_Jc),
    inst(0x03,  'Jump',         'o',                Flags.Jump),
    inst(0x04,  'Switch',       NoOperand,          Flags.EndBlock,      Handler_Switch),
    inst(0x05,  'Call',         'CH'),          # Call(scp index, func index)
])
