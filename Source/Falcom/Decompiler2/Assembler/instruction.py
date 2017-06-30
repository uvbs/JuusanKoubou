from Common import *
from enum import IntFlag
from . import function
from . import instruction_table

__all__ = (
    'Label',
    'Operand',
    'Flags',
    'Instruction',
)

OperandDescriptor = instruction_table.OperandDescriptor

class Label:
    def __init__(self, label: str, offset: int):
        self.label = label                          # type: str
        self.offset = offset                        # type: int

class Operand:
    def __init__(self):
        self.value      = None                      # type: Any
        self.size       = None                      # type: int
        self.descriptor = OperandDescriptor.Empty   # type: OperandDescriptor

class Flags(IntFlag):
    Empty               = 0
    EndBlock            = 1 << 0
    StartBlock          = 1 << 1
    Call                = (1 << 2) | StartBlock
    Jump                = (1 << 3) | EndBlock

    FormatArgNewLine    = 1 << 4

    @property
    def endBlock(self):
        return bool(self.value & self.EndBlock)

    @property
    def startBlock(self):
        return bool(self.value & self.StartBlock)

    @property
    def call(self):
        return bool(self.value & self.Call & ~self.StartBlock)

    @property
    def jump(self):
        return bool(self.value & self.Jump & ~self.EndBlock)

    @property
    def argNewLine(self):
        return bool(self.value & self.FormatArgNewLine)

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name

class Instruction:
    InvalidOffset   = None

    def __init__(self, opcode: int):
        self.opcode     = None                      # type: int
        self.offset     = self.InvalidOffset        # type: int
        self.size       = 0                         # type: int
        self.operands   = []                        # type: List[Operand]
        self.branches   = []                        # type: List[function.CodeBlock]
        self.descriptor = None                      # type: instruction_table.InstructionDescriptor
        self.label      = None                      # type: Label
        self.flags      = None                      # type: Flags
