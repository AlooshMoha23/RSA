#ifndef BIGNUM_HPP
#define BIGNUM_HPP

#include <cstdint>
#include <string>
#include <vector>

class Bignum {
public:
  Bignum(int = 0);
  Bignum(unsigned);
  Bignum(std::string const &s); /*on divise la chaine par 2*/
  Bignum(Bignum const &);
  Bignum(Bignum &&);
  Bignum(uint64_t);

  Bignum &operator=(Bignum const &);
  Bignum &operator=(Bignum &&);

  Bignum &operator+=(Bignum const &);
  Bignum &operator-=(Bignum const &);
  Bignum &operator*=(Bignum const &);
  Bignum &operator/=(Bignum const &);
  Bignum &operator<<=(unsigned n);

public:
  std::vector<uint32_t> tab;
  bool isPositive;

  friend bool operator<(Bignum const &, Bignum const &);
  friend bool operator<=(Bignum const &, Bignum const &);
  friend bool operator>(Bignum const &, Bignum const &);
  friend bool operator>=(Bignum const &, Bignum const &);
  friend bool operator==(Bignum const &, Bignum const &);
  friend bool operator!=(Bignum const &, Bignum const &);

  friend std::iostream operator<<(std::iostream &, Bignum const &);

  friend Bignum operator+(Bignum const &, Bignum const &);
  friend Bignum operator-(Bignum const &, Bignum const &);
  friend Bignum operator*(Bignum const &, Bignum const &);
  friend Bignum operator/(Bignum const &, Bignum const &);
  friend Bignum operator%(Bignum const &, Bignum const &);

  friend std::pair<Bignum, Bignum> division(Bignum const &, Bignum const &);

  friend Bignum inverseMod(Bignum const &, Bignum const &);
};

#endif
