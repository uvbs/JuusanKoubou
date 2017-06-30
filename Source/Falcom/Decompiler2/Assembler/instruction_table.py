from Common import *
from enum import IntEnum
from . import instruction

if TYPE_CHECKING:
    from . import disassembler

__all__ = (
    'OperandType',
    'OperandFormat',
    'OperandDescriptor',
    'InstructionDescriptor',
    'InstructionHandler',
    'InstructionHandlerInfo',
    'InstructionTable',
)

class OperandType(IntEnum):
    Empty,      \
    SInt8,      \
    SInt16,     \
    SInt32,     \
    SInt64,     \
    UInt8,      \
    UInt16,     \
    UInt32,     \
    UInt64,     \
    SHex8,      \
    SHex16,     \
    SHex32,     \
    SHex64,     \
    UHex8,      \
    UHex16,     \
    UHex32,     \
    UHex64,     \
    Float32,    \
    Float64,    \
    MBCS,       \
    Bytes,      \
    UserDefined = range(22)

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name

class OperandFormat:
    sizeTable = {
        OperandType.SInt8   : 1,
        OperandType.SInt16  : 2,
        OperandType.SInt32  : 4,
        OperandType.SInt64  : 8,

        OperandType.UInt8   : 1,
        OperandType.UInt16  : 2,
        OperandType.UInt32  : 4,
        OperandType.UInt64  : 8,

        OperandType.SHex8   : 1,
        OperandType.SHex16  : 2,
        OperandType.SHex32  : 4,
        OperandType.SHex64  : 8,

        OperandType.UHex8   : 1,
        OperandType.UHex16  : 2,
        OperandType.UHex32  : 4,
        OperandType.UHex64  : 8,

        OperandType.Float32 : 4,
        OperandType.Float64 : 8,

        OperandType.MBCS    : None,
        OperandType.Bytes   : None,
    }

    def __init__(self, oprType: OperandType, hex: bool = False, encoding: str = 'mbcs'):
        self.type       = oprType                   # type: OperandType
        self.hex        = hex                       # type: bool
        self.encoding   = encoding                  # type: str

    def __str__(self):
        return repr(self.type)

    def __repr__(self):
        return str(self)

    @property
    def size(self):
        return self.sizeTable.get(self.type)

class FormatOperandHandlerInfo:
    def __init__(self, format: OperandFormat):
        self.format = format                        # type: OperandFormat

FormatOperandHandler = Callable[[FormatOperandHandlerInfo], Any]

class OperandDescriptor:
    @classmethod
    def fromFormatString(cls, fmtstr: str, formatTable = None) -> 'Tuple[OperandDescriptor]':
        formatTable = formatTable if formatTable else cls.formatTable
        return tuple(formatTable[f] for f in fmtstr)

    def __init__(self, format: OperandFormat, formatHandler: FormatOperandHandler = None):
        self.format     = format                    # type: OperandFormat
        self.handler    = formatHandler             # type: FormatOperandHandler

    def readValue(self, fs: fileio.FileStream) -> Any:
        return {
            OperandType.SInt8   : lambda : fs.ReadChar(),
            OperandType.UInt8   : lambda : fs.ReadByte(),

            OperandType.SInt16  : lambda : fs.ReadShort(),
            OperandType.UInt16  : lambda : fs.ReadUShort(),

            OperandType.SInt32  : lambda : fs.ReadLong(),
            OperandType.UInt32  : lambda : fs.ReadULong(),

            OperandType.SInt64  : lambda : fs.ReadLong64(),
            OperandType.UInt64  : lambda : fs.ReadULong64(),

            OperandType.MBCS    : lambda : fs.ReadMultiByte(self.format.encoding),
        }[self.format.type]()

    def __str__(self):
        return repr(self.format)

    def __repr__(self):
        return self.__str__()

def oprdesc(*args, **kwargs):
    return OperandDescriptor(OperandFormat(*args, **kwargs))

