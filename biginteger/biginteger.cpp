#include "biginteger.h"
#include <deque>
#include <iostream>
#include <string>

void BigInteger::swap(BigInteger& bnum) {
    std::swap(isNegative, bnum.isNegative);
    std::swap(digits, bnum.digits);
}

void BigInteger::removeLeadingZeros() {
    while (!digits.empty() && digits.back() == 0) {
        digits.pop_back();
    }
}

BigInteger::BigInteger() : isNegative(false) {}

BigInteger::BigInteger(int num) : isNegative(num < 0) {
    num = std::abs(num);
    while (num > 0) {
        digits.push_back(num % kNumSys);
        num /= kNumSys;
    }
}

BigInteger::BigInteger(std::string s) : isNegative(s.front() == '-') {
    int start = static_cast<int>(s.front() == '-');
    for (int i = static_cast<int>(s.size()) - 1; i >= start; i -= kDigLen) {
        int now_dig = 0;
        for (int j = std::max(i - kDigLen + 1, start); j <= i; ++j) {
            now_dig = now_dig * 10 + s[j] - '0';
        }
        digits.push_back(now_dig);
    }
}

BigInteger::operator bool() const {
    return !digits.empty();
}

const int& BigInteger::operator[](size_t pos) const {
    return digits[pos];
}

int& BigInteger::operator[](size_t pos) {
    return digits[pos];
}

size_t BigInteger::numLength() const {
    return digits.size();
}

int BigInteger::numSign() const {
    if (isNegative) {
        return -1;
    }
    return 1;
}

void BigInteger::clear() {
    isNegative = false;
    digits.clear();
}

BigInteger BigInteger::operator-() const {
    BigInteger copy(*this);
    copy.isNegative = !copy.isNegative;
    return copy;
}

BigInteger& BigInteger::operator++() {
    return (*this) += 1;
}

BigInteger BigInteger::operator++(int) {
    BigInteger copy = *this;
    ++(*this);
    return copy;
}

BigInteger& BigInteger::operator--() {
    return (*this) -= 1;
}

BigInteger BigInteger::operator--(int) {
    BigInteger copy = *this;
    --(*this);
    return copy;
}

BigInteger& BigInteger::operator+=(const BigInteger& bnum) {
    if (isNegative != bnum.isNegative) {
        isNegative = !isNegative;
        (*this) -= bnum;
        isNegative = !isNegative;
        return *this;
    }
    while (digits.size() < bnum.digits.size()) {
        digits.push_back(0);
    }
    long long transf = 0;
    for (size_t i = 0; i < bnum.digits.size(); ++i) {
        transf += digits[i] + bnum.digits[i];
        if (transf < kNumSys) {
            digits[i] = transf;
            transf = 0;
        } else {
            digits[i] = transf - kNumSys;
            transf = 1;
        }
    }
    if (transf != 0 && digits.size() == bnum.digits.size()) {
        digits.push_back(transf);
    } else if (transf != 0) {
        digits[bnum.digits.size()] += transf;
    }
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& bnum) {
    if (isNegative != bnum.isNegative) {
        isNegative = !isNegative;
        (*this) += bnum;
        isNegative = !isNegative;
        return *this;
    }
    while (digits.size() < bnum.digits.size()) {
        digits.push_back(0);
    }
    for (size_t i = 0; i < bnum.digits.size(); ++i) {
        digits[i] -= bnum.digits[i];
    }
    int first_positive = -1, first_negative = -1;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (digits[i] < 0) {
            first_negative = i;
        }
        if (digits[i] > 0) {
            first_positive = i;
        }
    }
    if (first_positive < first_negative) {
        isNegative = !isNegative;
        for (size_t i = 0; i < digits.size(); ++i) {
            digits[i] *= -1;
        }
    }
    bool need_help = false;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (need_help && digits[i] <= 0) {
            digits[i] += kNumSys - 1;
        } else if (need_help) {
            --digits[i];
            need_help = false;
        } else if (digits[i] < 0) {
            digits[i] += kNumSys;
            need_help = true;
        }
    }
    removeLeadingZeros();
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& bnum) {
    BigInteger res;
    for (size_t i = 0; i < bnum.digits.size(); ++i) {
        BigInteger now_res;
        long long transf = 0;
        for (size_t j = 0; j < digits.size(); ++j) {
            transf += static_cast<long long>(digits[j]) * bnum.digits[i];
            now_res.digits.push_back(transf % kNumSys);
            transf /= kNumSys;
        }
        if (transf != 0) {
            now_res.digits.push_back(transf);
        }
        now_res.removeLeadingZeros();
        if (now_res.digits.empty()) {
            continue;
        }
        for (size_t j = 0; j < i; ++j) {
            now_res.siftLeft();
        }
        res += now_res;
    }
    res.isNegative = isNegative;
    if (bnum.isNegative) {
        res.isNegative = !res.isNegative;
    }
    swap(res);
    return *this;
}

