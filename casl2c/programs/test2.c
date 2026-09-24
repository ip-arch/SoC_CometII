var str = "abc\n\"d'ef";

proc foo(index)
{
  if (index >= 0 && index <= 2)
    *(str+index) = 'd';
}

main()
{
  foo(1);
}
