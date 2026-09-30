void insertionSort(int arr[], int n) {
    int i, j, key;

    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

//bubble sort 

void bubbleSort(int arr[], int n) {
    int i, j, temp;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Quick sort
void quickSort(int arr[], int n) {
    int i, j, pivot, temp;

    if (n <= 1)
        return;

    pivot = arr[n - 1];
    i = -1;

    for (j = 0; j < n - 1; j++) {
        if (arr[j] < pivot) {
            i++;

            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    temp = arr[i + 1];
    arr[i + 1] = arr[n - 1];
    arr[n - 1] = temp;

    quickSort(arr, i + 1);
    quickSort(arr + i + 2, n - i - 2);
} 

//heap_sort

void heapify(int arr[] , int s ,int i){
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

void swap(int *a, int *b){
    int t=*a;
    *a=*b;
    *b=t;
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


// merge sort

void merge(int arr[], int l, int r , int m){ 
    //divide the arr into subarrays

    // m for midle m=l+(r-l)/2
    int i,j,k;
    int n1=m-l+1;// first subarray 
    int n2=r-m;//second subarray
    
    int *L=malloc(n1*sizeof(int));
    int *R=malloc(n2*sizeof(int));

    for (i=0;i<n1;i++) L[i]=arr[l+i];

    for (j=0;j<n2;j++) R[i]=arr[r+j];

    i=j=0;
    k=l;

    //compaire

    while (i<n1 && j<n2){
        if (L[i]<R[j]){
            arr[k]=L[i];
            i++;
        }
        else{
            arr[k]=R[j];
            j++;
        }
        k++;
    }
    while(i<n1){
        arr[k]=L[i];
        i++;k++;
    }
    while(j<n1){
        arr[k]=R[j];
        j++;k++;
    }


}

void mergeSort(int arr[],int l, int r){
    if (l<r){
        int m=l+(r-l)/2;
        mergesort(arr,l,m);
        mergesort(arr,m+1,r);
        merge(arr,l,r,m);
    }
}