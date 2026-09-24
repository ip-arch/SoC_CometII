import sys

argv = sys.argv
argc = len(argv)
insts = {}
labels = {}
consttbl = {}
mem = {}
constnum = 0
regs = {
	"GR0":0,
	"GR1":1,
	"GR2":2,
	"GR3":3,
	"GR4":4,
	"GR5":5,
	"GR6":6,
	"GR7":7,
	"GR8":8,
	"GR9":9,
	"GR10":10,
	"GR11":11,
	"GR12":12,
	"GR13":13,
	"GR14":14,
	"GR15":15
	}
current = 0

def numparse(word):
	if word.isdigit():
		return int(word)
	elif word[0]=='#':
		return int(word[1:],16)

def pStart(line):
	global current
	if len(line) > 1:
		insts[current] = ["JUMP",line[1]]
		current = current +2

def pInOut(line):
	global current
	if len(line) != 3:
		print("Number of arguments for "+line+" is not match")
	else:
		insts[current] = ["PUSH","0","GR1"]
		insts[current+2] = ["PUSH","0","GR2"]
		insts[current+4] = ["LAD","GR1",line[1]]
		insts[current+6] = ["LAD","GR2",line[2]]
		if line[0] == "IN":
			insts[current+8] = ["SVC", "1"]
		else:
			insts[current+8] = ["SVC", "2"]
		insts[current+10] = ["POP","GR2"]
		insts[current+11] = ["POP","GR1"]
		current = current + 12

def pRpush(line):
	global current
	for i in [1,2,3,4,5,6,7]:
		insts[current] = ["PUSH", "0","GR"+repr(i)]
		current = current +2

def pRpop(line):
	global current
	for i in [7,6,5,4,3,2,1]:
		insts[current] = ["POP", "GR"+repr(i)]
		current = current +1

def pEnd(line):
	return

def pDs(line):
	global current
	if len(line) < 2:
		print("The number of argument is too small for DS")
		quit()
	length = numparse(line[1])
	for i in range(length):
		insts[current] = ["DS", "1"]
		current = current +1

def pDc(line):
	global current
	if len(line) < 2:
		print("The number of argument is too small for DC")
		quit()
	for i in line[1:]:
		insts[current] = ["DC", i]
		current = current +1

def pNop(line):
	global current
	insts[current] = [line[0]]
	current = current +1

def pNorm(line):
	global current, constnum
	if len(line) < 2:
		print("The number of argument is too small for Instruction ", line[0])
		quit()
	for i in range(1,len(line)):
		if line[i][0]=="=":
			if line[i][1:] in consttbl:
				line[i]=consttbl[line[i][1:]]
			else:
				lab = "LCONST"+repr(constnum);
				lines.append(" ".join([lab, "DC", line[i][1:]]))
				consttbl[line[i][1:]] = lab
				line[i]=lab
				constnum = constnum + 1
	if len(line) == 2 and opcode[line[0]][1]=="R":
		insts[current] = line
		current = current + 1
	elif opcode[line[0]][1]=="RX" and len(line) == 3 and line[2] in regs:
		insts[current] = [line[0]+"R", line[1], line[2]]
		current = current +1
	else:
		insts[current] = line
		current = current + 2

opc = {
	"START":pStart,
	"END":pEnd,
	"DS":pDs,
	"DC":pDc,
	"IN":pInOut,
	"OUT":pInOut,
	"RPUSH":pRpush,
	"RPOP":pRpop,
	"LD":pNorm,
	"ST":pNorm,
	"LAD":pNorm,
	"ADDA":pNorm,
	"ADDL":pNorm,
	"SUBA":pNorm,
	"SUBL":pNorm,
	"AND":pNorm,
	"OR":pNorm,
	"XOR":pNorm,
	"CPA":pNorm,
	"CPL":pNorm,
	"SLA":pNorm,
	"SRA":pNorm,
	"SLL":pNorm,
	"SRL":pNorm,
	"JPL":pNorm,
	"JMI":pNorm,
	"JNZ":pNorm,
	"JZE":pNorm,
	"JOV":pNorm,
	"JUMP":pNorm,
	"PUSH":pNorm,
	"POP":pNorm,
	"CALL":pNorm,
	"RET":pNop,
	"SVC":pNorm,
	"NOP":pNop,
	"PUSHF":pNop,
	"POPF":pNop,
	"RETI":pNop,
	"HLT":pNop
	}

