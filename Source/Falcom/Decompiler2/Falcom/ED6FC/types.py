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
        return {
            OperandType.MBCS        : self.readText,
            ED6FCOperandType.Offset : lambda fs: fs.ReadUShort(),
            ED6FCOperandType.Item   : lambda fs: fs.ReadUShort(),
            ED6FCOperandType.BGM    : lambda fs: fs.ReadShort(),

        }.get(self.format.type, super().readValue)(fs)

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

            try:
                code = TextCtrlCode(c)
            except ValueError:
                code = c

            o = TextObject(code = code)

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
                o.code = TextCtrlCode(o.code)
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

class TextCtrlCode(IntEnum):
    NewLine         = 0x01
    NewLine2        = 0x0A
    WaitForEnter    = 0x02
    Clear           = 0x03
    Clear2          = 0x04
    ShowAll         = 0x06
    SetColor        = 0x07
    Item            = 0x1F

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.__str__()

class TextObject:
    def __init__(self, code: int = None, value: Any = None):
        self.code   = code              # type: int
        self.value  = value             # type: Any

    def __str__(self):
        if self.code is None:
            return "'%s'" % self.value

        if self.value is not None:
            return 'code = %s, value = %s' % (self.code, self.value)

        return 'code = %s' % self.code

    def __repr__(self):
        return self.__str__()

class ScenaExpression:
    class Operator(IntEnum):
        Push                = 0x00
        End                 = 0x01
        Equ                 = 0x02
        Neq                 = 0x03
        Lss                 = 0x04
        Gtr                 = 0x05
        Leq                 = 0x06
        Ge                  = 0x07
        Ez                  = 0x08
        Nez64               = 0x09
        And                 = 0x0A
        Or                  = 0x0B
        Add                 = 0x0C
        Sub                 = 0x0D
        Neg                 = 0x0E
        Xor                 = 0x0F
        IMul                = 0x10
        IDiv                = 0x11
        IMod                = 0x12
        Stub                = 0x13
        IMul2               = 0x14
        IDiv2               = 0x15
        IMod2               = 0x16
        Add2                = 0x17
        Sub2                = 0x18
        And2                = 0x19
        Xor2                = 0x1A
        Or2                 = 0x1B
        Exec                = 0x1C
        Not                 = 0x1D
        TestScenaFlags      = 0x1E
        GetResult           = 0x1F
        PushValueIndex      = 0x20
        GetChrWork          = 0x21
        Rand                = 0x22

    @classmethod
    def readExpressions(cls, fs: fileio.FileStream) -> 'List[Expression]':
        Operator = cls.Operator

        exps = []

        while True:
            e = cls(operator = fs.ReadByte())
            e.readOperand(fs)

            exps.append(e)

            if e.operator == Operator.End:
                break

        return exps

    def __init__(self, operator: Operator, *operand: Tuple[int]):
        self.operator   = operator          # type: Operator
        self.operand    = operand and None  # type: Tuple[int]

    def readOperand(self, fs: fileio.FileStream) -> Tuple[int]:
        Operator = self.Operator

        self.operand = {
            Operator.Push   : lambda: fs.ReadULong(),

        }[self.operator]()

        return self.operand
