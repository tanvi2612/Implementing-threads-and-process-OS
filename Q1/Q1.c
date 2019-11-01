#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}
int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1);     // Index of smaller element

    for (int j = low; j < high; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void insertionSort(int arr[], int k, int n)
{
    int key, j;
    for (int i = k; i <= n; i++)
    {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void quicksort(int arr[], int i, int j)
{
    // printf("halo\n");

    if (i < j)
    {
        int l = j - i + 1;
        if (l <= 5)
        {
            insertionSort(arr, i, j);
            return;
        }
        int part = partition(arr, i, j);
        pid_t lpid, rpid;
        lpid = fork();
        if (lpid < 0)
        {
            // Lchild proc not created
            perror("Left Child Proc. not created\n");
            _exit(-1);
        }
        else if (lpid == 0)
        {
            quicksort(arr, i, i + part - 1);
            _exit(0);
        }
        else
        {
            rpid = fork();
            if (rpid < 0)
            {
                // Rchild proc not created
                perror("Right Child Proc. not created\n");
                _exit(-1);
            }
            else if (rpid == 0)
            {
                quicksort(arr, i + part, j);
                _exit(0);
            }
        }

        int status;

        // Wait for child processes to finish
        waitpid(lpid, &status, 0);
        waitpid(rpid, &status, 0);
    }
}

int main()
{
    int shmid;
    key_t key = IPC_PRIVATE;
    int *share_arr;
    int input_arr[100];
    int len;

    //take the length of array
    printf("Enter the number of elements: ");
    scanf("%d", &len);
    size_t sh_sz = sizeof(int) * len;

    //take input from user
    printf("Enter the elements: ");
    int temp;
    for (int i = 0; i < len; i++)
    {
        scanf("%d", &temp);
        input_arr[i] = temp;
    }

    //create a new seg
    if ((shmid = shmget(key, sh_sz, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        _exit(1);
    }

    // Now we attach the segment to our data space.
    if ((share_arr = shmat(shmid, NULL, 0)) == (int *)-1)
    {
        perror("shmat");
        _exit(1);
    }

    //copy elements to this new seg
    for (int i = 0; i < len; i++)
    {
        share_arr[i] = input_arr[i];
    }
    
    quicksort(share_arr, 0, len - 1);
    for (int i = 0; i < len; i++)
    {
        printf("%d ", share_arr[i]);
    }
    printf("\n");
}