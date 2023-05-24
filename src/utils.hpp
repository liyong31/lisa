#pragma once

#include <memory>
#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
#include <locale>
#include "myassert.hpp"


// TODO: use Boost

namespace sdf
{


inline
std::string readfile(const std::string& file_name)
{
    //https://stackoverflow.com/a/116220/801203
    std::fstream in(file_name);
    std::stringstream s;
    s << in.rdbuf();
    return s.str();
}


inline
std::vector<std::string> split_by_space(const std::string& s)
{
    std::istringstream iss(s);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

template<typename TC>
inline
std::string join(const std::string& sep, const TC& elements)
{
    std::stringstream ss;
    uint i = 1;
    for (const auto& e: elements)
    {
        ss << e;
        if (i != elements.size())
            ss << sep;
        i++;
    }
    return ss.str();
}


//https://stackoverflow.com/a/3418285/801203
inline
std::string substitute(const std::string& str,
                       const std::string& from, const std::string& to)
{   // substitutes the first occurrence
    std::string result(str);

    std::size_t start_pos = result.find(from);
    if (start_pos == std::string::npos)
        return result;

    result.replace(start_pos, from.length(), to);
    return result;
}


inline
std::string substituteAll(const std::string& str,
                          const std::string& from, const std::string& to)
{
    // https://stackoverflow.com/a/3418285/801203
    std::string result(str);
    MASSERT(!from.empty(), "`from` is empty: '" << from << "', '" << to << "'");

    std::size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos)
    {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return result;
}


inline
std::string trim_spaces(const std::string& arg)
{
    // python's strip()
    std::string result = arg;

    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    result.erase(std::find_if(result.rbegin(), result.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), result.end());

    return result;
}


inline
std::string lower(const std::string& what)
{
    // https://stackoverflow.com/a/313990/801203
    std::string result = what;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}


template<typename ... Args>
inline
std::string string_format(const std::string& format, Args ... args)
{
    // https://stackoverflow.com/a/26221725/801203
    size_t size = (size_t) snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

template<typename T>
inline
bool contains(const std::vector<T>& elements, const T &elem)
{
    return std::find(elements.begin(), elements.end(), elem) != elements.end();
}

template<typename T>
inline
bool contains(const std::set<T>& elements, const T& elem)
{
    return elements.find(elem) != elements.end();
}

/* Execute the command, return <rc, stdout, stderr>. */
std::tuple<int, std::string, std::string> execute(const char *cmd);
std::tuple<int, std::string, std::string> execute(const std::string &cmd);

/**
 * Similar to python's range.
 * @return [min, min+1, ..., max_excluded)
 */
template<typename T>
inline
std::vector<T> range(const T &min, const T &max_excluded)  // T to be able to use with int and uint, etc.
{
    std::vector<T> numbers;
    for (auto i = min; i < max_excluded; ++i)
        numbers.push_back(i);
    return numbers;
}


template<typename E>
E pop(std::set<E>& container)
{
    auto it = container.begin();
    E elem = *it;
    container.erase(it);
    return elem;
}


inline
std::string create_tmp_folder()
{
    std::string path_tmp_str = "/tmp/XXXXXX";
    std::string folder = mkdtemp((char*)path_tmp_str.c_str());  // (char*) is a hack but path_tmp_str lives long enough, so OK
    if (folder.empty())
        throw std::string("failure to create tmp folder");
    return folder;
}


template<typename E>
std::set<E>
a_minus_b(const std::set<E>& a, const std::set<E>& b)
{
    std::set<E> result;
    for (const auto& e : a)
        if (!contains(b, e))
            result.insert(e);
    return result;
}


template<typename E>
std::set<E>
a_union_b(const std::set<E>& a, const std::set<E>& b)
{
    std::set<E> result = a;
    result.insert(b.begin(), b.end());
    return result;
}


template<typename E, typename IteratorStart, typename IteratorEnd, typename UnaryPred>
std::vector<E> filter(IteratorStart it1, IteratorEnd it2, UnaryPred pred)
{
    std::vector<E> result;
    std::copy_if(it1, it2, std::back_inserter(result), pred);
    return result;
}


} // namespace sdf


