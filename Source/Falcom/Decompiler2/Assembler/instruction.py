from Common import *
from enum import IntFlag, IntEnum
from . import function
from . import instruction_table

__all__ = (
    'Label',
    'Operand',
    'Flags',
    'Instruction',
)

class Label:
    def __init__(self, label: str, offset: int):
        self.label = label          # type: str
        self.offset = offset        # type: int

class Operand:
    class Format(IntEnum):
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
        _ = range(22)

        def __init__(self, *args, **kwargs):
            super(IntEnum, self).__init__()

            self.lower = False      # type: bool
            self.encoding = 'mbcs'  # type: str

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

    def __init__(self):
        self.operand    = None                      # type: int
        self.size       = None                      # type: int
        self.format     = Operand.Format.Empty      # type: Operand.Format

class Flags(IntFlag):
    EndBlock            = 1 << 0
    StartBlock          = 1 << 1
    Call                = (1 << 2) | StartBlock
    Jump                = (1 << 3) | EndBlock

    FormatArgNewLine    = 1 << 4

    @property
    def isEndBlock(self):
        return bool(self.value & self.EndBlock)

    @property
    def isStartBlock(self):
        return bool(self.value & self.StartBlock)

    @property
    def isCall(self):
        return bool(self.value & self.Call & ~self.StartBlock)

    @property
    def isJump(self):
        return bool(self.value & self.Jump & ~self.EndBlock)

    @property
    def isArgNewLine(self):
        return bool(self.value & self.FormatArgNewLine)

class Instruction:
    def __init__(self):
        self.op         = None      # type: int
        self.operands   = []        # type: List[Operand]
        self.branches   = []        # type: List[function.CodeBlock]
        self.descriptor = None      # type: instruction_table.InstructionDescriptor
        self.label      = None      # type: Label
        self.flags      = None      # type: Flags
