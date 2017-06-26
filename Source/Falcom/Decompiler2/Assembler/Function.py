from Common import *
from .Instruction import *

class CodeBlock:
    def __init__(self, instructions):
        self.instructions = instructions or []
        self.labels = None

class Function:
    def __init__(self, name: str, blocks: List[CodeBlock]):
        self.blocks = blocks or []
        self.name = None
