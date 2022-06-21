import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Quicksort extends RecursiveAction {
    static final int THRESHOLD = 10;

    private int begin;
    private int end;
    private Integer[] array;

    public Quicksort(int begin, int end, Integer[] array) {
        this.begin = begin;
        this.end = end;
        this.array = array;
    }

    protected void compute() {
        if (end - begin < THRESHOLD) {
            // conquer stage
            // using seletion sorting
            this.selection_sort(begin, end);

        } else {
            // divide stage
            // quicksort,pick array[begin] as a pivot
            int pivot = array[begin];
            int low=begin,high=end;
            do {
                while (low < high && array[high] >= pivot)
                    --high;
                if (low < high) {
                    array[low] = array[high];
                    ++low;
                }
                while (low < high && array[low] <= pivot)
                    ++low;
                if (low < high) {
                    array[high] = array[low];
                    --high;
                }
            } while (low != high);
            array[low] = pivot;

            Quicksort leftTask = new Quicksort(begin, low-1, array);
            Quicksort rightTask = new Quicksort(low + 1, end, array);

            leftTask.fork();
            rightTask.fork();

            rightTask.join();
            leftTask.join();

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

        Quicksort task = new Quicksort(0, len - 1, array);
        ForkJoinPool pool = new ForkJoinPool();
        pool.invoke(task);

        System.out.print("The result array:\n");
        System.out.println(Arrays.toString(array));

        sc.close();
    }

    protected void selection_sort(int begin, int end) {
        int min, tmp;
        for (int i = begin; i <= end; i++) {
            min = i;
            for (int j = i + 1; j <= end; ++j)
                if (array[j] < array[min])
                    min = j;
            tmp = array[i];
            array[i] = array[min];
            array[min] = tmp;
        }
    }
}

