import java.util.Arrays;
import java.util.Scanner;

public class BuiltIn {
    private static final Scanner in = new Scanner(System.in);

    public static int readInt() {
        return in.nextInt();
    }

    public static float readFloat() {
        return in.nextFloat();
    }

    public static String readString() {
        return new String(in.next());
    }

    public static boolean readBool() {
        return in.nextBoolean();
    }

    public static void print(Object o) {
        System.out.print(o);
    }

    public static void println(Object o) {
        System.out.println(o);
    }

    public static int len(Object[] array) {
        return array.length;
    }

    public static Object[] append(Object[] array, Object element) {
        Object[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }
}
