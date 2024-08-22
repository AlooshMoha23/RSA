#include "Bignum.hpp"
#include <algorithm>
#include <bitset>
#include <boost/multiprecision/cpp_int.hpp>
#include <cmath>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

void reverseAndPrint(const Bignum &x) {
  Bignum reversed(x); // Create a copy of x

  std::reverse(reversed.tab.begin(),
               reversed.tab.end()); // Reverse the tab vector

  if (!reversed.isPositive) {
    std::cout << "-";
  }

  for (const auto &digit : reversed.tab) {
    std::cout << std::hex << std::setw(8) << std::setfill('0') << digit;
  }
  std::cout << std::endl;
}

Bignum::Bignum(int value) {

  isPositive = (value >= 0);
  if (value == 0) {
    tab.push_back(0);
  } else {

    if (!isPositive) {
      value = -value; // we want to push the absolute value if its negative!
    }
    tab.push_back(value);
  }
}

Bignum::Bignum(unsigned value) {
  isPositive = true;
  if (value == 0) {
    tab.push_back(0);
  } else {
    tab.push_back(value);
  }
}

Bignum::Bignum(Bignum const &copy) {
  this->tab = copy.tab;
  this->isPositive = copy.isPositive;
}

Bignum::Bignum(Bignum &&copy) {
  this->tab = std::move(copy.tab);
  this->isPositive = copy.isPositive;

  copy.tab.clear();
}

Bignum::Bignum(std::string const &s) {
  // Check if the string is empty or contains non-digit characters
  if (s.empty() || !std::all_of(s.begin(), s.end(), ::isdigit)) {
    throw std::invalid_argument("Invalid string format");
  }

  std::string binary;
  std::string number = s;

  while (!number.empty() && number != "0") {
    // Check if the number is even or odd
    if ((number.back() - '0') % 2 == 0) {
      binary += '0';
    } else {
      binary += '1';
    }

    // division by 2 on  number
    int carry = 0;
    for (char &c : number) {
      int digit = (c - '0') + 10 * carry;
      carry = digit % 2;
      c = (digit / 2) + '0';
    }

    // Remove zeros from the left side of the number
    number.erase(0, number.find_first_not_of('0'));
  }

  int n = 0;

  for (int i = 0; i < binary.size(); i += 32) {
    std::string binaryPart = binary.substr(i, 32);
    std::reverse(binaryPart.begin(), binaryPart.end());
    uint64_t value = std::bitset<32>(binaryPart).to_ulong();
    tab.push_back(value);
  }

  isPositive = true;
}

bool operator==(Bignum const &x, Bignum const &y) {
  int sizeX = x.tab.size();
  int sizeY = y.tab.size();

  if ((!x.isPositive && y.isPositive) || (!y.isPositive && x.isPositive)) {
    return false;
  }

  int i = sizeX - 1;
  int j = sizeY - 1;

  // Compare the digits from the right side
  while (j >= 0) {
    if (i < 0 || x.tab[i] != y.tab[j]) {
      return false;
    }
    i--;
    j--;
  }

  // Check if x has any non-zero digits remaining
  while (i >= 0) {
    if (x.tab[i] != 0) {
      return false;
    }
    i--;
  }

  return true;
}

bool operator!=(Bignum const &x, Bignum const &y) { return (not(x == y)); }

bool operator<(Bignum const &x, Bignum const &y) {
  int sizeX = x.tab.size();
  int sizeY = y.tab.size();

  if (!x.isPositive && y.isPositive) {
    return true;
  }
  if (!y.isPositive && x.isPositive) {
    return false;
  }

  int i = sizeX - 1;
  int j = sizeY - 1;

  if (sizeX + 1 > sizeY + 1) {
    return false;
  }
  if (sizeX + 1 < sizeY + 1) {
    return true;
  }

  // Compare the digits from the right side
  while (j >= 0) {
    if (x.tab[i] < y.tab[j]) {
      return true;
    }
    if (x.tab[i] > y.tab[j]) {
      return false;
    }
    i--;
    j--;
  }

  // Check if x has any non-zero digits remaining
  while (i >= 0) {
    if (x.tab[i] != 0) {
      return false;
    }
    i--;
  }

  return false;
}

