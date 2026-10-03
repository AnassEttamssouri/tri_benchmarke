#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sort.h"




void insertionSort(int *arr, int n)
{
    for (int i = 1; i < n; i++)
    {
        int key = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}


static void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int *arr, int low, int high) {
    int random_pivot = low + rand() % (high - low + 1);
    swap(&arr[random_pivot], &arr[high]);

    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSortRec(int *arr, int low, int high) {
    while (low < high) {
        int pi = partition(arr, low, high);

        if (pi - low < high - pi) {
            quickSortRec(arr, low, pi - 1);
            low = pi + 1; 
        } else {
            quickSortRec(arr, pi + 1, high);
            high = pi - 1; 
        }
    }
}


void quickSort(int *arr, int n) {
    quickSortRec(arr, 0, n - 1);
}

//well be used only for the best case expirement .

static int partitionMid(int *arr, int low, int high) {
    int mid = low + (high - low) / 2;      // deterministic pivot
    swap(&arr[mid], &arr[high]);
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
        if (arr[j] < pivot) { i++; swap(&arr[i], &arr[j]); }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void quickSortMidRec(int *arr, int low, int high) {
    while (low < high) {
        int pi = partitionMid(arr, low, high);
        if (pi - low < high - pi) { quickSortMidRec(arr, low, pi - 1); low = pi + 1; }
        else                      { quickSortMidRec(arr, pi + 1, high); high = pi - 1; }
    }
}


void quickSortMid(int *arr, int n) { quickSortMidRec(arr, 0, n - 1); }


static void merge(
    int *arr,
    int *temp,
    int left,
    int middle,
    int right
)
{
    int i = left;
    int j = middle + 1;
    int k = left;

    while (i <= middle && j <= right)
    {
        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    while (i <= middle)
        temp[k++] = arr[i++];

    while (j <= right)
        temp[k++] = arr[j++];

    for (i = left; i <= right; i++)
        arr[i] = temp[i];
}


static void mergeSortRecursive(
    int *arr,
    int *temp,
    int left,
    int right
)
{
    if (left >= right)
        return;

    int middle = left + (right - left) / 2;

    mergeSortRecursive(arr, temp, left, middle);
    mergeSortRecursive(arr, temp, middle + 1, right);

    merge(arr, temp, left, middle, right);
}


void mergeSort(int *arr, int n)
{
    if (n <= 1)
        return;

    int *temp = malloc(n * sizeof(int));

    if (temp == NULL)
    {
        fprintf(stderr, "Erreur allocation mémoire MergeSort\n");
        return;
    }

    mergeSortRecursive(arr, temp, 0, n - 1);

    free(temp);
}




void selectionSort(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int min_idx = i;
        for (int j = i + 1; j < n; j++)
        {
            if (arr[j] < arr[min_idx])
            {
                min_idx = j;
            }
        }
        if (min_idx != i)
        {
            int temp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = temp;
        }
    }
}



void bubbleSort(int *arr, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}



//heapSort : 
void heapify(int *arr , int s ,int i){
    int l=(i*2) + 1;//left child
    int r=(i*2) + 2;
    int max =i;
    if(arr[l]> arr[max]){
        max=l;
    }
    if(arr[r]>arr[max]){
        max=r;
    }
    if (max!=i){
        swap(arr[i],arr[max]);
        heapify(arr,s,max);
    }

}
void buildHeap(int arr[],int n){
    for (int i=(n/2) -1;i>0;i--){
        heapify(arr,n,i);
    }
}

void heapSort(int arr[], int n){
    buildHeap(arr,n);
    for (int i=n-1;i>=0;i--){
        swap(arr[0],arr[i]);
        heapify(arr,n,0);
    }
} 