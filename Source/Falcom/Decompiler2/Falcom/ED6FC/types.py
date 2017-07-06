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
        **OperandFormat.sizeTable,

        ED6FCOperandType.Offset     : 2,
        ED6FCOperandType.Item       : 2,
        ED6FCOperandType.BGM        : 2,
        ED6FCOperandType.Expression : None,
    }

class ED6FCOperandDescriptor(OperandDescriptor):
    def formatValue(self, info: 'FormatOperandHandlerInfo') -> str:
        return super().formatValue(info)

    def readValue(self, fs: fileio.FileStream) -> Any:
        if self.format.type != OperandType.MBCS:
            return super().readValue(fs)

        return self.readText(fs)

    def readText(self, fs: fileio.FileStream) -> 'List[TextObject]':
        s = bytearray()
        objs = []                   # type: List[TextObject]
        containsCtrlCodes = False

        def flushText():
            nonlocal s

            if len(s) != 0:
                objs.append(TextObject(value = s.decode(self.format.encoding)))
                s = bytearray()

        while True:
            c = fs.read(1)
            if len(c) == 0:
                break

            c = c[0]

            if c == 0:
                flushText()
                break

            if c >= 0x20:
                s.append(c)
                if c >= 0x80:
                    s.append(fs.read(1)[0])

                continue

            flushText()

            containsCtrlCodes = True

            o = TextObject(code = c)

            if c in [
                    TextCtrlCode.NewLine,
                    TextCtrlCode.NewLine2,
                    TextCtrlCode.WaitForEnter,
                    TextCtrlCode.Clear,
                    TextCtrlCode.Clear2,
                    0x05,
                    TextCtrlCode.ShowAll,
                    0x18,
                ]:
                pass

            elif c == TextCtrlCode.SetColor:
                o.value = fs.read(1)[0]

            elif c == TextCtrlCode.Item:
                o.value = fs.ReadUShort()

            objs.append(o)

        if containsCtrlCodes is False and len(objs) == 1:
            return objs[0].value

        return objs

def oprdesc(*args, **kwargs):
    return ED6FCOperandDescriptor(ED6FCOperandFormat(*args, **kwargs))

ED6FCOperandDescriptor.formatTable = {
    **OperandDescriptor.formatTable,

    'o' : oprdesc(ED6FCOperandType.Offset),
    'S' : oprdesc(OperandType.MBCS),
}

class TextCtrlCode:
    NewLine         = 0x01
    NewLine2        = 0x0A
    WaitForEnter    = 0x02
    Clear           = 0x03
    Clear2          = 0x04
    ShowAll         = 0x06
    SetColor        = 0x07
    Item            = 0x1F

class TextObject:
    def __init__(self, code: int = None, value: Any = None):
        self.code   = code              # type: int
        self.value  = value             # type: Any

    def __str__(self):
        if self.code is None:
            return "'%s'" % self.value

        if self.value is not None:
            return 'code = %02X, value = %s' % (self.code, self.value)

        return 'code = %02X' % self.code

    def __repr__(self):
        return self.__str__()

class Expression:
    pass
