from Common     import *
from Assembler  import *
from enum       import IntEnum

UserDefined = OperandType.UserDefined

class ED6FCOperandType(IntEnum):
    Offset,     \
    Item,       \
    BGM,        \
    Expression, \
    UserDefined = range(5)

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

