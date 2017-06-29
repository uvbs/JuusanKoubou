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
        operands[0]

    return InstructionDescriptor(opcode = opcode, mnemonic = mnemonic, operands = operands, flags = flags, handler = handler)

def Handler_Jc():
    pass

def Handler_Switch():
    pass

def Handler_NewScene():
    pass

ScenaOpTable = ED6FCInstructionTable([
    inst(0x00,  'ExitThread'),
    inst(0x01,  'Return',                       NoOperand,          Flags.EndBlock),
    inst(0x02,  'Jc',                           NoOperand,          Flags.StartBlock,   Handler_Jc),
    inst(0x03,  'Jump',                         'o',                Flags.Jump),
    inst(0x04,  'Switch',                       NoOperand,          Flags.EndBlock,     Handler_Switch),
    inst(0x05,  'Call',                         'CH'),                                                      # Call(scp index, func index)
    inst(0x06,  'NewScene',                     'LCCC',             Flags.Empty,        Handler_NewScene),
    inst(0x07,  'IdleLoop'),
    inst(0x08,  'Sleep',                        'I'),
    inst(0x09,  'SetMapFlags',                  'L'),
    inst(0x0A,  'ClearMapFlags',                'L'),
    inst(0x0B,  'FadeToDark',                   'iic'),
    inst(0x0C,  'FadeToBright',                 'ii'),
    inst(0x0D,  'OP_0D'),
    inst(0x0E,  'Fade',                         'I'),
])
