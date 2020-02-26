
class string
{
friend std::ostream& operator<<(osstrem &os,const string &str);
friend std::istream& operator>>(isstream &is,string &str);
public:
    string(const char*=nullptr);
    ~string(void);
    string(const string &str);
    string &operator=(const string& str);
    string operator+(const string& str);
    bool operator==(const string& str);
    char& operator[](unsigned int e);
    int getlength();

private:
    char *m_data;
}

string::string(const char *str)
{
    if(str==nullptr)
    {
        m_data=new char[1];
        *m_data='\0';
    }
    else
    {
        m_data=new char[strlen(str)+1];
        strcpy(m_data,str);
    }
}

string::~string()
{
    if(m_data)
    {
        delete[] m_data;
        m_data=nullptr;
    }
}

string::string(const string& str)
{
    if(str.m_data==nullptr)
    {
        m_data=nullptr;
    }
    else
    {
        m_data=new char[strlen(str.m_data)+1];
        strcpy(m_data,str.m_data);

    }
}

string &string::operator=(const string&str)
{
    if(this!=str)
    {
        delete[] m_data;
        m_data=nullptr;
        if(str.m_data==nullptr)
            m_data=nullptr;
        else
        {
            m_data=new char[strlen(str.m_data)+1];
            strcpy(m_data,str.m_data);
        }
    }
    return *this;
}

string string::operator+(const string &str)
{
    string newstring;
    if(str==nullptr)
        return *this;
    else if(m_data==nullptr)
    {
        newstring=str;
    }
    else
    {
        newstring.m_data=new char[strlen(str.m_data)+1+strlen(m_data)+1];
        strcpy(newstring.m_data,m_data);
        strcpy(newstring,str.m_data);
    }
    return newstring;  
}

bool string::operator==(const srtring &str)
{
    if(strlen(m_data)!=strlen(str.m_data))
        return false;
    else
    {
        return strcmp(m_data,str.m_data)?false:true;
    }
}

char &string::operator[](unsigned int e)
{
    if(e>=0&&e<=strlen(m_data))
        return m_data[e];
}

int string::getlength()
{
    return strlen(m_data);
}
ostream &operator<<(ostream &os,const string &str)
{
    os<<str.m_data;
    return os;
}

istream &operator>>(istream &is,string &str)
{
    char buff[4096];
    is>>buff;
    str.m_data=buff;
    return is;
}
