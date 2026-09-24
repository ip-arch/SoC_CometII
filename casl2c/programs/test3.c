var digits[6], work[6];
var x = 31572;

main()
{
  var i=0, n;
  while (x != 0)  {
    work[i++] = x % 10 + '0';
    x = x / 10;
  }
  work[i] = 0;
  n = i;
  for (i=0; i<n; i++)
    digits[n-i-1] = work[i];
  digits[n] = 0;
}
