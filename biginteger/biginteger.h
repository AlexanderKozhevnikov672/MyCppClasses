#include <deque>
#include <iostream>
#include <string>

class BigInteger {
  private:
    static const int kNumSys = 1'000'000'000, kDigLen = 9;
    bool isNegative;
    std::deque<int> digits;

    void swap(BigInteger& bnum);

    void removeLeadingZeros();

  public:
    BigInteger();

    BigInteger(int num);

    BigInteger(std::string s);

    explicit operator bool() const;

    const int& operator[](size_t pos) const;

    int& operator[](size_t pos);

    size_t numLength() const;

    int numSign() const;

    void clear();

    BigInteger operator-() const;

    BigInteger& operator++();

    BigInteger operator++(int);

    BigInteger& operator--();

    BigInteger operator--(int);

    BigInteger& operator+=(const BigInteger& bnum);

    BigInteger& operator-=(const BigInteger& bnum);

    BigInteger& operator*=(const BigInteger& bnum);

    BigInteger& operator/=(BigInteger bnum);

    BigInteger& operator%=(BigInteger bnum);

    std::string toString() const;

    void siftLeft();
};

BigInteger operator""_bi(unsigned long long num);

BigInteger operator""_bi(const char* str, size_t /*unused*/);

std::istream& operator>>(std::istream& in, BigInteger& bnum);

std::ostream& operator<<(std::ostream& out, const BigInteger& bnum);

bool operator==(const BigInteger& bnum1, const BigInteger& bnum2);

bool operator!=(const BigInteger& bnum1, const BigInteger& bnum2);

bool operator<(const BigInteger& bnum1, const BigInteger& bnum2);

bool operator>=(const BigInteger& bnum1, const BigInteger& bnum2);

bool operator>(const BigInteger& bnum1, const BigInteger& bnum2);

bool operator<=(const BigInteger& bnum1, const BigInteger& bnum2);

BigInteger operator+(BigInteger bnum1, const BigInteger& bnum2);

BigInteger operator-(BigInteger bnum1, const BigInteger& bnum2);

BigInteger operator*(BigInteger bnum1, const BigInteger& bnum2);

BigInteger operator/(BigInteger bnum1, const BigInteger& bnum2);

BigInteger operator%(BigInteger bnum1, const BigInteger& bnum2);

class Rational {
  private:
    BigInteger numerator, denominator;

    BigInteger gcd(BigInteger bnum1, BigInteger bnum2);

    void reduceFraction();

    void swap(Rational& rnum);

  public:
    Rational();

    Rational(int num);

    Rational(BigInteger bnum);

    BigInteger getNumerator() const;

    BigInteger getDenominator() const;

    Rational operator-() const;

    Rational& operator+=(const Rational& rnum);

    Rational& operator-=(const Rational& rnum);

    Rational& operator*=(const Rational& rnum);

    Rational& operator/=(const Rational& rnum);

    std::string toString() const;

    std::string asDecimal(size_t precision = 0) const;

    explicit operator double() const;
};

Rational operator+(Rational rnum1, const Rational& rnum2);

Rational operator-(Rational rnum1, const Rational& rnum2);

Rational operator*(Rational rnum1, const Rational& rnum2);

Rational operator/(Rational rnum1, const Rational& rnum2);

bool operator==(const Rational& rnum1, const Rational& rnum2);

bool operator!=(const Rational& rnum1, const Rational& rnum2);

bool operator<(const Rational& rnum1, const Rational& rnum2);

bool operator>=(const Rational& rnum1, const Rational& rnum2);

bool operator>(const Rational& rnum1, const Rational& rnum2);

bool operator<=(const Rational& rnum1, const Rational& rnum2);
