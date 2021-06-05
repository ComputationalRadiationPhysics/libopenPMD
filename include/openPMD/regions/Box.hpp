#ifndef REGIONS_BOX_HPP
#define REGIONS_BOX_HPP

#include "Helpers.hpp"
#include "Point.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <iostream>
#include <tuple>
#include <vector>

namespace openPMD {
namespace Regions {

/** A D-dimensional box
 *
 * A box is described by two points, its lower bound (inclusive) and
 * upper bound (exclusive). If the lower bound not less than the upper
 * bound, the box is empty. Empty boxes are fine (similar to an empty
 * array).
 *
 * The dimension D needs to be known at compile time. @see NDBox
 */
template <typename T, std::size_t D> class Box;

template <typename T> class Box<T, 0> {
  bool is_full;

public:
  constexpr static std::size_t D = 0;

  typedef typename Point<T, D>::value_type value_type;
  typedef typename Point<T, D>::size_type size_type;

  /** Create empty box
   */
  Box() : is_full(false) {}

  Box(const Box &) = default;
  Box(Box &&) = default;
  Box &operator=(const Box &) = default;
  Box &operator=(Box &&) = default;

  template <typename U> Box(const Box<U, D> &b) : is_full(b.is_full) {}

  /** Create box from lower (inclusive) and upper (exclusive) bound
   */
  Box(const Point<T, D> &lo, const Point<T, D> &hi) : is_full(false) {}
  /** Create box holding a single point
   */
  explicit Box(const Point<T, D> &p) : is_full(true) {}

  // Predicates
  size_type ndims() const { return D; }
  bool empty() const { return !is_full; }
  Point<T, D> lower() const { return Point<T, D>(); }
  Point<T, D> upper() const { return Point<T, D>(); }
  Point<T, D> shape() const { return Point<T, D>(); }
  size_type size() const { return is_full; }

  // Shift and scale operators
  Box &operator>>=(const Point<T, D> &p) { return *this; }
  Box &operator<<=(const Point<T, D> &p) { return *this; }
  Box &operator*=(const Point<T, D> &p) { return *this; }
  Box operator>>(const Point<T, D> &p) const { return Box(*this); }
  Box operator<<(const Point<T, D> &p) const { return Box(*this); }
  Box operator*(const Point<T, D> &p) const { return Box(*this); }
  Box grown(const Point<T, D> &dlo, const Point<T, D> &dhi) const {
    return *this;
  }
  Box grown(const Point<T, D> &d) const { return *this; }
  Box grown(const T &d) const { return *this; }
  Box shrunk(const Point<T, D> &dlo, const Point<T, D> &dhi) const {
    return *this;
  }
  Box shrunk(const Point<T, D> &d) const { return *this; }
  Box shrunk(const T &d) const { return *this; }

  // Comparison operators
  friend bool operator==(const Box &b1, const Box &b2) {
    return b1.empty() == b2.empty();
  }
  friend bool operator!=(const Box &b1, const Box &b2) { return !(b1 == b2); }

  // Set comparison operators
  bool contains(const Point<T, D> &p) const { return !empty(); }
  friend bool isdisjoint(const Box &b1, const Box &b2) {
    return b1.empty() || b2.empty();
  }
  friend bool operator<=(const Box &b1, const Box &b2) {
    return !b1.empty() <= !b2.empty();
  }
  friend bool operator>=(const Box &b1, const Box &b2) { return b2 <= b1; }
  friend bool operator<(const Box &b1, const Box &b2) {
    return b1 <= b2 && b1 != b2;
  }
  friend bool operator>(const Box &b1, const Box &b2) { return b2 < b1; }
  bool issubset(const Box &b) const { return *this <= b; }
  bool issuperset(const Box &b) const { return *this >= b; }
  bool is_strict_subset(const Box &b) const { return *this < b; }
  bool is_strict_superset(const Box &b) const { return *this > b; }

  // Set operations
  friend Box bounding_box(const Box &b1, const Box &b2) {
    return b1.empty() && b2.empty() ? Box() : Box(Point<T, D>());
  }

  friend Box operator&(const Box &b1, const Box &b2) {
    if (!b1.empty() & !b2.empty())
      return Box(Point<T, D>());
    return Box();
  }
  friend Box intersection(const Box &b1, const Box &b2) { return b1 & b2; }

  friend bool operator==(const Box &b, const std::vector<Box> &bs) {
    // This assumes that the elements of bs are disjoint
    return b.empty() == bs.empty();
  }
  friend bool operator==(const std::vector<Box> &bs, const Box &b) {
    return b == bs;
  }
  friend bool operator!=(const Box &b, const std::vector<Box> &bs) {
    return !(b == bs);
  }
  friend bool operator!=(const std::vector<Box> &bs, const Box &b) {
    return !(bs == b);
  }

  friend std::vector<Box> operator-(const Box &b1, const Box &b2) {
    if (!b1.empty() > b2.empty())
      return std::vector<Box>{Box(Point<T, D>())};
    return std::vector<Box>{};
  }
  friend std::vector<Box> difference(const Box &b1, const Box &b2) {
    return b1 - b2;
  }

  friend std::vector<Box> operator|(const Box &b1, const Box &b2) {
    if (!b1.empty() | !b2.empty())
      return std::vector<Box>{Box(Point<T, D>())};
    return std::vector<Box>{};
  }
  friend std::vector<Box> setunion(const Box &b1, const Box &b2) {
    return b1 | b2;
  }

  friend std::vector<Box> operator^(const Box &b1, const Box &b2) {
    if (!b1.empty() ^ !b2.empty())
      return std::vector<Box>{Box(Point<T, D>())};
    return std::vector<Box>{};
  }
  friend std::vector<Box> symmetric_difference(const Box &b1, const Box &b2) {
    return b1 ^ b2;
  }

  /** Output a box
   */
  friend std::ostream &operator<<(std::ostream &os, const Box &b) {
    return os << "(" << b.is_full << ")";
  }
};

template <typename T, std::size_t D> class Box {
  Point<T, D> lo, hi;

public:
  typedef typename Point<T, D>::value_type value_type;
  typedef typename Point<T, D>::size_type size_type;

  /** Create empty box
   */
  Box() = default;

  Box(const Box &) = default;
  Box(Box &&) = default;
  Box &operator=(const Box &) = default;
  Box &operator=(Box &&) = default;

  template <typename U> Box(const Box<U, D> &b) : lo(b.lo), hi(b.hi) {}

  /** Create box from lower (inclusive) and upper (exclusive) bound
   */
  Box(const Point<T, D> &lo, const Point<T, D> &hi) : lo(lo), hi(hi) {}
  /** Create box holding a single point
   */
  explicit Box(const Point<T, D> &p) : lo(p), hi(p + T(1)) {}

  // Predicates
  size_type ndims() const { return D; }
  bool empty() const { return any(hi <= lo); }
  Point<T, D> lower() const { return lo; }
  Point<T, D> upper() const { return hi; }
  Point<T, D> shape() const { return max(hi - lo, T(0)); }
  size_type size() const { return product(shape()); }

  // Shift and scale operators
  Box &operator>>=(const Point<T, D> &p) {
    lo += p;
    hi += p;
    return *this;
  }
  Box &operator<<=(const Point<T, D> &p) {
    lo -= p;
    hi -= p;
    return *this;
  }
  Box &operator*=(const Point<T, D> &p) {
    lo *= p;
    hi *= p;
    return *this;
  }
  Box operator>>(const Point<T, D> &p) const { return Box(*this) >>= p; }
  Box operator<<(const Point<T, D> &p) const { return Box(*this) <<= p; }
  Box operator*(const Point<T, D> &p) const { return Box(*this) *= p; }
  Box grown(const Point<T, D> &dlo, const Point<T, D> &dup) const {
    if (empty())
      return Box();
    Box nb(*this);
    nb.lo -= dlo;
    nb.hi += dup;
    return nb;
  }
  Box grown(const Point<T, D> &d) const { return grown(d, d); }
  Box grown(const T &d) const { return grown(Point<T, D>::pure(d)); }
  Box shrunk(const Point<T, D> &dlo, const Point<T, D> &dup) const {
    return grown(-dlo, -dup);
  }
  Box shrunk(const Point<T, D> &d) const { return shrunk(d, d); }
  Box shrunk(const T &d) const { return shrunk(Point<T, D>::pure(d)); }

  // Comparison operators
  friend bool operator==(const Box &b1, const Box &b2) {
    if (b1.empty() && b2.empty())
      return true;
    if (b1.empty() || b2.empty())
      return false;
    return all(b1.lo == b2.lo && b1.hi == b2.hi);
  }
  friend bool operator!=(const Box &b1, const Box &b2) { return !(b1 == b2); }

  // Set comparison operators
  bool contains(const Point<T, D> &p) const {
    if (empty())
      return false;
    return all(p >= lo && p < hi);
  }
  friend bool isdisjoint(const Box &b1, const Box &b2) {
    return (b1 & b2).empty();
  }
  friend bool operator<=(const Box &b1, const Box &b2) {
    if (b1.empty())
      return true;
    if (b2.empty())
      return false;
    return all(b1.lo >= b2.lo && b1.hi <= b2.hi);
  }
  friend bool operator>=(const Box &b1, const Box &b2) { return b2 <= b1; }
  friend bool operator<(const Box &b1, const Box &b2) {
    return b1 <= b2 && b1 != b2;
  }
  friend bool operator>(const Box &b1, const Box &b2) { return b2 < b1; }
  bool issubset(const Box &b) const { return *this <= b; }
  bool issuperset(const Box &b) const { return *this >= b; }
  bool is_strict_subset(const Box &b) const { return *this < b; }
  bool is_strict_superset(const Box &b) const { return *this > b; }

  // Set operations
  friend Box bounding_box(const Box &b1, const Box &b2) {
    if (b1.empty())
      return b2;
    if (b2.empty())
      return b1;
    auto r = Box(min(b1.lo, b2.lo), max(b1.hi, b2.hi));
// Postcondition
#if REGIONS_DEBUG
    assert(b1 <= r && b2 <= r);
#endif
    return r;
  }

  friend Box operator&(const Box &b1, const Box &b2) {
    auto nlo = max(b1.lo, b2.lo);
    auto nhi = min(b1.hi, b2.hi);
    auto r = Box(nlo, nhi);
// Postcondition
#if REGIONS_DEBUG
    assert(r <= b1 && r <= b2);
#endif
    return r;
  }
  friend Box intersection(const Box &b1, const Box &b2) { return b1 & b2; }

  friend bool operator==(const Box &b, const std::vector<Box> &bs) {
    // This assumes that the elements of bs are disjoint
    size_type sz = 0;
    for (const Box &c : bs)
      sz += c.size();
    if (b.size() != sz)
      return false;
    for (const Box &c : bs)
      if (!(c <= b))
        return false;
    return true;
  }
  friend bool operator==(const std::vector<Box> &bs, const Box &b) {
    return b == bs;
  }
  friend bool operator!=(const Box &b, const std::vector<Box> &bs) {
    return !(b == bs);
  }
  friend bool operator!=(const std::vector<Box> &bs, const Box &b) {
    return !(bs == b);
  }

private:
  /** Split a box at a point, creating up to 2^D new boxes
   */
  void split(const Point<T, D> &p, std::vector<Box> &rs) const {
    assert(!empty());
#if REGIONS_DEBUG
    const auto old_rs_size = rs.size();
#endif
    for (int m = 0; m < (1 << D); ++m) {
      Point<T, D> newlo = lo, newhi = hi;
      bool is_inside = true;
      for (int d = 0; d < D; ++d) {
        const bool lohi = m & (1 << d);
        if (p[d] > lo[d] && p[d] < hi[d]) {
          if (!lohi)
            newhi[d] = p[d];
          else
            newlo[d] = p[d];
        } else {
          is_inside &= !lohi;
        }
      }
      if (is_inside)
        rs.push_back(Box(newlo, newhi));
    }
#if REGIONS_DEBUG
    // Postcondition
    size_type vol = 0;
    for (auto i = old_rs_size; i < rs.size(); ++i) {
      assert(!rs[i].empty());
      assert(rs[i] <= *this);
      vol += rs[i].size();
    }
    assert(vol == size());
    for (size_t i = 0; i < rs.size(); ++i)
      for (size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
  }

public:
  std::vector<Box> operator-(const Box &b) const {
    if (empty())
      return std::vector<Box>{};
    if (b.empty())
      return std::vector<Box>{*this};
    std::vector<Box> bs1;
    split(b.lo, bs1);
    std::vector<Box> bs2;
    for (const auto &b1 : bs1)
      b1.split(b.hi, bs2);
    std::vector<Box> rs;
    for (const auto &b2 : bs2) {
      assert(isdisjoint(b2, b) || b2 <= b);
      if (isdisjoint(b2, b))
        rs.push_back(b2);
    }
#if REGIONS_DEBUG
    // Postcondition
    size_type vol = 0;
    for (const auto &r : rs) {
      assert(!r.empty());
      assert(r <= *this && !(r <= b));
      vol += r.size();
    }
    assert(vol >= max(size_type(0), size() - b.size()) && vol <= size());
    for (size_t i = 0; i < rs.size(); ++i)
      for (size_t j = i + 1; j < rs.size(); ++j)
        assert(rs[i].isdisjoint(rs[j]));
#endif
    return rs;
  }
  friend std::vector<Box> difference(const Box &b1, const Box &b2) {
    return b1 - b2;
  }

  friend std::vector<Box> operator|(const Box &b1, const Box &b2) {
    auto rs = b1 - b2;
    if (!b2.empty())
      rs.push_back(b2);
#if REGIONS_DEBUG
    // Postcondition
    size_type vol = 0;
    for (const auto &r : rs) {
      assert(!r.empty());
      assert(r <= b1 || r <= b2);
      vol += r.size();
    }
    assert(vol >= b1.size() && vol <= b1.size() + b2.size());
    for (size_t i = 0; i < rs.size(); ++i)
      for (size_t j = i + 1; j < rs.size(); ++j)
        assert(isdisjoint(rs[i], rs[j]));
#endif
    return rs;
  }
  friend std::vector<Box> setunion(const Box &b1, const Box &b2) {
    return b1 | b2;
  }

  friend std::vector<Box> operator^(const Box &b1, const Box &b2) {
    auto rs = b1 - b2;
    auto rs1 = b2 - b1;
    // TODO: Avoid this concatenation
    rs.insert(rs.end(), rs1.begin(), rs1.end());
#if REGIONS_DEBUG
    // Postcondition
    size_type vol = 0;
    for (const auto &r : rs) {
      assert(!r.empty());
      assert((r <= b1) ^ (r <= b2));
      vol += r.size();
    }
    assert(vol >= abs(b1.size() - b2.size()) && vol <= b1size() + b2.size());
    for (size_t i = 0; i < rs.size(); ++i)
      for (size_t j = i + 1; j < rs.size(); ++j)
        assert(isdisjoint(rs[i], rs[j]));
#endif
    return rs;
  }
  friend std::vector<Box> symmetric_difference(const Box &b1, const Box &b2) {
    return b1 ^ b2;
  }

  /** Output a box
   */
  friend std::ostream &operator<<(std::ostream &os, const Box &b) {
    return os << "(" << b.lo << ":" << b.hi << ")";
  }
};

} // namespace Regions
} // namespace openPMD

namespace std {

template <typename T, std::size_t D>
struct equal_to<openPMD::Regions::Box<T, D>>;

template <typename T> struct equal_to<openPMD::Regions::Box<T, 0>> {
  constexpr bool operator()(const openPMD::Regions::Box<T, 0> &x,
                            const openPMD::Regions::Box<T, 0> &y) const {
    return x.empty() == y.empty();
  }
};

template <typename T, std::size_t D>
struct equal_to<openPMD::Regions::Box<T, D>> {
  constexpr bool operator()(const openPMD::Regions::Box<T, D> &x,
                            const openPMD::Regions::Box<T, D> &y) const {
    if (x.empty() && y.empty())
      return true;
    if (x.empty() || y.empty())
      return false;
    return openPMD::Regions::helpers::tuple_eq(
        make_tuple(x.lower(), x.upper()), make_tuple(y.lower(), y.upper()));
  }
};

template <typename T, std::size_t D> struct hash<openPMD::Regions::Box<T, D>> {
  constexpr size_t operator()(const openPMD::Regions::Box<T, D> &x) const {
    if (x.empty())
      return size_t(0xc9df21a36550a048ULL);
    hash<openPMD::Regions::Point<T, D>> h;
    return openPMD::Regions::helpers::hash_combine(h(x.lower()), h(x.upper()));
  }
};

template <typename T, std::size_t D> struct less<openPMD::Regions::Box<T, D>>;

template <typename T> struct less<openPMD::Regions::Box<T, 0>> {
  constexpr bool operator()(const openPMD::Regions::Box<T, 0> &x,
                            const openPMD::Regions::Box<T, 0> &y) const {
    return !x.empty() < !y.empty();
  }
};

template <typename T, std::size_t D> struct less<openPMD::Regions::Box<T, D>> {
  constexpr bool operator()(const openPMD::Regions::Box<T, D> &x,
                            const openPMD::Regions::Box<T, D> &y) const {
    if (x.empty() && y.empty())
      return false;
    if (x.empty())
      return true;
    if (y.empty())
      return false;
    return openPMD::Regions::helpers::tuple_lt(
        make_tuple(x.lower(), x.upper()), make_tuple(y.lower(), y.upper()));
  }
};

} // namespace std

#endif // #ifndef REGIONS_BOX_HPP