bool operator<=(Bignum const &x, Bignum const &y) {

  return (x < y) || (x == y);
}

bool operator>(Bignum const &x, Bignum const &y) { return (not(x <= y)); }

bool operator>=(Bignum const &x, Bignum const &y) {
  return (x > y) || (x == y);
}

Bignum adding(Bignum const &x, Bignum const &y) {
  uint64_t c = 0;
  uint64_t b = static_cast<uint64_t>(1) << 32;

  Bignum z;
  z.tab.erase(z.tab.begin());
  int n = x.tab.size();
  int m = y.tab.size();
  int max_size = std::max(n, m);

  for (int i = 0; i < max_size; ++i) {
    uint64_t temp = c;
    if (i < n)
      temp += x.tab[i];
    if (i < m)
      temp += y.tab[i];

    z.tab.push_back(temp % b);
    c = temp / b;
  }

  if (c != 0) {
    z.tab.push_back(c);
  }

  z.isPositive = true;

  return z;
}

Bignum sustraction(Bignum const &x, Bignum const &y) {
  uint64_t borrow = 0;
  uint64_t b = static_cast<uint64_t>(1) << 32;

  Bignum z;
  z.tab.erase(z.tab.begin());
  int n = x.tab.size();
  int m = y.tab.size();

  Bignum copyX(x);
  Bignum copyY(y);
  int NewsizeX = copyX.tab.size();
  int NewsizeY = copyY.tab.size();

  z.isPositive = true;
  int i;
  for (i = 0; i < NewsizeY; i++) {
    if (copyX.tab[i] < copyY.tab[i] + borrow) {
      z.tab.push_back((b + copyX.tab[i]) - (copyY.tab[i] + borrow));
      borrow = 1;
    } else {
      z.tab.push_back(copyX.tab[i] - (copyY.tab[i] + borrow));
      borrow = 0;
    }
  }

  // Copy the remaining elements from x...if there are any
  for (; i < NewsizeX; i++) { // start with the current value of x
    if (copyX.tab[i] < borrow) {
      z.tab.push_back(b + copyX.tab[i] - borrow);
      borrow = 1;
    } else {
      z.tab.push_back(copyX.tab[i] - borrow);
      borrow = 0;
    }
  }

  int j = z.tab.size() - 1;

  while (j > 0 && z.tab[j] == 0) {

    z.tab.pop_back();
    j--;
  }

  return z;
}

Bignum operator+(Bignum const &x, Bignum const &y) {

  Bignum new_Big("0");
  Bignum temp1(x);
  Bignum temp2(y);
  if (x.isPositive != y.isPositive) {
    temp1.isPositive = true;
    temp2.isPositive = true;
    if ((temp1 < temp2)) {
      new_Big.isPositive = y.isPositive;
      new_Big = sustraction(temp2, temp1);
      return new_Big;
    }
    new_Big.isPositive = x.isPositive;
    new_Big = sustraction(temp1, temp2);
    return new_Big;
  }
  new_Big = adding(temp1, temp2);

  new_Big.isPositive = x.isPositive;
  return new_Big;
}

Bignum operator-(Bignum const &x, Bignum const &y) {
  Bignum copyX(x);
  Bignum copyY(y);
  Bignum z;
  z.tab.erase(z.tab.begin());

  if (x == y) {
    return Bignum(0);
  }

  // Check if y is zero
  if (y == Bignum(0)) {
    return x;
  }

  // Check if x is zero
  if (x == Bignum(0)) {

    copyY.isPositive = !y.isPositive;
    return copyY;
  }

  if (x.isPositive && y.isPositive) {

    if (x > y) {

      z = sustraction(x, y);
    } else {

      z = sustraction(y, x);
      z.isPositive = false;
    }

  } else if (!x.isPositive && !y.isPositive) {
    // x and y are both negative
    z.isPositive = false;
    if (x > y) {
      z = sustraction(x, y);

    } else {

      z = sustraction(y, x);
    }
  } else if (x.isPositive && !y.isPositive) {
    // x is positive and y is negative
    Bignum Ycopy(y);
    Ycopy.isPositive = true;

    z = x + Ycopy;
  } else {
    // x is negative and y is positive

    Bignum Xcopy(x);
    Xcopy.isPositive = true;

    z = Xcopy + y;
    z.isPositive = false;
  }

  return z;
}

