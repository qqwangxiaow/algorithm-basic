int KMP(string s,string p)
{
    int i=0,j=0;
    vector<int>next=getnext(p);
    while (i<s.size()&&j<(int)p.size())
    {
        if(j==-1||s[i]==p[j])
        {
            i++;
            j++;
        }
        else
        {
            j=next[j];
        }
    }
    if(j==p.size())
    {
        return i-j;
    }
    else
    {
        return -1;
    }
}
vector<int>getnext(string p)
{
    int i=0,j=-1;
    vector<int>next(p.size(),0);
    next[0]=-1;
    while (i<p.size())
    {
        if(j==-1||p[i]==p[j])
        {
            i++;
            j++;
            next[i]=j;
        }
        else
        {
            j=next[j];
        }
        
    }
    return next;

}
vector<int>getnextnext(string p)
{
    vector<int>next(p.size());
    next[0]=-1;
    int i=0,j=-1;
    while(i<p.size()-1)
    {
        if(j==-1||p[i]==p[j])
        {
            if(p[++i]==p[++j])
            {
                next[i]=next[j];
            }
            else
            {
                next[i]=j;
            }
        }
        else
        {
            j=next[j];
        }

    }
    return next;

}