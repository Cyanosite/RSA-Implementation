#ifndef BIGINT_H
#define BIGINT_H

#include <exception>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <random>
#include "memtrace.h"

/**
 * checks whether the input string contains only hexadecimal characters
 * @param x the input string
 * @return true if the string only contains hexadecimal characters
 */
bool string_check(const char *x)
{
    size_t size_of_x = strlen(x);
    for (size_t i = 0; i < size_of_x; ++i)
    {
        // isxdigit returns true if the character is hexadecimal ([a-fA-F]|[0-9])
        if (!isxdigit(x[i]))
            return false;
    }
    return true;
}

/**
 * @tparam bits the number of bits used for storage.
 * If bits % 32 != 0: it will be rounded downwards to the nearest multiple of 32.
 */
template <unsigned int bits = 64>
struct Bigint
{
    // stores the number in a little-endian order
    unsigned int *storage;
    Bigint(const unsigned long long & = 0);
    //
    Bigint(const char *const &);
    Bigint(const Bigint &);
    Bigint &operator=(const Bigint &);
    // randomizes the number up to (input/32) bits
    void rng(const unsigned int & = 0);
    bool operator==(const Bigint &) const;
    bool operator!=(const Bigint &) const;
    bool operator<(const Bigint &) const;
    bool operator>(const Bigint &) const;
    // returns the number of bits used to represent the number
    unsigned int num_bits() const;
    bool is_even() const;
    bool is_odd() const;
    Bigint operator+(const Bigint &) const;
    Bigint operator-(const Bigint &) const;
    Bigint operator*(const Bigint &) const;
    Bigint operator/(const Bigint &) const;
    Bigint operator%(const Bigint &) const;
    Bigint operator<<(const unsigned int &) const;
    Bigint operator>>(const unsigned int &) const;
    // algorithms
    // greatest common divisor
    Bigint gcd(const Bigint &) const;
    // modular exponentiation
    Bigint exponentiation(const Bigint<bits> &, const Bigint<bits> &) const;
    // modular multiplicative inverse
    Bigint inverse(const Bigint &) const;
    // Fermat primality test
    bool prime_check() const;
    ~Bigint();
};

/**
 * @param x a 64 bit integer.
 */
template <unsigned int bits>
Bigint<bits>::Bigint(const unsigned long long &x)
{
    storage = new unsigned int[bits / (sizeof(unsigned int) * 8)]{0};
    storage[0] = x;
    storage[1] = x >> (sizeof(unsigned int) * 8);
}

/**
 * @param x C string with a '\0' null terminator.
 * It may only consist a hexadecimal number (letter capitalization doesn't matter).
 */
template <unsigned int bits>
Bigint<bits>::Bigint(const char *const &x)
{
    if (!string_check(x))
        throw std::domain_error("found non-hexadecimal character in input string");
    storage = new unsigned int[bits / (sizeof(unsigned int) * 8)]{0};
    unsigned short number_of_runs = strlen(x) / 8;
    // a run consists of reading 8 hexadecimal digits enough to fill 32 bits
    if (number_of_runs > 1)
    {
        // starting 8 characters before the end and decrementing it by 8 every run
        const char *startpos = x + strlen(x) * sizeof(char) - 8 * sizeof(char);
        int i = 0;
        for (; i < number_of_runs; ++i)
        {
            sscanf(startpos, "%8X", &storage[i]);
            startpos -= 8;
        }
        if (strlen(x) % 8 != 0)
        {
            char read_this[7] = "%";
            sprintf(read_this + 1 * sizeof(char), "%dX", (int)strlen(x) % 8);
            sscanf(x, read_this, &storage[i]);
        }
    }
    else
        sscanf(x, "%X", &storage[0]);
}

template <unsigned int bits>
Bigint<bits>::Bigint(const Bigint &x)
{
    storage = new unsigned int[bits / (sizeof(unsigned int) * 8)];
    std::memcpy(storage, x.storage, bits / 8);
}

template <unsigned int bits>
Bigint<bits> &Bigint<bits>::operator=(const Bigint &x)
{
    if (this != &x)
    {
        delete[] storage;
        storage = new unsigned int[bits / (sizeof(unsigned int) * 8)];
        std::memcpy(storage, x.storage, bits / 8);
    }
    return *this;
}

