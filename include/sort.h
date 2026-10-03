#ifndef SORT_H
#define SORT_H

typedef void (*sort_func)(int *arr, int n);

void insertionSort(int *arr, int n);
void selectionSort(int *arr, int n);
void bubbleSort(int *arr, int n);
void quickSort(int *arr, int n);
void quickSortMid(int *arr, int n);
void mergeSort(int *arr, int n);


#endif