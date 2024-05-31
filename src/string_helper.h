#ifndef _STRING_HELPER_H_
#define _STRING_HELPER_H_
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
using namespace std;

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

static vector<string> explode( const string &str, const string &delimiter, bool trim = false)
{
    vector<string> arr;

    int strleng = str.length();
    int delleng = delimiter.length();
    if (delleng==0)
        return arr;//no change

    int i=0;
    int k=0;
    while( i<strleng )
    {
        int j=0;
        while (i+j<strleng && j<delleng && str[i+j]==delimiter[j])
            j++;
        if(trim && i == k && str[k] == ' ') k++; //trim start

        if (j==delleng)//found delimiter
        {
            if(trim && str[i-1] == ' ') i--; //trim end
            if(i-k > 0) //skip empty
                arr.push_back(  str.substr(k, i-k) );
            i+=delleng;
            k=i;
        }
        else
        {
            i++;
        }
    }
    if(i-k > 0 )
    arr.push_back(  str.substr(k, i-k) );
    return arr;
};

#endif