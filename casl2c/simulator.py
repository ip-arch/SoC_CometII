#!/usr/bin/python3
# CASL-II simulator for casl2c compiler

from collections import deque
import re
import sys

MinSignedVal = -32768
MaxSignedVal = 32767
MaxUnsignedVal = 65535
MinRegister = 0
MaxRegister = 15
MinIndexRegister = 1
MaxIndexRegister = 15
DataAreaSize = 65536
DataSection = 4096
RegNum = 16
SP = 15

UncertainLabel = -1
FirstOpIndex = 2

code = []
data = [0] * DataAreaSize
GR = [0] * RegNum
QF = 1
OF = 0
SF = 0
ZF = 0
PR = 0

HALT = False
STEP = False
MSIZE = 0
BINARY = False

codeAddress = 2
dataAddress = 0
lineNum = 1
labelTable = { 'main' : [UncertainLabel, [(0,0)]] }

def parseRRA(cmd,ops,exec):
  global codeAddress
  if len(ops) < 2:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 3:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  reg1 = getRegister(ops[0])
  if isRegister(ops[1]):
    reg2 = getRegister(ops[1])
    if len(ops) == 3:
      print('too many operands of {0} at {1}'.format(cmd,lineNum))
      sys.exit(1)
    code.append([exec, 'RR', reg1, reg2])
  else:
    addr = getAddress(ops[1],1)
    if len(ops) == 3:
      x = getIndexRegister(ops[2])
      code.append([exec, 'RAX', reg1, addr, x])
    else:
      code.append([exec, 'RA', reg1, addr])
  codeAddress += 1

def parseRA(cmd,ops,exec):
  global codeAddress
  if len(ops) < 2:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 3:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  reg1 = getRegister(ops[0])
  addr = getAddress(ops[1],1)
  if len(ops) == 3:
    x = getIndexRegister(ops[2])
    code.append([exec, 'RAX', reg1, addr, x])
  else:
    code.append([exec, 'RA', reg1, addr])
  codeAddress += 1

def parseA(cmd,ops,exec):
  global codeAddress
  if len(ops) < 1:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 2:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  addr = getAddress(ops[0],0)
  if len(ops) == 2:
    x = getIndexRegister(ops[1])
    code.append([exec, 'AX', addr, x])
  else:
    code.append([exec, 'A', addr])
  codeAddress += 1

def parseR(cmd,ops,exec):
  global codeAddress
  if len(ops) < 1:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 1:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  reg1 = getRegister(ops[0])
  code.append([exec, 'R', reg1])
  codeAddress += 1

def parseN(cmd,ops,exec):
  global codeAddress
  if len(ops) != 0:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  code.append([exec, 'N'])
  codeAddress += 1

def parseLL(cmd,ops,exec):
  global codeAddress
  if len(ops) < 2:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 2:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  lab1 = getLabel(ops[0],0)
  lab2 = getLabel(ops[0],1)
  code.append([exec, 'AA', lab1, lab2])
  codeAddress += 1

def parseSTART(cmd,ops,exec):
  if len(ops) < 1:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 1:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if ops[0] != 'main':
    print('start label is {0} at {1}'.format(ops[0],lineNum))
    sys.exit(1)

def parseEND(cmd,ops,exec):
  if len(ops) != 0:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)

def parseDS(cmd,ops,exec):
  global dataAddress
  if len(ops) < 1:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 1:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  m = re.match(r'^(\d+)$', ops[0])
  if m is None:
    print('{0} is not a non-negative integer at {1}'.format(ops[0],lineNum))
    sys.exit(1)
  size = int(m.group(1))
  if dataAddress + size >= DataAreaSize:
    print('{0} exceeds the size of available memory at {1}'.format(size,lineNum))
    sys.exit(1)
  dataAddress += size

def parseDC(cmd,ops,exec):
  if len(ops) == 0:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  for item in ops:
    storeConstant(item)

def parseDREG(cmd,ops,exec):
  global codeAddress
  if len(ops) != 0:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  code.append([exec, 'N'])
  codeAddress += 1