/**
 * @param size_max upper limit of randomization in terms of bit size.
 * If (size_max = 0) => the entire size of its storage will be randomized
 * @return randomized output (size in bits = size_max/32) using std::random_device.
 * Not that std::random_device may produce deterministic results depending on the device!
 */
template <unsigned int bits>
void Bigint<bits>::rng(const unsigned int &size_max)
{
    std::random_device rd;
    if (size_max == 0)
    {
        unsigned int n = bits / (sizeof(unsigned int) * 8);
        for (unsigned short i = 0; i < n; ++i)
        {
            this->storage[i] = rd();
        }
    }
    else
    {
        unsigned int n = size_max / (sizeof(unsigned int) * 8);
        for (unsigned short i = 0; i < n; ++i)
        {
            this->storage[i] = rd();
        }
    }
}

template <unsigned int bits>
bool Bigint<bits>::operator==(const Bigint &x) const
{
    for (unsigned int i = 0; i < bits / (sizeof(unsigned int) * 8); ++i)
    {
        // if any element of the array doesn't match they're not equal
        if (storage[i] != x.storage[i])
            return false;
    }
    return true;
}

template <unsigned int bits>
bool Bigint<bits>::operator!=(const Bigint &x) const
{
    for (unsigned int i = 0; i < bits / (sizeof(unsigned int) * 8); ++i)
    {
        // if any element of the array doesn't match they're not equal
        if (storage[i] != x.storage[i])
            return true;
    }
    return false;
}

template <unsigned int bits>
bool Bigint<bits>::operator<(const Bigint &x) const
{
    // start the loop from the MSB
    for (int i = bits / (sizeof(unsigned int) * 8) - 1; i >= 0; --i)
    {
        // the first non-zero element will determine the return value
        if (storage[i] != 0 || x.storage[i] != 0)
        {
            return storage[i] < x.storage[i];
        }
    }
    // if the inputs are 0 it will return false
    return false;
}

template <unsigned int bits>
bool Bigint<bits>::operator>(const Bigint &x) const
{
    // start the loop from MSB
    for (int i = bits / (sizeof(unsigned int) * 8) - 1; i >= 0; --i)
    {
        // the first non-zero element will determine the return value
        if (storage[i] != 0 || x.storage[i] != 0)
        {
            return storage[i] > x.storage[i];
        }
    }
    // if the inputs are 0 it will return false
    return false;
}

/**
 * @return the number of bits sufficient to represent *this
 */
template <unsigned int bits>
unsigned int Bigint<bits>::num_bits() const
{
    unsigned int i = bits / (sizeof(unsigned int) * 8) - 1;
    // i will be equal to the amount of array elements that are zero starting from MSB
    while (i > 0 && storage[i] == 0)
        --i;
    return storage[i] == 0 ? i * 8 * sizeof(unsigned int) : (i + 1) * 8 * sizeof(unsigned int) - __builtin_clz(storage[i]);
}

template <unsigned int bits>
bool Bigint<bits>::is_even() const
{
    // check LSB
    return !(this->storage[0] & 1);
}

template <unsigned int bits>
bool Bigint<bits>::is_odd() const
{
    // check LSB
    return this->storage[0] & 1;
}

template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator+(const Bigint &x) const
{
    Bigint res;
    unsigned int carry = 0;
    unsigned long long temp;
    for (unsigned int i = 0; i < bits / (sizeof(unsigned int) * 8); ++i)
    {
        // add together the current array elements of both inputs and the last carry
        temp = (unsigned long long)storage[i] + (unsigned long long)x.storage[i] + (unsigned long long)carry;
        // the carry will be the upper half of the unsigned long long integer
        // so we shift it down by that amount
        carry = temp >> (8 * sizeof(unsigned int));
        res.storage[i] = (unsigned int)temp;
    }
    return res;
}

template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator-(const Bigint &x) const
{
    Bigint res;
    unsigned int borrow = 0;
    unsigned long long temp;
    for (unsigned int i = 0; i < bits / (sizeof(unsigned int) * 8); ++i)
    {
        // subtract the inputs and add the last borrow
        temp = (unsigned long long)storage[i] - ((unsigned long long)x.storage[i] + (unsigned long long)borrow);
        borrow = temp >> (8 * sizeof(unsigned long long) - 1);
        res.storage[i] = (unsigned int)temp;
    }
    return res;
}

/**
 * This multiplication uses the classic schoolbook multiplication method
 * @return The return value will be the same size as the inputs, it will overflow
 * if the numbers are too big. Choose sufficiently large inputs ensuring it won't overflow.
 */
