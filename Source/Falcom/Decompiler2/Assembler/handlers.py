from Common import *
from enum import IntEnum
from . import instruction

if TYPE_CHECKING:
    from . import instruction_table
    from . import disassembler

__all__ = (
    'InstructionHandler',
    'InstructionHandlerInfo',
    'FormatOperandHandler',
    'FormatOperandHandlerInfo',
)

class BaseHandlerInfo:
    class Action(IntEnum):
        Disassemble = 0
        Assemble    = 1
        Format      = 2

    def __init__(self, action: 'BaseHandlerInfo.Action'):
        self.action = action # type: BaseHandlerInfo.Action

class InstructionHandlerInfo(BaseHandlerInfo):
    def __init__(self, action: 'BaseHandlerInfo.Action', descriptor: 'instruction_table.InstructionDescriptor', disasmInfo: 'disassembler.DisassembleInfo'):
        super().__init__(action)
        self.descriptor = descriptor                                # type: instruction_table.InstructionDescriptor
        self.disasmInfo = disasmInfo                                # type: disassembler.DisassembleInfo
        self.offset     = instruction.Instruction.InvalidOffset     # type: int

InstructionHandler = Callable[[InstructionHandlerInfo], Any]


class FormatOperandHandlerInfo:
    def __init__(self, operand: 'instruction.Operand', labels: 'Dict[int, str]' = None):
        self.operand = operand          # type: instruction.Operand
        self.labels  = labels or {}     # type: Dict[int, str]

FormatOperandHandler = Callable[[FormatOperandHandlerInfo], Any]
