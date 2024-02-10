#include "string.h"

void String::Swap(String& str) {
    std::swap(cap, str.cap);
    std::swap(sz, str.sz);
    std::swap(arr, str.arr);
}

void String::raiseCapIfRequired(size_t req_sz) {
    if (cap - sz >= req_sz) {
        return;
    }
    while (cap - sz < req_sz) {
        cap = cap * 2 + 1;
    }
    char* new_arr = new char[cap + 1];
    std::copy(arr, arr + sz + 1, new_arr);
    std::swap(new_arr, arr);
    delete[] new_arr;
}

bool String::checkSubstring(const String& substring, size_t pos) const {
    return memcmp(arr + pos, substring.arr, substring.sz) == 0;
}

String::String() : cap(0), sz(0), arr(new char[1]) {
    arr[0] = '\0';
}

String::String(const char* c_str)
    : cap(strlen(c_str)), sz(cap), arr(new char[cap + 1]) {
    std::copy(c_str, c_str + cap + 1, arr);
}

String::String(size_t n, char c) : cap(n), sz(n), arr(new char[n + 1]) {
    std::fill(arr, arr + n, c);
    arr[n] = '\0';
}

String::String(char c) : cap(1), sz(1), arr(new char[2]) {
    arr[0] = c;
    arr[1] = '\0';
}

String::String(const String& str)
    : cap(str.cap), sz(str.sz), arr(new char[str.cap + 1]) {
    std::copy(str.arr, str.arr + str.sz + 1, arr);
}

String& String::operator=(String str) {
    Swap(str);
    return *this;
}

String::~String() {
    delete[] arr;
}

const char& String::operator[](size_t id) const {
    return arr[id];
}

char& String::operator[](size_t id) {
    return arr[id];
}

size_t String::length() const {
    return sz;
}

void String::push_back(char c) {
    raiseCapIfRequired(1);
    arr[sz] = c;
    arr[++sz] = '\0';
}

void String::pop_back() {
    arr[--sz] = '\0';
}

const char& String::front() const {
    return arr[0];
}

char& String::front() {
    return arr[0];
}

const char& String::back() const {
    return arr[sz - 1];
}

char& String::back() {
    return arr[sz - 1];
}

String& String::operator+=(char c) {
    push_back(c);
    return *this;
}

String& String::operator+=(const String& str) {
    raiseCapIfRequired(str.sz);
    std::copy(str.arr, str.arr + str.sz + 1, arr + sz);
    sz += str.sz;
    return *this;
}

size_t String::find(const String& substring) const {
    for (size_t i = 0; i + substring.sz <= sz; ++i) {
        if (checkSubstring(substring, i)) {
            return i;
        }
    }
    return sz;
}

size_t String::rfind(const String& substring) const {
    for (int i = sz - substring.sz; i >= 0; --i) {
        if (checkSubstring(substring, static_cast<size_t>(i))) {
            return static_cast<size_t>(i);
        }
    }
    return sz;
}

bool String::empty() const {
    return sz == 0;
}

void String::clear() {
    sz = 0;
    arr[0] = '\0';
}

void String::shrink_to_fit() {
    if (cap == sz) {
        return;
    }
    cap = sz;
    char* new_arr = new char[cap + 1];
    std::copy(arr, arr + sz + 1, new_arr);
    std::swap(arr, new_arr);
    delete[] new_arr;
}

const char* String::data() const {
    return arr;
}

char* String::data() {
    return arr;
}

size_t String::size() const {
    return sz;
}

size_t String::capacity() const {
    return cap;
}

String String::substr(size_t start, size_t count) const {
    String res(count, '\0');
    std::copy(arr + start, arr + start + count, res.arr);
    return res;
}

bool operator==(const String& str1, const String& str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    return strcmp(str1.data(), str2.data()) == 0;
}

bool operator!=(const String& str1, const String& str2) {
    return !(str1 == str2);
}

bool operator<(const String& str1, const String& str2) {
    return strcmp(str1.data(), str2.data()) < 0;
}

bool operator>=(const String& str1, const String& str2) {
    return !(str1 < str2);
}

bool operator>(const String& str1, const String& str2) {
    return str2 < str1;
}

bool operator<=(const String& str1, const String& str2) {
    return !(str2 < str1);
}

String operator+(String str1, const String& str2) {
    str1 += str2;
    return str1;
}

std::istream& operator>>(std::istream& in, String& str) {
    str.clear();
    char now_c = ' ';
    while (isspace(now_c) != 0 && in.get(now_c)) {}
    do {
        str.push_back(now_c);
    } while (in.get(now_c) && isspace(now_c) == 0);
    return in;
}

std::ostream& operator<<(std::ostream& out, const String& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        out << str[i];
    }
    return out;
}
