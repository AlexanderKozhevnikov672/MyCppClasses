#include <algorithm>
#include <cstring>
#include <iostream>

class String {
  private:
    size_t cap;
    size_t sz;
    char* arr;

    void Swap(String& str);

    void raiseCapIfRequired(size_t req_sz);

    bool checkSubstring(const String& substring, size_t pos) const;

  public:
    String();

    String(const char* c_str);

    String(size_t n, char c);

    String(char c);

    String(const String& str);

    String& operator=(String str);

    ~String();

    const char& operator[](size_t id) const;

    char& operator[](size_t id);

    size_t length() const;

    void push_back(char c);

    void pop_back();

    const char& front() const;

    char& front();

    const char& back() const;

    char& back();

    String& operator+=(char c);

    String& operator+=(const String& str);

    size_t find(const String& substring) const;

    size_t rfind(const String& substring) const;

    bool empty() const;

    void clear();

    void shrink_to_fit();

    const char* data() const;

    char* data();

    size_t size() const;

    size_t capacity() const;

    String substr(size_t start, size_t count) const;
};

bool operator==(const String& str1, const String& str2);

bool operator!=(const String& str1, const String& str2);

bool operator<(const String& str1, const String& str2);

bool operator>=(const String& str1, const String& str2);

bool operator>(const String& str1, const String& str2);

bool operator<=(const String& str1, const String& str2);

String operator+(String str1, const String& str2);

std::istream& operator>>(std::istream& in, String& str);

std::ostream& operator<<(std::ostream& out, const String& str);
