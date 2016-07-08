#pragma once
#include <vector>
#include <algorithm>
#include <iterator>

namespace setOp
{
    /**
    * Use these operations if the sets are not sorted and no sort should be applied.
    * O(m*n) complexity where m is the size of s1 and n the size of s2.
    * This can be faster than setOp::sort operations for small sets. Profiling is key.
    */

    template <class T>
    std::vector<T> intersection(const std::vector<T>& s1, const std::vector<T>& s2);

    template <class T>
    std::vector<T> difference(const std::vector<T>& s1, const std::vector<T>& s2);

    template <class T>
    std::vector<T> unionOf(const std::vector<T>& s1, const std::vector<T>& s2);

    template <class T>
    bool equal(const std::vector<T>& s1, const std::vector<T>& s2);

    /**
    * Use these operations if the given sets are already sorted.
    * O(m+n) complexity where m is the size of s1 and n the size of s2.
    */
    namespace sorted
    {
        template <class TContainer>
        TContainer intersection(const TContainer& s1, const TContainer& s2);

        template <class TContainer>
        TContainer difference(const TContainer& s1, const TContainer& s2);

        template <class TContainer>
        TContainer unionOf(const TContainer& s1, const TContainer& s2);

        template <class TContainer>
        bool equal(const TContainer& s1, const TContainer& s2);
    }

    /**
    * Use these operations if a sort should be applied first.
    * O(m * log(m) + n * log(n) + m + n) complexity where m is the size of s1 and n the size of s2.
    */
    namespace sort
    {
        template <class TContainer>
        TContainer intersection(TContainer& s1, TContainer& s2);

        template <class TContainer>
        TContainer difference(TContainer& s1, TContainer& s2);

        template <class TContainer>
        TContainer unionOf(TContainer& s1, TContainer& s2);

        template <class TContainer>
        bool equal(TContainer& s1, TContainer& s2);
    }
}

// Simple custom implementation for non-sorted
template <class T>
std::vector<T> setOp::intersection(const std::vector<T>& s1, const std::vector<T>& s2)
{
    std::vector<T> r;

    for (size_t i = 0; i < s1.size(); ++i)
        for (size_t j = 0; j < s2.size(); ++j)
            if (s1[i] == s2[j])
                r.push_back(s1[i]);

    return r;
}

template <class T>
std::vector<T> setOp::difference(const std::vector<T>& s1, const std::vector<T>& s2)
{
    std::vector<T> r;

    for (size_t i = 0; i < s1.size(); ++i)
        if (std::find(s2.begin(), s2.end(), s1[i]) == s2.end())
            r.push_back(s1[i]);

    return r;
}

template <class T>
std::vector<T> setOp::unionOf(const std::vector<T>& s1, const std::vector<T>& s2)
{
    std::vector<T> r = s1;

    for (size_t i = 0; i < s2.size(); ++i)
        if (std::find(s1.begin(), s1.end(), s2[i]) == s1.end())
            r.push_back(s2[i]);

    return r;
}

template <class T>
bool setOp::equal(const std::vector<T>& s1, const std::vector<T>& s2)
{
    if (s1.size() != s2.size())
        return false;

    for (size_t i = 0; i < s1.size(); ++i)
    {
        bool s = false;
        for (size_t j = 0; j < s2.size(); ++j)
            if (s1[i] == s2[j])
            {
                s = true;
                break;
            }

        if (!s)
            return false;
    }

    return true;
}

// Already sorted
template <class TContainer>
TContainer setOp::sorted::intersection(const TContainer& s1, const TContainer& s2)
{
    TContainer result;
    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(result));
    return result;
}

template <class TContainer>
TContainer setOp::sorted::difference(const TContainer& s1, const TContainer& s2)
{
    TContainer result;
    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(result, result.begin()));
    return result;
}

template <class TContainer>
TContainer setOp::sorted::unionOf(const TContainer& s1, const TContainer& s2)
{
    TContainer result;
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(result));
    return result;
}

template <class TContainer>
bool setOp::sorted::equal(const TContainer& s1, const TContainer& s2)
{
    return s1 == s2;
}

// To be sorted
template <class TContainer>
bool setOp::sort::equal(TContainer& s1, TContainer& s2)
{
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    return sorted::equal(s1, s2);
}

template <class TContainer>
TContainer setOp::sort::intersection(TContainer& s1, TContainer& s2)
{
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    return sorted::intersection(s1, s2);
}

template <class TContainer>
TContainer setOp::sort::difference(TContainer& s1, TContainer& s2)
{
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    return sorted::difference(s1, s2);
}

template <class TContainer>
TContainer setOp::sort::unionOf(TContainer& s1, TContainer& s2)
{
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    return sorted::unionOf(s1, s2);
}
