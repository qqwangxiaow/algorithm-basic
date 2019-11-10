//考虑内存对齐 并且一个字一个字的复制提高效率 
//android内核版本
typedef long word
#define lsize word
#define lmask (lsize-1)
void *memcpy(void *dest,const void *src,size_t count)
{
    char *d=(char *)dest;
    const char *s=(const char *)src;
    int len;
    if(count==0||dest==src)
        return dest;
    if(((long)d|(long)s)&lmask)
    {
        // src and/or dest do not align on word boundary
        if((((long)d^(long)s)&lmask)||(count<lsize)) //第一个证明 两个指针对边界的差值不一样 所以只能一个个复制 
        {
            len=count;// copy the rest of the buffer with the byte mover
        }
        else
        {
            len=lsize-((long)d&mask);// move the ptrs up to a word boundary
        }
        count-=len;
        for(;len>0;len--)
            *d++=*s++;
    }
    for(len=count/lsize;len>0;len--)
    {
        *(word *)d=*(word *)s;
        d+=lsize;
        s+=lsize;
    }
    for(len=count&lmask;len>0;len--)
    {
        *d++=*s++;
    }
    return dest;
}
