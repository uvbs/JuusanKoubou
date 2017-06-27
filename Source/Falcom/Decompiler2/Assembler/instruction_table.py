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

class OperandDescriptor(IntEnum):
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
    Offset,     \
    MBCS,       \
    Bytes,      \
    UserDefined = range(23)

    def __init__(self, *args):
        super(IntEnum, self).__init__()

        self.lower      = False     # type: bool
        self.encoding   = 'mbcs'    # type: str

    def __repr__(self):
        return super().__repr__().rsplit('.', 1)[-1].split(':', 1)[0]

    @property
    def size(self):
        return {
            self.SInt8      : 1,
            self.SInt16     : 2,
            self.SInt32     : 4,
            self.SInt64     : 8,

            self.UInt8      : 1,
            self.UInt16     : 2,
            self.UInt32     : 4,
            self.UInt64     : 8,

            self.SHex8      : 1,
            self.SHex16     : 2,
            self.SHex32     : 4,
            self.SHex64     : 8,

            self.UHex8      : 1,
            self.UHex16     : 2,
            self.UHex32     : 4,
            self.UHex64     : 8,

            self.Float32    : 4,
            self.Float64    : 8,

            self.MBCS       : None,
            self.Bytes      : None,
        }[self]

class InstructionHandlerInfo:
    def __init__(self):
        pass

InstructionHandler = Callable[[InstructionHandlerInfo], Any]

class InstructionDescriptor:
    def __init__(self, opcode: int, mnemonic: str, operands: List[OperandDescriptor], flags: 'instruction.Flags' = 0, handler: InstructionHandler = None):
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
        return '\r\n'.join(['%s' % x for x in self.descriptors])
