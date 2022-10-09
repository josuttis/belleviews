#ifndef __NICO_CLASS_STRING
#define __NICO_CLASS_STRING

#include <cstring>
#include <iostream>

#if __cplusplus > 201402L
#if __has_include(<string_view>)
#include <string_view>
#define HAS_STRINGVIEW
#endif
#endif

// use string::log(val)
// to log (0: off, 5: all)
class string
{
  private:
    size_t siz = 0;
    size_t capa = 0;
    char* val = nullptr;
    bool destroyed = false;
    bool moved = false;

  //***** THE LOG API:
  private:
    static int& numMallocs() {
      static int num = 0;
      return num;
    }
    static void numMallocsInc() {
      numMallocs() += 1;
    }
  public:
    static void numMallocsReset() {
      numMallocs() = 0;
    }
    static void numMallocsStatus() {
      std::cout << "      num mallocs: " << numMallocs() << "\n";
    }
    static int log (int l = -1) {
        static int lg = 0;   // log level (0: off, 5: all)
        if (l >= 0) {
          lg = l;
        }
        return lg;
    }

  //***** THE FUNCTIONAL API:
  private:
    char* alloc(std::size_t sz) {
      if (log()>=4) std::cout << "        ALLOC for capacity " << sz << "\n";
      numMallocsInc();
      auto ret = new char[sz+1];
      ret[0] = '?';
      if (sz > 1) ret[1] = '?';
      if (sz > 2) ret[2] = '?';
      return ret; 
    }
    void dealloc() {
      if (val) {
        if (log()>=4) std::cout << "        DEALLOC '" << *this << "' with capacity " << capa << "\n";
        val[0] = '?';
        if (capa > 1) val[1] = '?';
        if (capa > 2) val[2] = '?';
        delete [] val;
        val = nullptr;
        capa = 0;
      }
    }
  public:
    void reserve(std::size_t newCapa) {
        if (log()>=3) std::cout << "      RESERVE(" << newCapa << ")\n";
        if (newCapa > capa) { 
          auto newval = alloc(newCapa);
          if (val) {
            if (log()>=4) std::cout << "        COPY characters\n";
            std::memcpy(newval, val, siz+1);  // save old value (if any)
          }
          dealloc();
          val = newval;
          capa = newCapa;
        }
    }

    string() {
        if (log()>=3) std::cout << "      CONSTRUCT default\n";
    }

    string(const char* s) 
     : siz{std::strlen(s)}
    {
        if (log()>=2) std::cout << "      CONSTRUCT from const char* '" << s << "'\n";
        val = alloc(siz);
        capa = siz;
        std::memcpy(val, s, siz+1);
    }
    string (const string& s)
     : siz{s.siz} {
        if (log()>=1) std::cout << "      CONSTRUCT/COPY from string '" << s << "'\n";
        val = alloc(siz);
        capa = siz;
        std::memcpy(val, s.val, siz+1);
    }

    string (string&& s)
     : siz{s.siz}, capa{s.capa}, val{s.val}
    {
        if (log()>=1) std::cout << "      CONSTRUCT/MOVE from string '" << *this << "'\n";
        s.val = nullptr;
        s.siz = 0;
        s.capa = 0;
        s.moved = true;
    }

#ifdef HAS_STRINGVIEW
    explicit string(std::string_view sv)
     : siz{sv.size()}
    {
        if (log()>=2) std::cout << "      CREATE from std::string_view '" << sv << "'\n";
        val = alloc(siz);
        capa = siz;
        std::memcpy(val,sv.data(),siz);
        val[siz] = '\0';
    }
    operator std::string_view() const noexcept {
        return std::string_view(val, siz);
    }
#endif

    ~string() {
        if (log()>=3) std::cout << "      DESTRUCT '" << *this << "' (capa: " << capa << ")\n";
        dealloc();
        val = nullptr;
        //siz = 0;
        //capa = 0;
        destroyed = true;
    }

