#include<iostream>
#include<vector>
using namespace std;
void adjust(vector<int>&arr,int len,int index)
{
    int left=2*index+1;
    int right=2*index+2;
    int maxindex=index;
    if(left<len&&arr[left]>arr[maxindex])  //maxindex not index 
        maxindex=left;
    if(right<len&&arr[right]>arr[maxindex])
        maxindex=right;
    if(maxindex!=index)
    {
        swap(arr[maxindex],arr[index]);  
        adjust(arr,len,maxindex); //the 3rd variable=maxindex
    } 
}

void heapsort(vector<int>&arr,int size)
{
    for(int i=size/2-1;i>=0;i--) //i=size/2-1
    {
        adjust(arr,size,i);
    }
    for(int i=size-1;i>=1;i--)  //i>=1
    {
        swap(arr[i],arr[0]);
        adjust(arr,i,0);
    }
}

int main()
{
    vector<int>arr{2,1,9,-1,6,0,22};
    heapsort(arr,arr.size());
    for(int i=0;i<arr.size();i++)
    {
        cout<<" "<<arr[i];
    }

}