Bignum operator*(Bignum const &x, Bignum const &y) {
  Bignum z;
  // z.tab.erase(z.tab.begin());
  int t = x.tab.size() + y.tab.size();
  int n = x.tab.size();
  int m = y.tab.size();
  uint64_t b = static_cast<uint64_t>(1) << 32;
  uint64_t c;
  uint64_t temp;

  for (int i = 0; i < t; i++) {
    z.tab.push_back(0);
  }

  for (int i = 0; i < n; i++) {
    c = 0;
    for (int j = 0; j < m; j++) {
      temp = (uint64_t)z.tab[i + j] +
             ((uint64_t)x.tab[i] * (uint64_t)y.tab[j]) + c;
      z.tab[i + j] = temp % b;
      c = temp / b;
    }
    z.tab[i + m] += c;
  }

  z.isPositive = !(x.isPositive ^ y.isPositive);

  int j = z.tab.size() - 1;
  while (j > 0 && z.tab[j] == 0) {

    z.tab.pop_back();
    j--;
  }

  return z;
}

Bignum &Bignum::operator=(Bignum &&other) {
  if (this != &other) {
    this->tab = std::move(other.tab);
    this->isPositive = other.isPositive;

    // Clear the moved object's tab
    other.tab.clear();
  }
  return *this;
}

Bignum &Bignum::operator=(const Bignum &other) {
  if (this != &other) {
    // Perform deep copy of tab and isPositive
    tab = other.tab;
    isPositive = other.isPositive;
  }
  return *this;
}

Bignum::Bignum(uint64_t value) {
  isPositive = true;
  if (value == 0) {
    tab.push_back(0);
  } else {
    tab.push_back(value);
  }
}

Bignum operator<<(const Bignum &x, uint64_t n) {
  if (n == 0) {
    return Bignum(x);
  }

  if (x == Bignum(0)) {
    return Bignum(0);
  }
  Bignum copyX(x);

  Bignum result(0);
  uint64_t carry = 0;
  int block = n / 32, bit_supp = n % 32;
  for (int i = 0; i < copyX.tab.size() + block; i++) {
    result.tab.push_back(0);
  }

  for (unsigned i = 0; i < copyX.tab.size(); i++) {
    uint32_t shifted = (copyX.tab[i] << bit_supp) | carry;
    carry = copyX.tab[i] >> (32 - bit_supp);

    if (bit_supp == 0) {
      carry = 0;
    }
    result.tab[i + block] = shifted;
  }
  result.tab[block + copyX.tab.size()] = carry;

  int j = result.tab.size() - 1;
  while (j > 0 && result.tab[j] == 0) {

    result.tab.pop_back();
    j--;
  }

  result.isPositive = x.isPositive;
  return result;
}
Bignum operator>>(const Bignum &x, uint64_t n) {
  if (n == 0) {
    return Bignum(x);
  }

  if (x == Bignum(0)) {
    return Bignum(0);
  }
  Bignum copyX(x);
  Bignum result(0);
  uint64_t carry = 0;
  int block = n / 32, bit_supp = n % 32;
  for (int i = 0; i < copyX.tab.size() - block - 1; i++) {
    result.tab.push_back(0);
  }

  for (int i = copyX.tab.size() - 1; i >= block; i--) {
    result.tab[i - block] = copyX.tab[i] >> bit_supp | carry;
    carry = copyX.tab[i] << (32 - bit_supp);
    if (bit_supp == 0) {
      carry = 0;
    }
  }
  int j = result.tab.size() - 1;
  while (j > 0 && result.tab[j] == 0) {

    result.tab.pop_back();
    j--;
  }
  result.isPositive = x.isPositive;
  return result;
}