    string& operator= (const string& s) {
        if (destroyed) std::cout << "      OP= of " << s << " to destroyed string\n";
        if (s.destroyed) std::cout << "      OP= of destroyed string to " << s << "\n";
        if (s.moved) std::cout << "      OP= of moved string to " << s << "\n";
        if (log()>=1) std::cout << "      COPY ASSIGN '" << s << "' to '" << *this << "'\n";
        if (this == &s) return *this;
        siz = s.siz;
        val = alloc(siz);
        capa = siz;
        std::memcpy(val,s.val,siz+1);
        return *this;
    }

    string& operator= (string&& s) {
        if (destroyed) std::cout << "      OP= of " << s << " to destroyed string\n";
        if (s.destroyed) std::cout << "      OP= of destroyed string to " << s << "\n";
        if (s.moved) std::cout << "      OP= of moved string to " << s << "\n";
        if (log()>=1) std::cout << "      MOVE ASSIGN '" << s << "' to '" << *this << "'\n";
        if (this == &s) return *this;
        std::swap(val, s.val);
        std::swap(siz, s.siz);
        std::swap(capa, s.capa);
        //s.dealloc();
        //s.val = nullptr;
        //s.siz = 0;
        //s.capa = 0;
        s.moved = true;
        return *this;
    }

    static void inspect(const string& s) {
      if (s.val) {
        std::cout << "        val: '" << s.val << "' siz: " << s.siz << " capa: " << s.capa << '\n';  
      }
      else {
        std::cout << "        val: nullptr\n";
      }
    }

    std::size_t size() const {
      return siz;
    }
    bool empty() const {
      return siz == 0;
    }

    const char& operator [] (std::size_t i) const & {
        if (log()>=1) std::cout << "      OP[" << i << "] const& '" << *this << "'\n";
        if (destroyed) std::cout << "      OP[" << i << "] const& on destroyed string\n";
        if (moved) std::cout << "      OP[" << i << "] const& on moved string\n";
        if (i >= siz) std::cout << "      OP[" << i << "] const&: index > size\n";
        return val[i];
    }
    char& operator [] (std::size_t i) & {
        if (log()>=1) std::cout << "      OP[" << i << "] & '" << *this << "'\n";
        if (destroyed) std::cout << "      OP[" << i << "] const& on destroyed string\n";
        if (moved) std::cout << "      OP[" << i << "] const& on moved string\n";
        if (i >= siz) std::cout << "      OP[" << i << "] const&: index > size\n";
        return val[i];
    }
    char operator [] (std::size_t i) && {
        if (log()>=1) std::cout << "      OP[" << i << "] && '" << *this << "'\n";
        if (destroyed) std::cout << "      OP[" << i << "] const& on destroyed string\n";
        if (moved) std::cout << "      OP[" << i << "] const& on moved string\n";
        if (i >= siz) std::cout << "      OP[" << i << "] const&: index > size\n";
        return val[i];
    }

    friend bool operator < (const string& s1, const string& s2)
    {
      if (s2.val == nullptr) return false;
      if (s1.val == nullptr) return true;
      return std::strcmp(s1.val,s2.val) < 0;
    }

    friend std::ostream& operator << (std::ostream& strm, const string& s) {
        return strm << (s.val ? s.val : "");
    }

    typedef const char* iterator;
    typedef char* const_iterator;
    iterator begin() {
      return val;
    }
    const_iterator begin() const {
      return val;
    }
    const_iterator end() const {
      return val + siz;
    }
    iterator end() {
      return val + siz;
    }

  private:
    string (const string& s, const string& t)
     : siz{s.siz + t.siz} {
        if (log()>=2) std::cout << "      CREATE + constructor for '" << s << "' and '" << t << "'\n";
        val = alloc(siz);
        capa = siz;
        std::memcpy(val, s.val, s.siz);
        std::memcpy(val+s.siz, t.val, t.siz+1);
    }
  public:
    friend string operator+(const string& s1, const string& s2) {
      return string{s1, s2};
    }
};

#endif // __NICO_CLASS_STRING
