var ___buffer___[257], ___len___;

input(str)
{
  var i;
  in ___buffer___ ,___len___;
  for (i=0; i<___len___; i++)
    *(str+i) = *(___buffer___ + i);
  *(str+___len___) = 0;
}

print(str)
{
  ___len___ = 0;
  while (*(str+___len___))  {
    ___buffer___[___len___] = *(str+___len___);
    ++___len___;
  }
  out ___buffer___, ___len___;
}
