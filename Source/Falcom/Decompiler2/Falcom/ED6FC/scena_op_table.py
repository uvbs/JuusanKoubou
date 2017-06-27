from Common import *
from Assembler import Flags, Instruction, InstructionTable, InstructionDescriptor, OperandDescriptor
from Assembler import *

__all__ = (
    'ScenaOpTable',
)

class ED6FCInstructionTable(InstructionTable):
    def readOpCode(self, fs: fileio.FileStream) -> int:
        return fs.ReadByte()

    def writeOpCode(self, fs: fileio.FileStream, inst: 'Instruction'):
        fs.WriteByte(inst.opcode)

for i in OperandDescriptor:
    globals()[i.name] = i

def inst(opcode: int, mnemonic: str, operands: List[OperandDescriptor] = None, flags: Flags = Flags.Empty, handler: InstructionHandler = None) -> InstructionDescriptor:
    if operands == OperandDescriptor.Empty:
        operands = None

    if operands and not isinstance(operands, (list, tuple)):
        operands = (operands,)

    return InstructionDescriptor(opcode = opcode, mnemonic = mnemonic, operands = operands, flags = flags, handler = handler)

def Handler_Jc():
    pass

def Handler_Switch():
    pass

ScenaOpTable = ED6FCInstructionTable([
    inst(0x00, 'ExitThread'),
    inst(0x01, 'Return',        Empty,          Flags.EndBlock),
    inst(0x02,  'Jc',           Empty,          Flags.StartBlock,    Handler_Jc),
    inst(0x03,  'Jump',         Offset,         Flags.Jump),
    inst(0x04,  'Switch',       Empty,          Flags.EndBlock,      Handler_Switch),
    inst(0x05,  'Call',         (UInt8, UInt16)),          # Call(scp index, func index)
])