std::pair<Bignum, Bignum> division(Bignum const &x, Bignum const &y) {
  Bignum copyX(x);
  Bignum CopyY(y);
  Bignum r(x);
  Bignum q = Bignum(0);
  Bignum CopyCopyY(0);

  if (copyX < CopyY) {
    return std::make_pair(q, r);
  }
  if (copyX == CopyY) {
    return std::make_pair(Bignum(1), Bignum(0));
  }

  CopyCopyY = CopyY;
  while (CopyCopyY < copyX) {
    CopyCopyY = CopyCopyY << 1;
  }
  if (CopyCopyY > copyX) {
    CopyCopyY = CopyCopyY >> 1;
  }

  while (CopyCopyY >= CopyY) {
    q = q << 1;

    if (r >= CopyCopyY) {
      q = q + 1;
      r = r - CopyCopyY;
    }

    CopyCopyY = CopyCopyY >> 1;
  }
  return std::make_pair(q, r);
}

Bignum operator/(Bignum const &x, Bignum const &y) {

  Bignum copyX(x);
  Bignum copyY(y);

  std::pair<Bignum, Bignum> result = division(copyX, copyY);

  Bignum q = result.first;

  q.isPositive = !(x.isPositive ^ y.isPositive);

  return q;
}

Bignum operator%(Bignum const &x, Bignum const &y) {

  Bignum copyX(x);
  Bignum copyY(y);

  std::pair<Bignum, Bignum> result = division(copyX, copyY);

  Bignum r = result.second;

  r.isPositive = true;

  return r;
}

std::string to_Base10(Bignum const &x) {
  if (x == 0) {
    return "0";
  }

  Bignum copy(x);
  std::string result;
  std::vector<uint64_t> quotient;
  uint64_t remainder = 0;
  uint64_t divisor = 10;

  while (copy.tab.size() > 1 || copy.tab[0] > 0) {
    remainder = 0;

    for (int i = copy.tab.size() - 1; i >= 0; --i) {
      uint64_t temp =
          copy.tab[i] + remainder * (static_cast<uint64_t>(1) << 32);
      copy.tab[i] = temp / divisor;
      remainder = temp % divisor;
    }

    quotient.push_back(remainder);
    while (copy.tab.size() > 1 && copy.tab.back() == 0) {
      copy.tab.pop_back();
    }
  }

  for (int i = quotient.size() - 1; i >= 0; --i) {
    result += std::to_string(quotient[i]);
  }

  if (!copy.isPositive) {
    result = "-" + result;
  }

  return result;
}

Bignum modularExponentiation(const Bignum &x, const Bignum &e,
                             const Bignum &m) {
  if (e == Bignum(0))
    return Bignum(1);

  Bignum result(1);
  Bignum base(x % m);
  Bignum exponent(e);

  while (exponent > Bignum(0)) {
    if (exponent % Bignum(2) == Bignum(1)) {
      result = (result * base) % m;
    }
    base = (base * base) % m;
    exponent = exponent / Bignum(2);
  }

  return result;
}

Bignum pgcd(Bignum const &x, Bignum const &y) {

  Bignum copyX(x);
  Bignum copyY(y);

  if (copyY == Bignum(0)) {
    return copyX;
  }

  return pgcd(copyY, copyX % copyY);
}

Bignum inverseMod(const Bignum &a, const Bignum &b) { // an algo seen in ADA
  Bignum x0 = Bignum(1);
  Bignum y0 = Bignum(0);
  Bignum x1 = Bignum(0);
  Bignum y1 = Bignum(1);
  Bignum q0(0);
  Bignum r0(0);
  Bignum tempX(0);
  Bignum tempY(0);
  Bignum a0 = a, b0 = b;

  while (b0 != 0) {

    q0 = a0 / b0;
    r0 = a0 % b0;

    a0 = b0;
    b0 = r0;

    tempX = x0;
    x0 = x1;
    x1 = tempX - (q0 * x1);

    tempY = y0;
    y0 = y1;
    y1 = tempY - (q0 * y1);
  }

  return (x0 < Bignum(0)) ? x0 + b : x0;
}

