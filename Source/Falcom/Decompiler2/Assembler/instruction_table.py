from Common import *
from . import instruction

__all__ = (
    'InstructionDescriptor',
    'InstructionTable',
)

class InstructionDescriptor:
    def __init__(self):
        self.fuck = 0

class InstructionTable:
    def __init__(self, decriptors: List[InstructionDescriptor]):
        self.decriptors = decriptors or []        # type: List[InstructionDescriptor]

    def readInstruction(self, fs: fileio.FileStream) -> 'instruction.Instruction':
        raise NotImplementedError
