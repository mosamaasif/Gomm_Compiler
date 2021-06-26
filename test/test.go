/*Output will be:
  0
  1
  2
  3
  4
  4
  5
  6
  7
  8*/
func Integer: numPrint (Integer: num, Integer: length)
{
    Integer: i, j, k;
    In >> i;

    j := 0;
    while j < i:
    {
        k := i / 2;
        if j < k:{
            println(j);
        }

        else {
            k := j - 1;
            println(k);
        }
        j := j + 1;
    }
}