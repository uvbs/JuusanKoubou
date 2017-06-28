from Common import *
from Assembler import OperandFormat, OperandDescriptor

UserDefined = OperandFormat.UserDefined

class ED6FCOperandFormat(OperandFormat):
    Offset,     \
    UserDefined = range(UserDefined + 1, UserDefined + 2)

    _sizeTable = {
        Offset  : 2,
    }.update(OperandFormat._sizeTable)

class ED6FCOperandDescriptor(OperandDescriptor):
    formatTable = {
        'o' : OperandDescriptor(OperandFormat.SInt8, hex = False),

    }.update(OperandDescriptor.formatTable)
