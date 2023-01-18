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

    public static int len(int[] array) { return array.length; }

    public static int len(float[] array) { return array.length; }

    public static int len(boolean[] array) { return array.length; }

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
        return lhs.compareTo(rhs);
    }

    public static boolean equals(Object[] lhs, Object[] rhs) {
        return Arrays.deepEquals(lhs, rhs);
    }

    public static boolean equals(int[] lhs, int[] rhs) {
        return Arrays.equals(lhs, rhs);
    }

    public static boolean equals(float[] lhs, float[] rhs) {
        return Arrays.equals(lhs, rhs);
    }

    public static boolean equals(boolean[] lhs, boolean[] rhs) {
        return Arrays.equals(lhs, rhs);
    }
}
