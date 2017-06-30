from ml import *
import Assembler

from Falcom import ED6FC

def test():
    print(ED6FC.ScenaOpTable)
    print()

    dis = Assembler.Disassembler(ED6FC.ScenaOpTable)

    fs = fileio.FileStream()
    fs.Open('tests\\T2610_1 ._SN')

    fs.Position = 0x64

    info = Assembler.DisassembleInfo(fs)

    f = dis.disasmFunction(info)

    f.name = 'test'

    print(f)

def main():
    test()

    console.pause('done')

if __name__ == '__main__':
    Try(main)
