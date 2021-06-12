func Integer: numPrint (Integer: num, Integer: length)
{
    Integer: i, j, first, temp;
    char : a;
    a := 'x';
    print ( "enter number" );
    In>> i;
    println (i);
    i := length;
    while i > 0:
    {
        first:=0;
        j := 1;
        while j < i:
        {
            write( j);
            j:=j + 1 * 3 / 4;
        }
        if j = 1:{
            print("one");
        }
        elif j=2:{
            print("two");
            if j = 2:{
                print("two1");
            }
            elif j=3: {
                print("How even?!");
            }
            else {
                print("no two1");
            }
        }
        elif j=3:{
            print("three");
        }
        else
        {
            print("others");
            }

        /* this is a comment */
        i:=i - 1;
        /*This is a
                Multiline
                Comment*/
        }
        print( "temp is");
        println(temp);
        ret i;
}