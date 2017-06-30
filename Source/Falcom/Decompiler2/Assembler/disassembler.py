from Common import *
from .instruction       import *
from .instruction_table import *
from .function          import *

__all__ = (
    'Disassembler',
)

class DisassembleInfo:
    def __init__(self, fs: fileio.FileStream):
        self.fs     = fs    # type: fileio.FileStream
        self.offset = 0     # type: int

class Disassembler:
    def __init__(self, instructionTable: InstructionTable = None):
        self.instructionTable   = instructionTable      # type: InstructionTable
        self.disassembledBlocks = []                    # type: Dict[int, CodeBlock]

    def disasmFunction(self, info: DisassembleInfo) -> Function:
        fun = Function()

        fun.offset = info.fs.Position
        fun.blocks = self.disasmBlock(info)

        return fun

    def disasmBlock(self, info: DisassembleInfo) -> List[CodeBlock]:
        block = CodeBlock([])
        block.offset = info.fs.Position

    def disasmInstruction(self, info: DisassembleInfo) -> Instruction:
        pos = info.fs.Position

        opcode  = self.instructionTable.readOpCode(info.fs)
        desc    = self.instructionTable.fromOpCode(opcode)

        handlerInfo = InstructionHandlerInfo(InstructionHandlerInfo.Action.Disassemble, desc, info)
        handlerInfo.offset = pos

        inst = (desc.handler or self.defaultInstructionParser)(handlerInfo)

        if inst is None:
            raise Exception('disasmInstruction %02X @ %08X failed' % (opcode, pos))

        return inst

    def defaultInstructionParser(self, info: InstructionHandlerInfo) -> Instruction:
        fs   = info.disasmInfo.fs
        desc = info.descriptor

        inst = Instruction(desc.opcode)

        inst.offset   = info.offset
        inst.operands = [oprdesc.read(fs) for oprdesc in desc.operands]

        return inst
