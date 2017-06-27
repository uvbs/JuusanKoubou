from Common import *

__all__ = (
    'InstructionDescriptor',
    'InstructionTable',
)

class InstructionDescriptor:
    def __init__(self):
        self.fuck = 0

class InstructionTable:
    def __init__(self, decriptors: List[InstructionDescriptor]):
        self.decriptors = []        # type: List[InstructionDescriptor]

    def readInstruction(self, fs: fileio.FileStream) -> InstructionDescriptor:
        raise NotImplementedError