def parseDVAR(cmd,ops,exec):
  global codeAddress
  if len(ops) == 0:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 2:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  lab = getLabel(ops[0],1)
  if (len(ops) == 1):
    inst = [exec, 'A', ops[0], lab]
  else:
    m = re.match(r'^(\d+)$', ops[1])
    if m is None:
      print('{0} is not a non-negative integer at {1}'.format(ops[1],lineNum))
      sys.exit(1)
    inst = [exec, 'AI', ops[0], lab, int(m.group(1))]
  code.append(inst)
  codeAddress += 1

def parseDMEM(cmd,ops,exec):
  global codeAddress
  if len(ops) < 2:
    print('too few operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) > 2:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  lab1 = getLabel(ops[0],0)
  lab2 = getLabel(ops[1],1)
  code.append([exec, ops, lab1, lab2])
  codeAddress += 1

def parseDSTACK(cmd,ops,exec):
  global codeAddress
  if len(ops) > 1:
    print('too many operands of {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  if len(ops) == 1:
    m = re.match(r'^(\d+)$', ops[0])
    if m is None:
      print('{0} is not a non-negative integer at {1}'.format(ops[0],lineNum))
      sys.exit(1)
    size = int(m.group(1))
    code.append([exec, 'I', size])
  else:
    code.append([exec, 'N'])
  codeAddress += 1


def storeConstant(item):
  global dataAddress, data
  m = re.match(r'^-?\d+$', item)
  if m is not None:
    data[dataAddress] = int(item)
    dataAddress += 1
  else:
    m = re.match(r"^'(.*)'$", item)
    if m is not None:
      str = m.group(1)
      for c in str:
        data[dataAddress] = ord(c)
        dataAddress += 1
    else:
      print('illegal constant {0} at {1}'.format(item,lineNum))
      sys.exit(1)

def getRegister(op):
  m = re.match(r'^GR(\d+)$', op)
  if m is None:
    print('{0} is not register at {1}'.format(op,lineNum))
    sys.exit(1)
  reg = int(m.group(1))
  if reg > MaxRegister:
    print('{0} is iilegal register at {1}'.format(op,lineNum))
    sys.exit(1)
  else:
    return reg

def getIndexRegister(op):
  m = re.match(r'^GR(\d+)$', op)
  if m is None:
    print('{0} is not register at {1}'.format(op,lineNum))
    sys.exit(1)
  reg = int(m.group(1))
  if reg > MaxIndexRegister or reg < MinIndexRegister:
    print('{0} is iilegal index register at {1}'.format(op,lineNum))
    sys.exit(1)
  else:
    return reg

def getAddress(op,index):
  if isLabel(op):
    return getLabel(op,index)
  else:
    m = re.match(r'^-?\d+$', op)
    if m is not None:
      return int(op)
    else:
      print('lllegal address {0} at {1}'.format(op,lineNum))
      sys.exit(1)

def getLabel(op, index):
  if not isLabel(op):
    print('illegal Label {0} at {1}'.format(op,lineNum))
    sys.exit(1)
  if op in labelTable:
    addr = labelTable[op][0]
    if addr == UncertainLabel:
      labelTable[op][1].append((codeAddress,index))
    return addr
  else:
    labelTable[op] = [UncertainLabel,[(codeAddress,index)]]
    return UncertainLabel
  
def isRegister(op):
  m = re.match(r'^GR(\d+)$', op)
  if m is None:
    return False
  reg = int(m.group(1))
  if reg > MaxRegister:
    return False
  else:
    return True

def parseCASL2(filename):
  global lineNum
  labels = []
  file = open(filename)
  for line in file:
    m = re.match(r'^(\S*)(?:\s+([A-Z]+)(?:\s+(.+))?)?\n$', line)
    items = m.groups()
    if items[0] == '' and items[1] is None:
      print('Empty line at {0}'.format(lineNum))
      sys.exit(1)
    if items[0] != '':
      labels.append(items[0])
    if items[1] is not None:
      if items[2] is None:
        ops = []
      else:
        ops = items[2].split(',')
      parseInst(items[1],ops,labels)
      labels = []
      lineNum += 1
  file.close()

def isLabel(label):
  return (re.match(r'^@?[A-Za-z_][A-Za-z_0-9]*$', label) is not None)

def saveLabel(label,addr):
  if not isLabel(label):
    print('Illegal label {0} at {1}'.format(label,lineNum))
    sys.exit(1)
  if label in labelTable:
    if labelTable[label][0] != UncertainLabel:
      print('Duplicated label {0} at {1}'.format(label,lineNum))
      sys.exit(1)
    labelTable[label][0] = addr
    for (codeAddr,index) in labelTable[label][1]:
      code[codeAddr][index+FirstOpIndex] = addr
  else:
    labelTable[label] = [addr, []]

def parseInst(cmd,ops,labels):
  if labels != []:
    if cmd in dataInsts:
      for lab in labels:
        saveLabel(lab,dataAddress)
    else:
      for lab in labels:
        saveLabel(lab,codeAddress)
  if cmd not in parseInstTable:
    print('illegal instruction {0} at {1}'.format(cmd,lineNum))
    sys.exit(1)
  inst = parseInstTable[cmd]
  inst[0](cmd,ops,inst[1])


def toUnsignedWord(x):
  return (x & 0xffff)

def toSignedInt(x):
  v = x if x <= MaxSignedVal else x - (MaxUnsignedVal + 1)
  return v

def ladd(x,y):
  return toUnsignedWord(toUnsignedWord(x) + toUnsignedWord(y))

def getEA(ops):
  if len(ops) == 1:
    return toUnsignedWord(ops[0])
  else:
    return ladd(ops[0],GR[ops[1]])


def execLD(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  GR[dest] = v
  SF = 1 if v < 0  else 0
  ZF = 1 if v == 0 else 0
  OF = 0
  PR += 1

def execST(type,ops):
  global PR
  addr = getEA(ops[1:])
  data[addr] = GR[ops[0]]
  PR += 1

def execLAD(type,ops):
  global GR, PR
  v = getEA(ops[1:])
  if ops[0] == SP:
    GR[ops[0]] = v
  else:
    GR[ops[0]] = toSignedInt(v)
  PR += 1

def execADDA(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = toSignedInt(GR[dest]) + toSignedInt(v2)
  OF = 1 if val < MinSignedVal or val > MaxSignedVal else 0
  SF = 1 if val & 0x8000 != 0                        else 0
  ZF = 1 if val == 0                                 else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execADDL(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v1 = toUnsignedWord(GR[dest])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = v1 + toUnsignedWord(v2)
  OF = 1 if val > MaxUnsignedVal else 0
  SF = 1 if val & 0x8000 != 0    else 0
  ZF = 1 if val == 0             else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execSUBA(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = toSignedInt(GR[dest]) - toSignedInt(v2)
  OF = 1 if val < MinSignedVal or val > MaxSignedVal else 0
  SF = 1 if val & 0x8000 != 0                        else 0
  ZF = 1 if val == 0                                 else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execSUBL(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v1 = toUnsignedWord(GR[dest])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = v1 - toUnsignedWord(v2)
  OF = 1 if val < 0            else 0
  SF = 1 if val & 0x8000 != 0  else 0
  ZF = 1 if val == 0           else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execAND(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v1 = toUnsignedWord(GR[dest])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = v1 & toUnsignedWord(v2)
  OF = 0
  SF = 1 if val & 0x8000 != 0  else 0
  ZF = 1 if val == 0           else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execOR(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v1 = toUnsignedWord(GR[dest])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = v1 | toUnsignedWord(v2)
  OF = 0
  SF = 1 if val & 0x8000 != 0  else 0
  ZF = 1 if val == 0           else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execXOR(type,ops):
  global GR, SF, ZF, OF, PR
  dest = ops[0]
  v1 = toUnsignedWord(GR[dest])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  val = v1 ^ toUnsignedWord(v2)
  OF = 0
  SF = 1 if val & 0x8000 != 0  else 0
  ZF = 1 if val == 0           else 0
  GR[dest] = toSignedInt(toUnsignedWord(val))
  PR += 1

def execCPA(type,ops):
  global PR, OF, SF, ZF
  v1 = toSignedInt(GR[ops[0]])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  diff = v1 - toSignedInt(v2)
  OF = 0
  if diff > 0:
    SF = ZF = 0
  elif diff == 0:
    SF = 0
    ZF = 1
  else:
    SF = 1
    ZF = 0
  PR += 1

def execCPL(type,ops):
  global PR, OF, SF, ZF
  v1 = toUnsignedWord(GR[ops[0]])
  v2 = GR[ops[1]] if type == 'RR' else data[getEA(ops[1:])]
  diff  = v1 - toUnsignedWord(v2)
  OF = 0
  if diff > 0:
    SF = ZF = 0
  elif diff == 0:
    SF = 0
    ZF = 1
  else:
    SF = 1
    ZF = 0
  PR += 1

def execSLA(type,ops):
  global GR, SF, ZF, OF, PR
  shift = toSignedInt(getEA(ops[1:]))
  if shift < 0:
    print('SLA: negative shift: {0} at {1}'.format(shift))
    sys.exit(1)
  sign = GR[ops[0]] & 0x8000
  mask = 0 if shift >= 15 else (1 << (15 - shift)) - 1
  OF = 0 if shift > 15 or shift == 0 \
         else (GR[ops[0]] & (1 << (15-shift))) >> (15-shift)
  GR[ops[0]] = toSignedInt(sign | ((GR[ops[0]] & mask) << shift))
  SF = 1 if sign != 0 else 0
  ZF = 1 if GR[ops[0]] == 0 else 0
  PR += 1

def execSRA(type,ops):
  global GR, SF, ZF, OF, PR
  shift = toSignedInt(getEA(ops[1:]))
  if shift < 0:
    print('SRA: negative shift: {0} at {1}'.format(shift))
    sys.exit(1)
  sign = GR[ops[0]] & 0x8000
  if shift == 0:
    OF = 0
  elif shift > 15:
    OF = 1 if sign != 0 else 0
  else:
    OF = 1 if (GR[ops[0]] & (1 << (shift-1))) != 0 else 0
  mask = 0 if shift >= 15 else 0x7fff ^ ((1 << shift) - 1)
  if sign != 0:
    padding = 0x7fff if shift >= 15 else 0x7fff ^ ((1 << (15-shift)) - 1)
  else:
    padding = 0
  GR[ops[0]] = toSignedInt(sign | padding | ((GR[ops[0]] & mask) >> shift))
  SF = 1 if sign != 0 else 0
  ZF = 1 if GR[ops[0]] == 0 else 0
  PR += 1

def execSLL(type,ops):
  global GR, SF, ZF, OF, PR
  shift = toSignedInt(getEA(ops[1:]))
  if shift < 0:
    print('SLL: negative shift: {0} at {1}'.format(shift))
    sys.exit(1)
  mask = 0 if shift >= 16 else (1 << (16-shift)) - 1
  OF = 0 if shift > 16 or shift == 0 \
         else (GR[ops[0]] & (1 << (16-shift))) >> (16-shift)
  GR[ops[0]] = toSignedInt((GR[ops[0]] & mask) << shift)
  SF = 1 if GR[ops[0]] & 0x8000 != 0 else 0
  ZF = 1 if GR[ops[0]] == 0          else 0
  PR += 1

def execSRL(type,ops):
  global GR, SF, ZF, OF, PR
  shift = toSignedInt(getEA(ops[1:]))
  if shift < 0:
    print('SLL: negative shift: {0} at {1}'.format(shift))
    sys.exit(1)
  OF = 0 if shift > 16 or shift == 0 \
         else (GR[ops[0]] & (1 << (shift-1))) >> (shift-1)
  mask = 0 if shift >= 16 else 0xffff ^ ((1 << shift) - 1)
  GR[ops[0]] = toSignedInt((GR[ops[0]] & mask) >> shift)
  SF = 1 if GR[ops[0]] & 0x8000 != 0 else 0
  ZF = 1 if GR[ops[0]] == 0          else 0
  PR += 1

def execJPL(type,ops):
  global PR
  addr = getEA(ops)
  PR = addr if ZF == 0 and SF == 0 else PR + 1

def execJMI(type,ops):
  global PR
  addr = getEA(ops)
  PR = addr if SF == 1 else PR + 1

def execJNZ(type,ops):
  global PR
  addr = getEA(ops)
  PR = addr if ZF == 0 else PR + 1

def execJZE(type,ops):
  global PR
  addr = getEA(ops)
  PR = addr if ZF == 1 else PR + 1

def execJOV(type,ops):
  global PR
  addr = getEA(ops)
  PR = addr if OF == 1 else PR + 1

def execJUMP(type,ops):
  global PR
  PR = getEA(ops)

def execPUSH(type,ops):
  global GR, PR
  GR[SP] = toUnsignedWord(toUnsignedWord(GR[SP]) - 1)
  data[GR[SP]] = toSignedInt(getEA(ops))
  PR += 1

def execPOP(type,ops):
  global GR, PR
  stackTop = toUnsignedWord(GR[SP])
  GR[ops[0]] = toSignedInt(data[stackTop])
  GR[SP] = stackTop + 1
  PR += 1

def execPUSHF(type,ops):
  global GR, PR, QF, OF, SF, ZF
  GR[SP] = toUnsignedWord(toUnsignedWord(GR[SP]) - 1)
  data[GR[SP]] = ((QF << 3) | (OF << 2) | (SF << 1) | ZF)
  PR += 1

def execPOPF(type,ops):
  global GR, PR, QF, OF, SF, ZF
  stackTop = toUnsignedWord(GR[SP])
  QF = ((data[stackTop] >> 3) & 1)
  OF = ((data[stackTop] >> 2) & 1)
  SF = ((data[stackTop] >> 1) & 1)
  ZF = (data[stackTop] & 1)
  GR[SP] = stackTop + 1
  PR += 1

def execCALL(type,ops):
  global GR, PR
  GR[SP] = toUnsignedWord(toUnsignedWord(GR[SP]) - 1)
  data[GR[SP]] = PR + 1
  if BINARY:
    data[GR[SP]] += 1
  PR = getEA(ops)

def execRET(type,ops):
  global GR, PR
  stackTop = toUnsignedWord(GR[SP])
  PR = toUnsignedWord(data[stackTop])
  GR[SP] = stackTop + 1

def execRETI(type,ops):
  global GR, PR, QF, OF, SF, ZF
  stackTop = toUnsignedWord(GR[SP])
  PR = toUnsignedWord(data[stackTop])
  stackTop += 1
  QF = ((data[stackTop] >> 3) & 1)
  OF = ((data[stackTop] >> 2) & 1)
  SF = ((data[stackTop] >> 1) & 1)
  ZF = (data[stackTop] & 1)
  GR[SP] = stackTop + 1

def execSVC(type,ops):
  global GR, PR
  GR[SP] = toUnsignedWord(toUnsignedWord(GR[SP]) - 1)
  data[GR[SP]] = ((QF << 3) | (OF << 2) | (SF << 1) | ZF)
  GR[SP] -= 1
  data[GR[SP]] = PR + 1
  if BINARY:
    data[GR[SP]] += 1
  addr = getEA(ops)
  PR = data[GR[14] + addr]

def execNOP(type,ops):
  global PR
  PR += 1

def execIN(type,ops):
  global PR
  print('IN is not supported')
  PR += 1

def execOUT(type,ops):
  global PR
  print('OUT is not supported')
  PR += 1

def execHALT(type,ops):
  global HALT
  HALT = True

def execDREG(type,ops):
  global PR
  showRegisters()
  PR += 1

def execDVAR(type,ops):
  global PR
  print('{0}: '.format(ops[0]), end="")
  if (type == 'A'):
    print('{0}'.format(data[ops[1]]))
  else:
    Row = 8
    i = 1
    for (addr) in range(ops[1], ops[1]+ops[2]):
      if i % Row == 0 or addr == ops[1] + ops[2] - 1:
        print('{0:6}'.format(data[addr]))
      else:
        print('{0:6}  '.format(data[addr]), end="")
      i += 1
  PR += 1

def execDMEM(labels,ops):
  global PR
  if ops[0] > ops[1]:
    print('DMEM: the start address is greatear than the end address')
    sys.exit(1)
  if labels != []:
    print('{0} - {1}'.format(labels[0],labels[1]))
  printMemory(ops[0],ops[1])
  PR += 1

def printMemory(s,e):
  Row = 8
  i = 1
  for addr in range(s, e+1):
    if i % Row == 0 or addr == e:
      print('{0:6}'.format(data[addr]))
    else:
      print('{0:6}  '.format(data[addr]), end="")
    i += 1

def execDSTACK(type,ops):
  global PR
  i = 1
  Row = 8
  stackTop = toUnsignedWord(GR[SP])
  if type == 'N':
    end = DataAreaSize
  else:
    end = min(stackTop + ops[0], DataAreaSize)
  print('Stack')
  for addr in range(stackTop, end):
    if i % Row == 1:
      print('{0:3}: {1:6}'.format(i-1,data[addr]) ,end="")
    else:
      print(' {0:6}'.format(data[addr]), end="")
    if i % Row == 0 or addr == end - 1:
      print('')
    i += 1
  PR += 1


def showRegisters():
  for i in range(0, RegNum, 4):
    for j in range(i, i+3):
      print('GR{0:02}: {1:6}  '.format(j,GR[j]), end="")
    print('GR{0:02}: {1:6}'.format(i+3,GR[i+3]))
  print('PR: {0}  SP: {1} FR: ({2},{3},{4})'.format(hex(PR),GR[SP],OF,SF,ZF))

def run():
  global GR
  GR[SP] = DataAreaSize
  while not HALT:
    if 0 > PR or PR >= len(code):
      print('illegal PR: {0}'.format(PR))
      sys.exit(1)
    if STEP:
      showRegisters()
    inst = code[PR]
    inst[0](inst[1],inst[2:])
  if STEP:
    showRegisters()

parseInstTable = {
    'LD'     : (parseRRA,  execLD),
    'ST'     : (parseRA,   execST),
    'LAD'    : (parseRA,   execLAD),
    'ADDA'   : (parseRRA,  execADDA),
    'ADDL'   : (parseRRA,  execADDL),
    'SUBA'   : (parseRRA,  execSUBA),
    'SUBL'   : (parseRRA,  execSUBL),
    'AND'    : (parseRRA,  execAND),
    'OR'     : (parseRRA,  execOR),
    'XOR'    : (parseRRA,  execXOR),
    'CPA'    : (parseRRA,  execCPA),
    'CPL'    : (parseRRA,  execCPL),
    'SLA'    : (parseRA,   execSLA),
    'SRA'    : (parseRA,   execSRA),
    'SLL'    : (parseRA,   execSLL),
    'SRL'    : (parseRA,   execSRL),
    'JPL'    : (parseA,    execJPL),
    'JMI'    : (parseA,    execJMI),
    'JNZ'    : (parseA,    execJNZ),
    'JZE'    : (parseA,    execJZE),
    'JOV'    : (parseA,    execJOV),
    'JUMP'   : (parseA,    execJUMP),
    'PUSH'   : (parseA,    execPUSH),
    'POP'    : (parseR,    execPOP),
    'PUSHF'  : (parseN,    execPUSHF),
    'POPF'   : (parseN,    execPOPF),
    'CALL'   : (parseA,    execCALL),
    'RET'    : (parseN,    execRET),
    'SVC'    : (parseA,    execSVC),
    'NOP'    : (parseN,    execNOP),
    'RETI'   : (parseN,    execRETI),
    'IN'     : (parseLL,   execIN),
    'OUT'    : (parseLL,   execOUT),
    'HLT'    : (parseN,    execHALT),
    'START'  : (parseSTART,0),
    'END'    : (parseEND,  0),
    'DS'     : (parseDS,   0),
    'DC'     : (parseDC,   0),
    'DREG'   : (parseDREG, execDREG),
    'DVAR'   : (parseDVAR, execDVAR),
    'DMEM'   : (parseDMEM, execDMEM),
    'DSTACK' : (parseDSTACK, execDSTACK)
  }

dataInsts = [ 'DC', 'DS' ]


def decodeN(r, x, exec, cmd):
  if r != 0:
    print('{0}: illegal register number {1}'.format(cmd,r))
    sys.exit(1)
  if x != 0:
    print('{0}: illegal index register number {1}'.format(cmd,x))
    sys.exit(1)
  if STEP:
    print('{0}\t({1})'.format(cmd,hex(PR)))
  exec('N',[])

def decodeRAX(r, x, exec, cmd):
  global PR
  adr = data[PR+1]
  if x == 0:
    if STEP:
      print('{0}\tGR{1},{2}\t({3})'.format(cmd,r,hex(adr),hex(PR)))
    exec('RA',[r,adr])
  else:
    if STEP:
      print('{0}\tGR{1},{2},GR{3}\t({4})'.format(cmd,r,hex(adr),x,hex(PR)))
    exec('RAX',[r,adr,x])
  # ２語長の命令なので、PR をさらに１つ進める
  PR += 1

def decodeRR(r1, r2, exec, cmd):
  if STEP:
    print('{0}\tGR{1},GR{2}\t({3})'.format(cmd,r1,r2,hex(PR)))
  exec('RR',[r1, r2])

def decodeAX(r, x, exec, cmd):
  global PR
  if r != 0:
    print('{0}: illegal register number {1}'.format(cmd,r))
  oldPR = PR
  adr = data[PR+1]
  if x == 0:
    if STEP:
      print('{0}\t{1}\t({2})'.format(cmd,hex(adr),hex(PR)))
    exec('A',[adr])
  else:
    if STEP:
      print('{0}\t{1},GR{2}\t({3})'.format(cmd,hex(adr),x,hex(PR)))
    exec('AX',[adr,x])
  # ジャンプしなかったなら、２語長の命令なのでPRをさらに１つ進める
  if PR == oldPR + 1:
    PR += 1

def decodeR(r, x, exec, cmd):
  if x != 0:
    print('{0}: illegal index register number {1}'.format(cmd,x))
  if STEP:
    print('{0}\tGR{1}\t({2})'.format(cmd,r,hex(PR)))
  exec('R', [r])


def decodeAA(r, x, exec, cmd):
  global PR
  data[PR+1] = adr1
  data[PR+2] = adr2
  exec([],[adr1,adr2])
  PR += 2

def simulateBin(filename):
  global data, PR
  i = 0
  file = open(filename)
  for line in file:
    data[i] = int(line.rstrip(),16)
    i += 1
  file.close()
  PR = 0
  while not HALT:
    opcode = data[PR] >> 8
    r = (data[PR] & 0xf0) >> 4
    x = data[PR] & 0x0f
    exec = decodeTable[opcode]
    if STEP:
      showRegisters()
    exec[0](r,x,exec[1],exec[2])
#  while PR < data[1]:
#    opcode = data[PR] >> 8
#    r = (data[PR] & 0xf0) >> 4
#    x = data[PR] & 0x0f
#    exec = decodeTable[opcode]
#    exec[0](r,x,exec[1],exec[2])
#    PR += 1
  print('registers')
  showRegisters()
  if MSIZE != 0:
    print('data: {0}-{1}'.format(DataSection,DataSection+MSIZE-1))
    printMemory(DataSection,DataSection+MSIZE-1)



decodeTable = {
    0x00     : (decodeN,    execNOP,    'NOP'),
    0x10     : (decodeRAX,  execLD,     'LD'),
    0x11     : (decodeRAX,  execST,     'ST'),
    0x12     : (decodeRAX,  execLAD,    'LAD'),
    0x14     : (decodeRR,   execLD,     'LD'),
    0x20     : (decodeRAX,  execADDA,   'ADDA'),
    0x21     : (decodeRAX,  execSUBA,   'SUBA'),
    0x22     : (decodeRAX,  execADDL,   'ADDL'),
    0x23     : (decodeRAX,  execSUBL,   'SUBL'),
    0x24     : (decodeRR,   execADDA,   'ADDA'),
    0x25     : (decodeRR,   execSUBA,   'SUBA'),
    0x26     : (decodeRR,   execADDL,   'ADDL'),
    0x27     : (decodeRR,   execSUBL,   'SUBL'),
    0x30     : (decodeRAX,  execAND,    'AND'),
    0x31     : (decodeRAX,  execOR,     'OR'),
    0x32     : (decodeRAX,  execXOR,    'XOR'),
    0x34     : (decodeRR,   execAND,    'AND'),
    0x35     : (decodeRR,   execOR,     'OR'),
    0x36     : (decodeRR,   execXOR,    'XOR'),
    0x40     : (decodeRAX,  execCPA,    'CPA'),
    0x41     : (decodeRAX,  execCPL,    'CPL'),
    0x44     : (decodeRR,   execCPA,    'CPA'),
    0x45     : (decodeRR,   execCPL,    'CPL'),
    0x50     : (decodeRAX,  execSLA,    'SLA'),
    0x51     : (decodeRAX,  execSRA,    'SRA'),
    0x52     : (decodeRAX,  execSLL,    'SLL'),
    0x53     : (decodeRAX,  execSRL,    'SRL'),
    0x61     : (decodeAX,   execJMI,    'JMI'),
    0x62     : (decodeAX,   execJNZ,    'JNZ'),
    0x63     : (decodeAX,   execJZE,    'JZE'),
    0x64     : (decodeAX,   execJUMP,   'JUMP'),
    0x65     : (decodeAX,   execJPL,    'JPL'),
    0x66     : (decodeAX,   execJOV,    'JOV'),
    0x70     : (decodeAX,   execPUSH,   'PUSH'),
    0x71     : (decodeR,    execPOP,    'POP'),
    0x72     : (decodeN,    execPUSHF,  'PUSHF'),
    0x73     : (decodeN,    execPOPF,   'POPF'),
    0x80     : (decodeAX,   execCALL,   'CALL'),
    0x81     : (decodeN,    execRET,    'RET'),
    0xF0     : (decodeAX,   execSVC,    'SVC'),
    0xF1     : (decodeN,    execRETI,    'RETI'),
    0xF2     : (decodeN,    execHALT,   'HLT'),
    0xA0     : (decodeN,    execDREG,   'DREG'),
    0xA1     : (decodeAX,   execDVAR,   'DVAR'),
    0xA2     : (decodeAA,   execDMEM,   'DMEM'),
    0xA3     : (decodeAX,   execDSTACK, 'DSTACK')
  }


def main():
  global code, STEP, MSIZE, BINARY
  args = deque(sys.argv)
  args.popleft()		# コマンド名を捨てる
  if len(args) == 0:
    print('too few arguments')
    sys.exit(1)
  # オプション処理
  item = args.popleft()
  while item[0] == '-':
    if item[1] == 's':
      STEP = True
    if item[1] == 'm':
      m = re.match(r'^-m(\d+)$', item)
      MSIZE = int(m.group(1))
    if args == deque([]):
      print('too few arguments')
      sys.exit(1)
    item = args.popleft()
  # ファイルの拡張子に応じてシミュレーションを実行
  filename = item
  m = re.match(r'^.*\.([^.]+)$', filename);
  if m is None:
    print("illegal file type")
    sys.exit(1)
  ext = m.group(1)
  if ext == 'cas':
    # アセンブリコードのシミュレーション
    code.extend([[execCALL,'A','main'],[execHALT,'N']])
    parseCASL2(filename)
#   i = 0
#   for inst in code:
#     print('{0}: {1}'.format(i,inst))
#     i += 1
#   i = 0
#   for word in data:
#     print('{0}: {1}'.format(i,word))
#     i += 1
    run()
  elif ext == 'hex':
    # バイナリコードのシミュレーション
    BINARY = True
    simulateBin(filename)
  else:
    print('illegal file extension')


if __name__ == '__main__':
  main()
