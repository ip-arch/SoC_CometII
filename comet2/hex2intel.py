import sys

argv = sys.argv
argc = len(argv)

if argc < 2:
	print("Usage hex2intel.py  input.hex [output.intel]")
try:
	infile = open(argv[1])
except:
	print("File "+argv[1]+" cannot read")
	quit()

if len(argv) > 2:
	wf = argv[2]
else:
	wf = argv[1].strip(".hex")+".intel"
try:
	ofile = open(wf, 'w')
except:
	print("File "+wf+" cannot open")
	quit()

lines = infile.read().split()
count = 0
for line in lines:
	data = int(line,16)
	ofile.write(":02%04X00%04X%02X" %(count, data, ( (-(2 + (count>>8) + ((count) & 0xff) + (data&0xff) + (data>>8)))&0xff))+'\n')
	count = count + 1

infile.close()
ofile.write(":00000001FF")
ofile.close()

