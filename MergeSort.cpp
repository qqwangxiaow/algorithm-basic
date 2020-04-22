#include<iostream>
#include<vector>
#include<unordered_map>
#include<string>
#include<bitset>
#include<stack>
#include<algorithm>
#include<set>
#include<unordered_set>
#include<map>
using namespace std;
void merge(vector<int>&arr,int l,int m,int r,vector<int>temp)
{
    int i=l,j=m+1,k=0;
    while(i<=m&&j<=r)
    {
        if(arr[i]<arr[j])
        {
            temp[k++]=arr[i++];
        }
        else
        {
            temp[k++]=arr[j++];
        }
    }
    while(i<=m)
        temp[k++]=arr[i++];
    while(j<=r)
        temp[k++]=arr[j++];
    k=0;
    while(l<=r)
    {
        arr[l++]=temp[k++];
    }
}
void mergesort(vector<int>&arr,int l,int r,vector<int>temp)
{
    if(l<r)
    {
        int m=(l+r)>>1;
        mergesort(arr,l,m,temp);
        mergesort(arr,m+1,r,temp);
        merge(arr,l,m,r,temp);
    }

}
void MergeSort(vector<int>&arr)
{
    vector<int>temp(arr.size());
    mergesort(arr,0,arr.size()-1,temp);
}

int main() {
    vector<int>arr{2,1,9,-1,6,0,22};
    MergeSort(arr);
    for(int i=0;i<arr.size();i++)
        cout<<arr[i]<<" ";

}