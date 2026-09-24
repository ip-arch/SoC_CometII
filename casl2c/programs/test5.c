var max = 5002;
var x, y;

main()
{
  var i=0;

  for (i=0; i<max; i++)  {
    while (1)  {
      for ( ; i<max; i++)  {
        if (i % 10 == 0)
          break;
      }
      if (i == max) break;
      for ( ; i<max; i++)
        if (i % 127 == 0) break;
      x = (y = i) + 5;
      if (i == max) break;
    }
    if (i < 3) break;
  }
}