bool isPrime(const Bignum &n) {
  if (n <= Bignum(1))
    return false;

  // Predefined set of bases for the Miller-Rabin primality test
  const std::vector<Bignum> bases = {Bignum(2), Bignum(3), Bignum(5), Bignum(7),
                                     Bignum(11)};

  // Perform Miller-Rabin primality test with the predefined bases
  for (const Bignum &base : bases) {
    Bignum result = modularExponentiation(base, n - Bignum(1), n);
    if (result != Bignum(1))
      return false;
  }

  return true; // n is probably prime
}

Bignum generateRandomPrime(int bits) {
  std::random_device rd;
  std::mt19937 gen(rd());

  Bignum prime;

  // Generate random bits for the Bignum number
  prime.tab.resize((bits + 31) / 32); // Adjust the size of the tab vector

  int k = 0;
  while (bits > 32) {

    int bitsp = 32;
    bits -= 32;
    std::uniform_int_distribution<uint32_t> dis(1, (1 << bitsp) - 1);
    prime.tab[k] = dis(gen);

    k++;
  }

  std::uniform_int_distribution<uint32_t> dis(1, (1 << bits) - 1);
  prime.tab[k] = dis(gen);

  // Set the least significant bit to 1 to ensure an odd number
  prime.tab[0] |= 1;

  if (prime % Bignum(2) == Bignum(0)) {
    prime = prime + Bignum(1);
  }

  // Make sure the generated number is prime

  while (!isPrime(prime)) {

    /*std::string base = to_Base10(prime);
    std::cout << base << std::endl;*/
    // Increment the number by 2 to preserve oddness and retest
    prime = prime + Bignum(2);
  }

  return prime;
}

std::pair<std::pair<Bignum, Bignum>, std::pair<Bignum, Bignum>>
genetrateKeys(int bits) {

  Bignum prime1 = generateRandomPrime(bits / 2);
  Bignum prime2 = generateRandomPrime(bits / 2);

  while (prime1 == prime2) {
    prime2 = generateRandomPrime(bits / 2);
  }

  Bignum n = prime1 * prime2;
  Bignum fi = (prime1 - 1) * (prime2 - 1);
  Bignum Public(65537);

  Bignum privat(inverseMod(Public, fi));

  return std::make_pair(std::make_pair(Public, n), std::make_pair(privat, n));
}

Bignum encode(std::string message) {
  Bignum asciiValues(0);
  asciiValues.tab.erase(asciiValues.tab.begin());

  std::string strin = "";
  std::string strin2 = "";
  int i = 0;

  for (auto it = message.begin(); it != message.end(); ++it) {
    strin2 += std::to_string(static_cast<int>(*it));

    // Check if the accumulated string exceeds 9 digits
    if (strin2.size() > 9) {
      Bignum neww(strin);

      asciiValues.tab.push_back(
          neww.tab[0]); // Add the value to the end of the vector

      neww.tab.erase(neww.tab.begin());

      strin2 = "";
      strin = "";
      --it;
      i++;
    } else {

      strin += std::to_string(static_cast<int>(*it));
    }
  }

  if (!strin2.empty()) {
    Bignum neww(strin2);

    asciiValues.tab.push_back(
        neww.tab[0]); // Add the value to the end of the vector
  }

  return asciiValues;
}

std::string decode(const Bignum &message) {
  std::string result;
  std::string asciiValueStr; // To store digits to form ASCII value

  for (int asciiValue : message.tab) {
    asciiValueStr +=
        std::to_string(asciiValue); // Append the current value to the string

    // Check if the accumulated string forms a valid ASCII value
    while (asciiValueStr.length() >= 2) {
      int value = std::stoi(asciiValueStr.substr(0, 2)); // Extract 2 digits

      // Check if the value is within the desired range [32, 122]
      if (value >= 32 && value <= 122) {
        result += static_cast<char>(
            value); // Convert to character and append to result
        asciiValueStr = asciiValueStr.substr(2); // Remove processed digits
      } else {
        // The accumulated value does not fall within the valid range
        // Check for 3 digits
        if (asciiValueStr.length() >= 3) {
          value = std::stoi(asciiValueStr.substr(0, 3)); // Extract 3 digits
          if (value >= 32 && value <= 122) {
            result += static_cast<char>(
                value); // Convert to character and append to result
            asciiValueStr = asciiValueStr.substr(3); // Remove processed digits
          } else {
            // The 3-digit value is still not within the valid range
            throw std::runtime_error("Invalid ASCII value");
          }
        } else {
          // Not enough digits to form a valid ASCII value
          throw std::runtime_error("Invalid ASCII value");
        }
      }
    }
  }

  return result;
}

