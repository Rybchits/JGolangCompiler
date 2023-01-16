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
        return new String(in.nextLine());
    }

    public static boolean readBool() { return in.nextBoolean(); }

    public static void print(int i) { System.out.print(i); }

    public static void print(float f) { System.out.print(f); }

    public static void print(String s) { System.out.print(s); }

    public static void print(boolean b) { System.out.print(b); }

    public static void print(Object[] array) { System.out.print(Arrays.deepToString(array)); }

    public static void print(int[] array) { System.out.print(Arrays.toString(array)); }

    public static void print(float[] array) { System.out.print(Arrays.toString(array)); }

    public static void print(boolean[] array) { System.out.print(Arrays.toString(array)); }

    public static void println(int i) { System.out.println(i); }

    public static void println(float f) { System.out.println(f); }

    public static void println(String s) { System.out.println(s); }

    public static void println(boolean b) { System.out.println(b); }

    public static void println(Object[] array) { System.out.println(Arrays.deepToString(array)); }

    public static void println(int[] array) { System.out.println(Arrays.toString(array)); }

    public static void println(float[] array) { System.out.println(Arrays.toString(array)); }

    public static void println(boolean[] array) { System.out.println(Arrays.toString(array)); }

    public static int len(Object[] array) { return array.length; }

    public static int len(String string) { return string.length(); }

    public static int[] append(int[] array, int element) {
        int[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static float[] append(float[] array, float element) {
        float[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static String[] append(String[] array, String element) {
        String[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static boolean[] append(boolean[] array, boolean element) {
        boolean[] newArray = Arrays.copyOf(array, array.length+1);
        newArray[array.length] = element;
        return newArray;
    }

    public static int compare(String lhs, String rhs) {
        int lhsLen = lhs.length();
        int rhsLen = rhs.length();

        if (lhsLen < rhsLen) return -1;
        else if (lhsLen > rhsLen) return 1;

        for (int i = 0; i < lhsLen; ++i) {
            if (lhs.charAt(i) < rhs.charAt(i)) return -1;
            else if (lhs.charAt(i) > rhs.charAt(i)) return 1;
        }

        return 0;
    }
}
