//using first element as pivot
void QuickSort(int A[],int left,int right)
{
    int l=left,r=right;
    int pivot;
    if(left<right)
    {
        pivot=A[left];
        while(l<r)
        {
            while(l<r&&A[r]>=pivot)
                r--;
            if(l<r)
            {
                A[l]=A[r];
                l++;
            }
            while(l<r&&A[l]<pivot)
                l++;
            if(l<r)
            {
                A[r]=A[l];
                r--;
            }
        }
        A[l]=pivot;
        QuickSort(A,left,l-1);
        QuickSort(A,l+1,right);
    }
}
//partition
void swap(int* a, int* b) 
{ 
    int t = *a; 
    *a = *b; 
    *b = t; 
} 
int partition(int arr[],int low,int high)
{
    int pivot=arr[high];
    int l=low-1;
    for(int i=low;i<=high-1;i++)
    {
        if(arr[i]<=pivot)
        {
            l++;
            swap(&arr[l],&arr[i]);
        }
    }
    swap(&arr[i+1],&arr[high]);
    return (i+1);
}
void QuickSort(int arr[],int low,int high)
{
    if(low<high)
    {
        int m=partition(arr,low,high);
        QuickSort(arr,low,m-1);
        QuickSort(arr,m+1,high);
    }

}