template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator*(const Bigint &x) const
{
    Bigint res;
    unsigned long long carry = 0;
    unsigned short k = 0;
    unsigned long long temp;
    for (unsigned int i = 0; i < bits / (sizeof(unsigned int) * 8); ++i)
    {
        carry = 0;
        for (unsigned int j = 0; j < bits / (sizeof(unsigned int) * 8); ++j)
        {
            k = i + j;
            if (k < bits / (sizeof(unsigned int) * 8))
            {
                temp = (unsigned long long)res.storage[k] + (unsigned long long)storage[i] * (unsigned long long)x.storage[j] + (unsigned long long)carry;
                res.storage[k] = (unsigned int)temp;
                carry = temp >> (8 * sizeof(unsigned int));
            }
            else
                break;
        }
    }
    return res;
}

template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator/(const Bigint &x) const
{
    if (*this < x)
        return *this;
    unsigned int bd = this->num_bits() - x.num_bits();
    Bigint rem(*this);
    Bigint quo;
    Bigint c(x);
    Bigint e(1);
    c = c << bd;
    e = e << bd;
    // MSB of storage
    unsigned int ms_part = bits / (sizeof(unsigned int) * 8) - 1;
    // 31 bits
    unsigned int bitsize_m_1 = (sizeof(unsigned int) * 8) - 1;
    Bigint r;
    while (true)
    {
        r = rem - c;
        unsigned int d = 1 - (r.storage[ms_part] >> bitsize_m_1);
        if (d != 0)
        {
            rem = r;
            r = quo + e;
            quo = r;
        }
        if (bd-- == 0)
            break;
        c = c >> 1;
        e = e >> 1;
    }
    return quo;
}

/**
 * uses the same algorithm as division but returns the remainder
 */
template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator%(const Bigint &x) const
{
    if (*this < x)
        return *this;
    unsigned int bd = this->num_bits() - x.num_bits();
    Bigint rem(*this);
    Bigint c(x);
    c = c << bd;
    unsigned int ms_part = bits / (sizeof(unsigned int) * 8) - 1;
    unsigned int bitsize_m_1 = (sizeof(unsigned int) * 8) - 1;
    Bigint r;
    while (true)
    {
        r = rem - c;
        unsigned int d = r.storage[ms_part] >> bitsize_m_1;
        if (d == 0)
            rem = r;
        if (bd-- == 0)
            break;
        c = c >> 1;
    }
    return rem;
}

template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator<<(const unsigned int &shift) const
{
    if (shift >= bits)
        return Bigint();
    Bigint ret;
    unsigned int full_shifts = shift / (sizeof(unsigned int) * 8);
    unsigned int lshift = shift % (sizeof(unsigned int) * 8);
    unsigned int rshift = sizeof(unsigned int) * 8 - lshift;
    if (lshift == 0)
    {
        for (unsigned int i = bits / (sizeof(unsigned int) * 8) - 1; i > full_shifts; --i)
            ret.storage[i] = storage[i - full_shifts] << lshift;
    }
    else
    {
        for (unsigned int i = bits / (sizeof(unsigned int) * 8) - 1; i > full_shifts; --i)
        {
            unsigned int lo = storage[i - full_shifts] << lshift;
            lo |= storage[i - full_shifts - 1] >> rshift;
            ret.storage[i] = lo;
        }
    }
    ret.storage[full_shifts] = storage[0] << lshift;
    return ret;
}

template <unsigned int bits>
Bigint<bits> Bigint<bits>::operator>>(const unsigned int &shift) const
{
    if (shift >= bits)
        return Bigint();
    Bigint ret;
    unsigned int full_shifts = shift / (sizeof(unsigned int) * 8);
    unsigned int small_shift = shift & ((sizeof(unsigned int) * 8) - 1);
    unsigned int n = bits / (sizeof(unsigned int) * 8) - full_shifts;
    if (small_shift == 0)
    {
        for (unsigned int i = 0; i < n; ++i)
            ret.storage[i] = storage[i + full_shifts];
    }
    else
    {
        for (unsigned int i = 0; i < n - 1; ++i)
        {
            unsigned int lo = storage[i + full_shifts] >> small_shift;
            lo |= storage[i + full_shifts + 1] << (sizeof(unsigned int) * 8 - small_shift);
            ret.storage[i] = lo;
        }
        ret.storage[n - 1] = storage[n - 1 + full_shifts] >> small_shift;
    }
    return ret;
}

