var i, j, k, l;

main()
{
  var x;
  for (i=0,(j=i,k=i-1, l=i+j); i<10; i++,(j++,k++), l++)
    x = (i++, (j=k-1, k++), l=x-1);
}
