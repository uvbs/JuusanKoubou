from Common import *
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from . import instruction

__all__ = (
    'CodeBlock',
    'Function',
)

class CodeBlock:
    def __init__(self, instructions: 'List[instruction.Instruction]'):
        self.instructions   = instructions  # type: List[instruction.Instruction]
        self.labels         = None          # type: List[instruction.Label]
        self.offset         = None          # type: int

    def __str__(self):
        return '\n'.join(['%s' % inst for inst in self.instructions])

class Function:
    def __init__(self, name: str = '', offset: int = 0, blocks: List[CodeBlock] = None):
        self.name   = name                  # type: str
        self.offset = offset                # type: int
        self.blocks = blocks                # type: List[CodeBlock]

    def __str__(self):
        return '\n'.join([
            '%s @ 0x%X' % (self.name, self.offset),
            '%s' % self.blocks,
        ])
