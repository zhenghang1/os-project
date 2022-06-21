#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void sort(void *arg);
void merge();
int *arr;
int *result;

int main()
{
    arr = malloc(sizeof(int) * 10);
    int len = 0;
    int maxlen = 10;
    printf("please input the array to be sorted:\n");
    while (scanf("%d", &arr[len]))
    {
        len++;
        if (len == maxlen)
        {
            int *tmp = realloc(arr, sizeof(int) * maxlen * 2);
            arr = tmp;
        }
    }

    int arg1[2] = {0, len / 2};
    int arg2[2] = {len / 2 + 1, len - 1};

    pthread_t sort_thread1, sort_thread2, merge_thread;
    pthread_attr_t sort_thread1_attr, sort_thread2_attr, merge_thread_attr;
    pthread_attr_init(&sort_thread1_attr);
    pthread_attr_init(&sort_thread2_attr);
    pthread_attr_init(&merge_thread_attr);

    int ret = pthread_create(&sort_thread1, &sort_thread1_attr, sort, arg1);
    if (ret == -1)
    {
        printf("Create sort_thread1 error!\n");
        return 1;
    }
    ret = pthread_create(&sort_thread2, &sort_thread2_attr, sort, arg2);
    if (ret == -1)
    {
        printf("Create sort_thread2 error!\n");
        return 1;
    }

    pthread_join(sort_thread1, NULL); //阻塞主线程，直到子线程结束才恢复执行
    pthread_join(sort_thread2, NULL);

    result = malloc(sizeof(int) * len);
    ret = pthread_create(&merge_thread, &merge_thread_attr, merge, &len);
    if (ret == -1)
    {
        printf("Create merge_thread error!\n");
        return 1;
    }

    pthread_join(merge_thread, NULL);

    for (int i = 0; i < len; i++)
    {
        printf("%d ", result[i]);
    }
    printf("\n");

    free(arr);
    free(result);
    return 0;
}

//快速排序(o(NlogN))    不稳定排序
int divide(int a[], int low, int high)
{
    int k = a[low];
    do
    {
        while (low < high && a[high] >= k)
            --high;
        if (low < high)
        {
            a[low] = a[high];
            ++low;
        }
        while (low < high && a[low] <= k)
            ++low;
        if (low < high)
        {
            a[high] = a[low];
            --high;
        }
    } while (low != high);
    a[low] = k;
    return low;
}

void quickSort(int a[], int low, int high)
{
    int mid;

    if (low >= high)
        return;
    mid = divide(a, low, high);
    quickSort(a, low, mid - 1);  //排序左一半
    quickSort(a, mid + 1, high); //排序右一半
}

// 包裹函数
void sort(void *arg)
{
    int begin = ((int *)arg)[0];
    int end = ((int *)arg)[1];
    quickSort(arr, begin, end);
}

void merge(void *arg)
{
    int len = *((int *)arg);
    int p1 = 0, p2 = len / 2 + 1;
    int count = 0;
    while (p1 <= len / 2 && p2 < len)
    {
        if (arr[p1] < arr[p2])
            result[count] = arr[p1++];
        else
            result[count] = arr[p2++];
        count++;
    }
    if (p2 == len && p1 <= len / 2)
        while (count < len)
            result[count++] = arr[p1++];

    else if (p2 < len && p1 > len / 2)
        while (count < len)
            result[count++] = arr[p2++];
}