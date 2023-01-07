import java.util.Arrays;
import java.util.Scanner;

public class $Base {
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

    public static boolean readBool() { return in.nextBoolean(); }

    public static void printInt(int i) { System.out.print(i); }

    public static void printFloat(float f) { System.out.print(f); }

    public static void printString(String s) { System.out.print(s); }

    public static void printBoolean(boolean b) { System.out.print(b); }

    public static void printRune(int r) { System.out.print((char)r); }

    public static int lenArray(Object[] array) { return array.length; }

    public static int lenString(String string) { return string.length(); }

    public static int[] appendIntArray(int[] array, int element) {
        int[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static float[] appendFloatArray(float[] array, float element) {
        float[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static String[] appendStringArray(String[] array, String element) {
        String[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static boolean[] appendBooleanArray(boolean[] array, boolean element) {
        boolean[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }
}
