from Common import *
from . import instruction

__all__ = (
    'CodeBlock',
    'Function',
)

class CodeBlock:
    def __init__(self, instructions: 'List[instruction.Instruction]'):
        self.instructions = []      # type: List[instruction.Instruction]
        self.labels = None          # type: List[instruction.Label]
        self.offset = None          # type: int

class Function:
    def __init__(self, name: str, blocks: List[CodeBlock]):
        self.blocks = blocks or []
        self.name = None