OperandDescriptor.formatTable = {
    'c' : oprdesc(OperandType.SInt8, hex = False),
    'C' : oprdesc(OperandType.UInt8, hex = False),
    'b' : oprdesc(OperandType.SInt8, hex = True),
    'B' : oprdesc(OperandType.UInt8, hex = True),

    'h' : oprdesc(OperandType.SInt16, hex = False),
    'H' : oprdesc(OperandType.UInt16, hex = False),
    'w' : oprdesc(OperandType.SInt16, hex = True),
    'W' : oprdesc(OperandType.UInt16, hex = True),

    'i' : oprdesc(OperandType.SInt32, hex = False),
    'I' : oprdesc(OperandType.UInt32, hex = False),
    'l' : oprdesc(OperandType.SInt32, hex = True),
    'L' : oprdesc(OperandType.UInt32, hex = True),

    'q' : oprdesc(OperandType.SInt64, hex = True),
    'Q' : oprdesc(OperandType.UInt64, hex = True),

    'S' : oprdesc(OperandType.MBCS, encoding = DefaultEncoding)
}


class InstructionHandlerInfo:
    class Action(IntEnum):
        Disassemble = 0
        Assemble    = 1
        Format      = 2

    def __init__(self, action: 'InstructionHandlerInfo.Action', descriptor: 'InstructionDescriptor', disasmInfo: 'disassembler.DisassembleInfo'):
        self.action     = action                                    # type: InstructionHandlerInfo.Action
        self.descriptor = descriptor                                # type: InstructionDescriptor
        self.disasmInfo = disasmInfo                                # type: disassembler.DisassembleInfo
        self.offset     = instruction.Instruction.InvalidOffset     # type: int

InstructionHandler = Callable[[InstructionHandlerInfo], Any]

class InstructionDescriptor:
    NoOperand = None

    def __init__(self, opcode: int, mnemonic: str, operands: List[OperandDescriptor] = NoOperand, flags: 'instruction.Flags' = 0, handler: InstructionHandler = None):
        self.opcode     = opcode                    # type: int
        self.mnemonic   = mnemonic                  # type: str
        self.operands   = operands                  # type: List[OperandDescriptor]
        self.flags      = flags                     # type: instruction.Flags
        self.handler    = handler                   # type: InstructionHandler

    def __str__(self):
        return ' '.join([
            '0x%02X %s' % (self.opcode, self.mnemonic),
            self.operands and '%s' % (self.operands,) or '()',
            ('%s' % self.flags) if self.flags != instruction.Flags.Empty else '',
        ])

class InstructionTable:
    def __init__(self, descriptors: List[InstructionDescriptor]):
        self.descriptors    = descriptors           # type: List[InstructionDescriptor]
        self.lookup         = {}                    # type: Dict[int, InstructionDescriptor]

        for desc in self.descriptors:
            self.lookup[desc.opcode] = desc

    def getDescriptor(self, opcode: int) -> InstructionDescriptor:
        return self.lookup[opcode]

    def readOpCode(self, fs: fileio.FileStream) -> int:
        raise NotImplementedError

    def writeOpCode(self, fs: fileio.FileStream, inst: 'instruction.Instruction'):
        raise NotImplementedError

    def readInstruction(self, fs: fileio.FileStream) -> 'instruction.Instruction':
        raise NotImplementedError

    def writeInstruction(self, fs: fileio.FileStream, inst: 'instruction.Instruction'):
        raise NotImplementedError

    def readOperand(self, fs: fileio.FileStream, desc: OperandDescriptor) -> 'instruction.Operand':
        operand = instruction.Operand()

        operand.size = desc.format.size
        operand.descriptor = desc
        operand.value = desc.readValue(fs)

        return operand

    def writeOperand(self, fs: fileio.FileStream, operand: 'instruction.Operand'):
        raise NotImplementedError

    def __str__(self):
        return '\n'.join(['%s' % x for x in self.descriptors])
