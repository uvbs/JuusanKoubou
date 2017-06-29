from Common import *
from .instruction import *
from .instruction_table import *
from .function import *

__all__ = (
    'Disassembler',
)

class Disassembler:
    def __init__(self, instructionTable: InstructionTable = None):
        self.instructionTable = instructionTable        # type: InstructionTable

    def disasmFunction(self):
        pass