std::string BigInteger::toString() const {
    if (digits.empty()) {
        return "0";
    }
    std::string res;
    if (isNegative) {
        res.push_back('-');
    }
    res += std::to_string(digits.back());
    for (int i = static_cast<int>(digits.size()) - 2; i >= 0; --i) {
        std::string now_dig = std::to_string(digits[i]);
        for (size_t j = now_dig.size(); j < kDigLen; ++j) {
            res.push_back('0');
        }
        res += now_dig;
    }
    return res;
}

void BigInteger::siftLeft() {
    if (!digits.empty()) {
        digits.push_front(0);
    }
}

BigInteger operator""_bi(unsigned long long num) {
    return BigInteger(std::to_string(num));
}

BigInteger operator""_bi(const char* str, size_t /*unused*/) {
    return BigInteger(std::string(str));
}

std::istream& operator>>(std::istream& in, BigInteger& bnum) {
    bnum.clear();
    std::string s;
    in >> s;
    bnum = BigInteger(s);
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& bnum) {
    return out << bnum.toString();
}

bool operator==(const BigInteger& bnum1, const BigInteger& bnum2) {
    if (bnum1.numLength() != bnum2.numLength()) {
        return false;
    }
    if (bnum1.numLength() == 0) {
        return true;
    }
    if (bnum1.numSign() != bnum2.numSign()) {
        return false;
    }
    for (size_t i = 0; i < bnum1.numLength(); ++i) {
        if (bnum1[i] != bnum2[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const BigInteger& bnum1, const BigInteger& bnum2) {
    return !(bnum1 == bnum2);
}

bool operator<(const BigInteger& bnum1, const BigInteger& bnum2) {
    if (bnum1.numLength() == 0) {
        if (bnum2.numLength() == 0) {
            return false;
        }
        return bnum2.numSign() == 1;
    }
    if (bnum2.numLength() == 0) {
        return bnum1.numSign() != 1;
    }
    if (bnum1.numSign() != bnum2.numSign()) {
        return bnum1.numSign() < bnum2.numSign();
    }
    if (bnum1.numSign() == 1) {
        if (bnum1.numLength() != bnum2.numLength()) {
            return bnum1.numLength() < bnum2.numLength();
        }
        for (int i = static_cast<int>(bnum1.numLength()) - 1; i >= 0; --i) {
            if (bnum1[i] != bnum2[i]) {
                return bnum1[i] < bnum2[i];
            }
        }
        return false;
    }
    if (bnum1.numLength() != bnum2.numLength()) {
        return bnum1.numLength() > bnum2.numLength();
    }
    for (int i = static_cast<int>(bnum1.numLength()) - 1; i >= 0; --i) {
        if (bnum1[i] != bnum2[i]) {
            return bnum1[i] > bnum2[i];
        }
    }
    return false;
}

bool operator>=(const BigInteger& bnum1, const BigInteger& bnum2) {
    return !(bnum1 < bnum2);
}

bool operator>(const BigInteger& bnum1, const BigInteger& bnum2) {
    return bnum2 < bnum1;
}

bool operator<=(const BigInteger& bnum1, const BigInteger& bnum2) {
    return !(bnum2 < bnum1);
}

BigInteger operator+(BigInteger bnum1, const BigInteger& bnum2) {
    return bnum1 += bnum2;
}

BigInteger operator-(BigInteger bnum1, const BigInteger& bnum2) {
    return bnum1 -= bnum2;
}

BigInteger operator*(BigInteger bnum1, const BigInteger& bnum2) {
    return bnum1 *= bnum2;
}

BigInteger& BigInteger::operator/=(BigInteger bnum) {
    if (bnum.isNegative) {
        isNegative = !isNegative;
        bnum.isNegative = false;
    }
    BigInteger res, suff;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        suff.siftLeft();
        suff += digits[i];
        res.siftLeft();
        int left = 0, right = kNumSys;
        while (right - left > 1) {
            int middle = (left + right) / 2;
            if (bnum * middle <= suff) {
                left = middle;
            } else {
                right = middle;
            }
        }
        suff -= bnum * left;
        res += left;
    }
    std::swap(digits, res.digits);
    return *this;
}

BigInteger& BigInteger::operator%=(BigInteger bnum) {
    if (bnum.isNegative) {
        isNegative = !isNegative;
        bnum.isNegative = false;
    }
    BigInteger res;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        res.siftLeft();
        res += digits[i];
        int left = 0, right = kNumSys;
        while (right - left > 1) {
            int middle = (left + right) / 2;
            if (bnum * middle <= res) {
                left = middle;
            } else {
                right = middle;
            }
        }
        res -= bnum * left;
    }
    std::swap(digits, res.digits);
    return *this;
}

BigInteger operator/(BigInteger bnum1, const BigInteger& bnum2) {
    return bnum1 /= bnum2;
}

BigInteger operator%(BigInteger bnum1, const BigInteger& bnum2) {
    return bnum1 %= bnum2;
}

BigInteger Rational::gcd(BigInteger bnum1, BigInteger bnum2) {
    if (bnum2 == 0) {
        return bnum1;
    }
    return gcd(bnum2, bnum1 % bnum2);
}

void Rational::reduceFraction() {
    BigInteger div = gcd(numerator, denominator);
    numerator /= div;
    denominator /= div;
    if (denominator.numSign() == -1) {
        denominator *= -1;
        numerator *= -1;
    }
}

void Rational::swap(Rational& rnum) {
    std::swap(numerator, rnum.numerator);
    std::swap(denominator, rnum.denominator);
}

Rational::Rational() : numerator(0), denominator(1) {}

Rational::Rational(int num) : numerator(num), denominator(1) {}

Rational::Rational(BigInteger bnum) : numerator(bnum), denominator(1){};

BigInteger Rational::getNumerator() const {
    return numerator;
}

BigInteger Rational::getDenominator() const {
    return denominator;
}

Rational Rational::operator-() const {
    Rational copy(*this);
    copy.numerator *= -1;
    return copy;
}

Rational& Rational::operator+=(const Rational& rnum) {
    numerator = numerator * rnum.denominator + rnum.numerator * denominator;
    denominator *= rnum.denominator;
    reduceFraction();
    return *this;
}

Rational& Rational::operator-=(const Rational& rnum) {
    numerator = numerator * rnum.denominator - rnum.numerator * denominator;
    denominator *= rnum.denominator;
    reduceFraction();
    return *this;
}

Rational& Rational::operator*=(const Rational& rnum) {
    numerator *= rnum.numerator;
    denominator *= rnum.denominator;
    reduceFraction();
    return *this;
}

Rational& Rational::operator/=(const Rational& rnum) {
    numerator *= rnum.denominator;
    denominator *= rnum.numerator;
    reduceFraction();
    return *this;
}

std::string Rational::toString() const {
    std::string res(numerator.toString());
    if (denominator != 1) {
        res.push_back('/');
        res += denominator.toString();
    }
    return res;
}

std::string Rational::asDecimal(size_t precision) const {
    BigInteger copy(numerator);
    for (size_t i = 0; i < precision; ++i) {
        copy *= 10;
    }
    copy /= denominator;
    std::string res(copy.toString());
    if (precision == 0) {
        return res;
    }
    size_t len = res.size();
    if (res.front() == '-') {
        --len;
    }
    if (len <= precision) {
        for (size_t i = 0; i <= precision - len; ++i) {
            res.push_back('0');
        }
        size_t start = static_cast<size_t>(res.front() == '-');
        for (size_t i = 0; i < len; ++i) {
            std::swap(res[res.size() - i - 1], res[start + len - i - 1]);
        }
    }
    res.push_back('.');
    for (int i = static_cast<int>(res.size()) - 1;
         i >= static_cast<int>(res.size() - precision); --i) {
        std::swap(res[i], res[i - 1]);
    }
    return res;
}

Rational::operator double() const {
    return std::stod((*this).asDecimal(18));
}

Rational operator+(Rational rnum1, const Rational& rnum2) {
    return rnum1 += rnum2;
}

Rational operator-(Rational rnum1, const Rational& rnum2) {
    return rnum1 -= rnum2;
}

Rational operator*(Rational rnum1, const Rational& rnum2) {
    return rnum1 *= rnum2;
}

Rational operator/(Rational rnum1, const Rational& rnum2) {
    return rnum1 /= rnum2;
}

bool operator==(const Rational& rnum1, const Rational& rnum2) {
    return rnum1.getNumerator() == rnum2.getNumerator() &&
           rnum1.getDenominator() == rnum2.getDenominator();
}

bool operator!=(const Rational& rnum1, const Rational& rnum2) {
    return !(rnum1 == rnum2);
}

bool operator<(const Rational& rnum1, const Rational& rnum2) {
    return rnum1.getNumerator() * rnum2.getDenominator() <
           rnum1.getDenominator() * rnum2.getNumerator();
}

bool operator>=(const Rational& rnum1, const Rational& rnum2) {
    return !(rnum1 < rnum2);
}

bool operator>(const Rational& rnum1, const Rational& rnum2) {
    return rnum2 < rnum1;
}

bool operator<=(const Rational& rnum1, const Rational& rnum2) {
    return !(rnum2 < rnum1);
}