std::vector<Bignum> encrypt(std::string message, const Bignum &publicKey,
                            const Bignum &n) {
  std::vector<Bignum> ciphertexts;
  size_t i = 0;
  while (i < message.length()) {
    std::string part;
    Bignum encoded;
    do {
      if (i < message.length()) {
        part += message[i++];
        encoded = encode(part);
      }
    } while (encoded < n && i < message.length());

    if (encoded >= n) {
      // Remove the last character from 'part' and 'encoded'
      part.pop_back();
      encoded = encode(part);
      --i;
    }

    Bignum ciphertext = modularExponentiation(encoded, publicKey, n);
    ciphertexts.push_back(ciphertext);
  }
  return ciphertexts;
}

std::string decrypt(const std::vector<Bignum> &ciphertexts,
                    const Bignum &privateKey, const Bignum &n) {
  std::string message;
  for (const Bignum &ciphertext : ciphertexts) {
    Bignum decrypted = modularExponentiation(ciphertext, privateKey, n);
    message += decode(decrypted);
  }
  return message;
}

void printEncryptedMessage(const std::vector<Bignum> &ciphertexts) {
  for (const Bignum &ciphertext : ciphertexts) {

    std::cout << to_Base10(ciphertext);
  }

  std::cout << std::endl;
}

int main() {
  std::cout << "this program is an implementation of the RSA encryption "
               "progarm, here is a predefined example with a 20-bit key"
            << std::endl;

  std::string message1 = "Never go grocery shopping when you're hungry. You'll "
                         "grab the wrong things...";

  int siz = 20;
  std::cout << "message to encrypt = " << message1 << std::endl;

  std::pair<std::pair<Bignum, Bignum>, std::pair<Bignum, Bignum>> keys =
      genetrateKeys(siz);
  std::cout << "key size = " << siz << std::endl;

  std::vector<Bignum> cipher2 =
      encrypt(message1, keys.first.first, keys.first.second);

  std::cout << "Encrypted message= ";

  printEncryptedMessage(cipher2);

  std::string plientxt =
      decrypt(cipher2, keys.second.first, keys.second.second);
  std::cout << "decrypted message = " << plientxt << std::endl;

  std::cout << std::endl;

  while (true) {

    std::string message;
    int siz2;

    std::cout << "Enter the message that you wish to encrypt (or 0 to quit): "
              << std::endl;
    std::getline(std::cin, message);

    if (message == "0") {
      break;
    }

    std::cout << "Enter the key size [8-1024]: ";
    std::cin >> siz2;
    std::cin.ignore(); // clear the newline character from the input buffer

    if (siz2 <= 7) {
      std::cout << "Key size must be greater than 7." << std::endl;
      break;
    }

    std::cout << "message to encrypt = " << message << std::endl;

    std::cout << "keys generation... " << std::endl;

    std::pair<std::pair<Bignum, Bignum>, std::pair<Bignum, Bignum>> keys2 =
        genetrateKeys(siz2);
    std::cout << "key size = " << siz2 << std::endl;

    std::vector<Bignum> cipher =
        encrypt(message, keys.first.first, keys.first.second);

    std::cout << "Encrypted message= ";

    printEncryptedMessage(cipher);

    std::string plientxt2 =
        decrypt(cipher, keys.second.first, keys.second.second);
    std::cout << "decrypted message  = " << plientxt2 << std::endl;
    std::cout << std::endl;
  }

  return 0;
}
