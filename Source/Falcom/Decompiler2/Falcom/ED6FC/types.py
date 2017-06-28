from Common import *
from enum import IntEnum
from Assembler import OperandType, OperandFormat, OperandDescriptor

UserDefined = OperandType.UserDefined

class ED6FCOperandType(IntEnum):
    Offset = UserDefined + 1

    __str__     = OperandType.__str__
    __repr__    = OperandType.__repr__

class ED6FCOperandFormat(OperandFormat):
    sizeTable = {
        ED6FCOperandType.Offset : 2,

        **OperandFormat.sizeTable,
    }

class ED6FCOperandDescriptor(OperandDescriptor):
    pass

def oprdesc(*args, **kwargs):
    return ED6FCOperandDescriptor(ED6FCOperandFormat(*args, **kwargs))

ED6FCOperandDescriptor.formatTable = {
    'o' : oprdesc(ED6FCOperandType.Offset),

    **OperandDescriptor.formatTable,
}