template <unsigned int bits>
Bigint<bits>::~Bigint()
{
    delete[] storage;
}

/**
 * Euclidean algorithm finding the greatest common divisor of the given inputs.
 * @tparam bits number of bits used to store the integers, usually ommited in functions calls.
 * @param a
 * @return Greatest common divisor of a and b.
 */
template <unsigned int bits>
Bigint<bits> Bigint<bits>::gcd(const Bigint &b) const
{
    Bigint a = *this;
    Bigint b_temp = b;
    Bigint temp;
    const Bigint null;
    while (b_temp != null)
    {
        temp = b_temp;
        b_temp = a % b_temp;
        a = temp;
    }
    return a;
}

/**
 * Modular exponentiation algorithm.
 * @tparam bits number of bits used to store the integers, usually ommited in functions calls.
 * @param a must be less than m
 * @param b must be greater than 0
 * @param m must be larger than a
 * @return aˆb % m
 */
template <unsigned int bits>
Bigint<bits> Bigint<bits>::exponentiation(const Bigint &b, const Bigint &m) const
{
    Bigint a = *this;
    Bigint b_temp = b;
    Bigint c(1);
    const Bigint null;
    while (b_temp != null)
    {
        if (b_temp.is_odd())
        {
            c = (c * a) % m;
        }
        a = (a * a) % m;
        b_temp = b_temp >> 1;
    }
    return c;
}

/**
 * Modular multiplicative inverse algorithm (using extended Euclidean algorithm).
 * A modular multiplicative inverse of an integer a is an integer x such that the product ax is congruent to 1 with respect to the modulus m.
 * @tparam bits number of bits used to store the integers, usually ommited in functions calls.
 * @return a*t congruent 1 (mod b)
 */
template <unsigned int bits>
Bigint<bits> Bigint<bits>::inverse(const Bigint &b) const
{
    Bigint a = *this;
    Bigint b_temp = b;
    Bigint x0;
    bool x0_sign = false;
    Bigint x1(1);
    bool x1_sign = false;
    while (a > 1)
    {
        Bigint q(a / b_temp);
        Bigint t = b_temp;
        b_temp = a % b_temp;
        a = t;
        Bigint t2(x0);
        bool t2_sign = x0_sign;
        Bigint qx0(q * x0);
        if (x0_sign != x1_sign)
        {
            x0 = x1 + qx0;
            x0_sign = x1_sign;
        }
        else
        {
            x0 = (x1 > qx0) ? x1 - qx0 : qx0 - x1;
            x0_sign = x1 > qx0 ? x1_sign : !x0_sign;
        }
        x1 = t2;
        x1_sign = t2_sign;
    }
    return x1_sign ? b - x1 : x1;
}

/**
 * Fermat primality test algorithm.
 * @tparam bits number of bits used to store the integers, usually ommited in functions calls.
 * @param m the integer to test
 * @return True if m is a prime.
 */
template <unsigned int bits>
bool Bigint<bits>::prime_check() const
{
    Bigint high(*this - 1);
    const Bigint one(1);
    Bigint a;
    for (unsigned short k = 0; k < 100; ++k)
    {
        a.rng(high.num_bits());
        if (this->gcd(a) != one)
            return false;
        if (a.exponentiation(high, *this) != one)
            return false;
    }
    return true;
}

/**
 * displays the Bigint in normal ordering with hexadecimal characters
 * @param x the Bigint to display
 */
template <unsigned int bits>
std::ostream &operator<<(std::ostream &os, const Bigint<bits> &x)
{
    unsigned short size = bits / (sizeof(unsigned int) * 8);
    os << std::hex;
    // true until we find an array element that isn't 0
    // we need this because we don't want to display the leading zeros
    bool isnull = true;
    for (int i = size - 1; i >= 0; --i)
    {
        if (isnull)
        {
            if (x.storage[i] != 0)
            {
                os << (unsigned int)x.storage[i];
                isnull = false;
            }
        }
        else
            os << std::setw(8) << std::setfill('0') << (unsigned int)x.storage[i];
    }
    // if the number is actually 0 then we just display a single 0
    if (isnull)
        os << 0;
    // reset std::hex flag
    os << std::dec;
    return os;
}

#endif
