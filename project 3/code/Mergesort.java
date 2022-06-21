import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Mergesort extends RecursiveAction {
    static final int THRESHOLD = 10;

    private int begin;
    private int end;
    private Integer[] array;

    public Mergesort(int begin, int end, Integer[] array) {
        this.begin = begin;
        this.end = end;
        this.array = array;
    }

    protected void compute() {
        if (end - begin < THRESHOLD) {
            // conquer stage
            // using insertion sorting
            this.insertion_sort(begin, end);

        } else {
            // divide stage
            int mid = begin + (end - begin) / 2;

            Mergesort leftTask = new Mergesort(begin, mid, array);
            Mergesort rightTask = new Mergesort(mid + 1, end, array);

            leftTask.fork();
            rightTask.fork();

            rightTask.join();
            leftTask.join();

            // merge
            Integer[] tmparr = new Integer[end - begin + 1];
            int p1 = begin, p2 = mid + 1, count = 0;
            while (p1 <= mid && p2 <= end) {
                if (array[p1] < array[p2])
                    tmparr[count] = array[p1++];
                else
                    tmparr[count] = array[p2++];
                count++;
            }
            if (p1 > mid && p2 <= end) {
                while (p2 <= end)
                    tmparr[count++] = array[p2++];

            } else if (p1 <= mid && p2 > end) {
                while (p1 <= mid)
                    tmparr[count++] = array[p1++];
            }
            for (int i = begin; i <= end; i++)
                array[i] = tmparr[i - begin];
        }
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.print("Please input the length of the array to be sorted:\n");
        int len = sc.nextInt();
        Integer[] array = new Integer[len];

        System.out.print("mode1: randomly generate an array of integers automatically\n");
        System.out.print("mode2: input your own array\n");
        System.out.print("please select the mode:(1/2)\n");
        int flag = sc.nextInt();
        if (flag == 1) {
            System.out.print(
                    "please input the high bound of the randomized integers:(the low bound is 0)\n");
            int bound = sc.nextInt() + 1;
            java.util.Random rand = new java.util.Random();
            for (int i = 0; i < len; i++) {
                array[i] = rand.nextInt(bound);
            }
        } else if (flag == 2) {
            System.out.print("please input the array:\n");
            for (int i = 0; i < len; i++) {
                array[i] = sc.nextInt();
            }
        } else {
            System.out.print("Error:invalid input,exit\n");
            System.exit(1);
        }

        Mergesort task = new Mergesort(0, len - 1, array);
        ForkJoinPool pool = new ForkJoinPool();
        pool.invoke(task);

        System.out.print("The result array:\n");
        System.out.println(Arrays.toString(array));

        sc.close();
    }

    protected void insertion_sort(int begin, int end) {
        for (int i = begin + 1; i <= end; i++) {
            int key = array[i];
            int j = i - 1;
            while ((j >= begin) && (key < array[j])) {
                array[j + 1] = array[j];
                j--;
            }
            array[j + 1] = key;
        }
    }
}

