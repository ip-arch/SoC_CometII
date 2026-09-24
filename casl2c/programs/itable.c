var light = 0xC01A;
var vga = 0xA000;

interrupt _input_(buffer, len)
{
  var i;
  disableIntr();
  for (i=0; i<len; i++)
    *(buffer+i) = *(vga+i);
  enableIntr();
}

interrupt _print_(buffer, len)
{
  var i;
  disableIntr();
  for (i=0; i<len; i++)
    *(vga+i) = *(buffer+i);
  enableIntr();
}

interrupt _abort_()
{
  disableIntr();
  halt;
}

var vector[] = { _input_, _print_, _abort_ };

proc setvec()
{
  setVector(vector);
}


