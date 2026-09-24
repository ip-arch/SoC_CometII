#include "itable.c"
#include "io.c"

var msg = "Hello !!";
var len, x;

proc strlen/1;

main()
{
  setvec();
  print(msg);
  len = strlen(msg);
}

proc strlen(str)
{
  var len = 0;
  while (*(str+len++)) ;
  return x = len;
}