opcode = {
	"LD":(0x10,"RX"),
	"LDR":(0x14,"RR"),
	"ST":(0x11,"RX"),
	"LAD":(0x12,"RX"),
	"ADDA":(0x20,"RX"),
	"SUBA":(0x21,"RX"),
	"ADDL":(0x22,"RX"),
	"SUBL":(0x23,"RX"),
	"AND":(0x30,"RX"),
	"OR":(0x31,"RX"),
	"XOR":(0x32,"RX"),
	"CPA":(0x40,"RX"),
	"CPL":(0x41,"RX"),
	"ADDAR":(0x24,"RR"),
	"SUBAR":(0x25,"RR"),
	"ADDLR":(0x26,"RR"),
	"SUBLR":(0x27,"RR"),
	"ANDR":(0x34,"RR"),
	"ORR":(0x35,"RR"),
	"XORR":(0x36,"RR"),
	"CPAR":(0x44,"RR"),
	"CPLR":(0x45,"RR"),
	"SLA":(0x50,"RX"),
	"SRA":(0x51,"RX"),
	"SLL":(0x52,"RX"),
	"SRL":(0x53,"RX"),
	"JMI":(0x61,"X"),
	"JNZ":(0x62,"X"),
	"JZE":(0x63,"X"),
	"JUMP":(0x64,"X"),
	"JPL":(0x65,"X"),
	"JOV":(0x66,"X"),
	"PUSH":(0x70,"X"),
	"POP":(0x71,"R"),
	"CALL":(0x80,"X"),
	"RET":(0x81,"N"),
	"SVC":(0xf0,"X"),
	"NOP":(0x00,"N"),
	"PUSHF":(0x72,"N"),
	"POPF":(0x73,"N"),
	"RETI":(0xf1,"N"),
	"HLT":(0xf2,"N")
	}

def parse(line):
	word = line.split()
	if len(word) == 0:
		return
	elif len(word[0]) == 0:
		return
	elif word[0] in opc:
		opc[word[0]](word)
	elif line[0].isalpha() and word[0] not in regs:
		labels[word[0]]=current
		parse(" ".join(word[1:]))
	else:
		print("Syntax error at:"+line)
		quit(1)

def setadr(loc, code):
	if code.isdigit():
		mem[loc] = int(code)
	elif code[0] == '#':
		mem[loc] = int(code[1:], 16)
	elif code in labels:
		mem[loc] = labels[code]
	else:
		print("Address resolution error:"+repr(loc)+code)
		quit(1)

def geninst(inst, code):
	if code[0] == "DS":
		setadr(inst, "0")
		return
	elif code[0] == "DC":
		if len(code) == 2:
			setadr(inst, code[1])
			return
		else:
			print("Number of argument error "+code)
			quit()

	mem[inst] = opcode[code[0]][0] << 8
	ity = opcode[code[0]][1]
	if ity=="N" and len(code)==1:
		return
	elif ity=="R" and len(code)==2:
		if code[1] in regs:
			mem[inst] = mem[inst] | (regs[code[1]]<<4)
	elif ity=="RR" and len(code)==3:
		if code[1] in regs and code[2] in regs:
			mem[inst] = mem[inst] | (regs[code[1]]<<4)
			mem[inst] = mem[inst] | regs[code[2]]
	elif ity=="X" and len(code)>1:
		setadr(inst+1, code[1])
		if len(code)==3 and code[2] in regs:
			mem[inst] = mem[inst] | regs[code[2]]
	elif ity=="RX" and len(code) > 2:
		mem[inst] = mem[inst] | (regs[code[1]]<<4)
		setadr(inst+1, code[2])
		if len(code) == 4 and code[3] in regs:
			mem[inst] = mem[inst] | regs[code[3]]
	else:
		print("operand error "+repr(code))
		quit()
		
		 
if argc < 2:
	print("Usage asm.py  input.cas [output.hex]")
try:
	infile = open(argv[1])
except:
	print("File "+argv[1]+" cannot read")
	quit()
lines = infile.read().split("\n")

for line in lines:
	idx=line.find(';')
	if(idx>=0):
		line=line[0:idx]
	parse(line.upper().replace(',',' '))
	print(line)

for inst in sorted(insts.keys()):
	geninst(inst, insts[inst])

infile.close()

if len(argv) > 2:
	wf = argv[2]
else:
	wf = argv[1].strip(".cas")+".hex"
try:
	ofile = open(wf, 'w')
except:
	print("File "+wf+" cannot open")
	quit()

for data in sorted(mem.keys()):
	ofile.write("%04X" % (mem[data])+'\n')

ofile.close()

