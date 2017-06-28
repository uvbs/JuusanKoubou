from Common import *
from enum import IntEnum
from . import instruction

__all__ = (
    'OperandDescriptor',
    'InstructionDescriptor',
    'InstructionHandler',
    'InstructionHandlerInfo',
    'InstructionTable',
)

class OperandFormat(IntEnum):
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

    _sizeTable = {
        SInt8      : 1,
        SInt16     : 2,
        SInt32     : 4,
        SInt64     : 8,

        UInt8      : 1,
        UInt16     : 2,
        UInt32     : 4,
        UInt64     : 8,

        SHex8      : 1,
        SHex16     : 2,
        SHex32     : 4,
        SHex64     : 8,

        UHex8      : 1,
        UHex16     : 2,
        UHex32     : 4,
        UHex64     : 8,

        Float32    : 4,
        Float64    : 8,

        MBCS       : None,
        Bytes      : None,
    }

    def __repr__(self):
        return super().__repr__().rsplit('.', 1)[-1].split(':', 1)[0]

    @property
    def size(self):
        return self.sizeTable.get(self)

class FormatOperandHandlerInfo:
    def __init__(self):
        pass

FormatOperandHandler = Callable[[FormatOperandHandlerInfo], Any]

class OperandDescriptor:
    formatTable = {
        'c' : OperandDescriptor(OperandFormat.SInt8, hex = False),
    }

    @classmethod
    def fromFormatString(cls, fmtstr: str, formatTable: Dict[str, OperandDescriptor] = OperandDescriptor.formatTable):
        return [formatTable[f] for f in fmtstr]

    def __init__(self, format: OperandFormat, hex: bool = False, encoding: str = 'mbcs', formatHandler: FormatOperandHandler = None):
        self.format     = format                    # type: OperandFormat
        self.hex        = hex                       # type: bool
        self.encoding   = encoding                  # type: str
        self.handler    = formatHandler             # type: FormatOperandHandler

class InstructionHandlerInfo:
    def __init__(self):
        pass

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

    def fromOpCode(self, opcode: int) -> InstructionDescriptor:
        return self.lookup[opcode]

    def readInstruction(self, fs: fileio.FileStream) -> 'instruction.Instruction':
        raise NotImplementedError

    def readOpCode(self, fs: fileio.FileStream) -> int:
        raise NotImplementedError

    def writeInstruction(self, fs: fileio.FileStream, inst: 'instruction.Instruction'):
        raise NotImplementedError

    def writeOpCode(self, fs: fileio.FileStream, inst: 'instruction.Instruction'):
        raise NotImplementedError

    def __str__(self):
        return '\n'.join(['%s' % x for x in self.descriptors])
