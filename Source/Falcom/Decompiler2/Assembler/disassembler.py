from Common             import *
from .instruction       import *
from .instruction_table import *
from .function          import *
from .handlers          import *

__all__ = (
    'Disassembler',
    'DisassembleInfo',
)

class DisassembleInfo:
    def __init__(self, fs: fileio.FileStream):
        self.fs     = fs    # type: fileio.FileStream

class Disassembler:
    def __init__(self, instructionTable: InstructionTable):
        self.instructionTable   = instructionTable      # type: InstructionTable
        self.disassembledBlocks = {}                    # type: Dict[int, CodeBlock]

    def disasmFunction(self, info: DisassembleInfo) -> Function:
        fun = Function()

        fun.offset = info.fs.Position
        fun.block = self.disasmBlock(info)

        return fun

    def disasmBlock(self, info: DisassembleInfo) -> CodeBlock:
        offset = info.fs.Position
        block = self.disassembledBlocks.get(offset)

        if block is not None:
            return block

        block = CodeBlock(instructions = [])
        block.offset = offset

        self.disassembledBlocks[offset] = block

        while True:
            inst = self.disasmInstruction(info)

            block.instructions.append(inst)

            if not inst.flags.startBlock and not inst.flags.endBlock:
                continue

            for index, branch in enumerate(inst.branches):
                pos = info.fs.Position

                info.fs.Position = branch.offset
                inst.branches[index] = self.disasmBlock(info)

                info.fs.Position = pos

            if inst.flags.endBlock:
                break

        return block

    def disasmInstruction(self, info: DisassembleInfo) -> Instruction:
        pos = info.fs.Position

        try:
            opcode = self.instructionTable.readOpCode(info.fs)
        except Exception as e:
            print('error occur %s @ position %X' % (e, pos))
            raise e

        desc = self.instructionTable.getDescriptor(opcode)

        handlerInfo = InstructionHandlerInfo(InstructionHandlerInfo.Action.Disassemble, desc)

        handlerInfo.offset      = pos
        handlerInfo.disasmInfo  = info

        inst = None

        if desc.handler is not None:
            inst = desc.handler(handlerInfo)

        if inst is None:
            inst = self.defaultInstructionParser(handlerInfo)

        if inst is None:
            raise Exception('disasmInstruction %02X @ %08X failed' % (opcode, pos))

        inst.descriptor = inst.descriptor if inst.descriptor is None else desc
        inst.flags = inst.descriptor.flags if inst.flags is None else inst.flags

        return inst

    def defaultInstructionParser(self, info: InstructionHandlerInfo) -> Instruction:
        fs      = info.disasmInfo.fs
        desc    = info.descriptor

        inst = Instruction(desc.opcode)

        inst.offset     = info.offset
        inst.operands   = [self.instructionTable.readOperand(fs, oprdesc) for oprdesc in (desc.operands or [])]
        inst.size       = fs.Position - inst.offset
        inst.descriptor = desc
        inst.flags      = desc.flags

        return inst

    def formatFuncion(self, fun: Function) -> List[str]:
        return self.formatBlock(fun.block)

    def formatBlock(self, block: CodeBlock) -> List[str]:
        text = []

        for inst in block.instructions:
            t = self.formatInstruction(inst)
            if not inst.flags.argNewLine:
                text.append(''.join(t))
                continue

            text.append('')
            text.append(',\n'.join(t))
            text.append('')

        return text

    def formatInstruction(self, inst: Instruction) -> List[str]:
        handler = inst.descriptor.handler
        if handler is not None:
            handlerInfo = InstructionHandlerInfo(InstructionHandlerInfo.Action.Format, inst.descriptor)
            handlerInfo.instruction = inst

            ret = handler(handlerInfo)
            if ret is not None:
                return ret

        mnemonic = inst.descriptor.mnemonic
        operands = self.instructionTable.formatAllOperand(inst.operands)

        return ['%s(%s)' % (mnemonic, ', '.join(operands))]
