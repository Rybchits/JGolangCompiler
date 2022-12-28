import java.util.Arrays;
import java.util.Scanner;

public class BuiltIn {
    static Scanner in = new Scanner(System.in);

    static public int readInt() {
        return in.nextInt();
    }

    public float readFloat() {
        return in.nextFloat();
    }

    public String readString() {
        return new String(in.next());
    }

    public boolean readBool() {
        return in.nextBoolean();
    }

    static void print(Object o) {
        System.out.print(o);
    }

    static void println(Object o) {
        System.out.println(o);
    }

    static int len(Object[] array) {
        return array.length;
    }

    static Object[] append(Object[] array, Object element) {
        Object[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }
}
