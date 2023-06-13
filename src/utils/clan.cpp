#include "alioth/utils/clan.h"

#include <stdexcept>

#include "alioth/utils/hex.h"

namespace alioth {

bool Clan::iterator::eoc() const { return index_ >= ref_.elements_.size(); }
Clan::iterator::iterator(Clan& ref, int index, Clan::number value)
    : ref_(ref), index_(index), value_(value) {}
Clan::iterator::iterator(const iterator& i)
    : ref_(i.ref_), index_(i.index_), value_(i.value_) {}
Clan::iterator::~iterator() {}
Clan::iterator& Clan::iterator::operator++() {
  if (value_ == 255) {
    index_ = ref_.elements_.size();
    return *this;
  }
  auto end = Clan::GetEnd(ref_.elements_[index_]);
  if (value_ == end) {
    index_ += 1;
    if (index_ < ref_.elements_.size()) {
      if (index_ < ref_.elements_.size()) {
        value_ = Clan::GetBegin(ref_.elements_[index_]);
      }
    }
  } else {
    value_ += 1;
  }

  return *this;
}
Clan::iterator Clan::iterator::operator++(int) {
  iterator an = *this;
  ++(*this);
  return an;
}
Clan::iterator& Clan::iterator::operator--() {
  if (value_ == 0) {
    index_ = ref_.elements_.size();
    return *this;
  }
  auto begin = Clan::GetBegin(ref_.elements_[index_]);
  if (value_ == begin) {
    index_ -= 1;
    if (index_ < ref_.elements_.size()) {
      if (index_ < ref_.elements_.size()) {
        value_ = Clan::GetEnd(ref_.elements_[index_]);
      }
    }
  } else {
    value_ += 1;
  }
  return *this;
}
Clan::iterator Clan::iterator::operator--(int) {
  iterator an = *this;
  --(*this);
  return an;
}
Clan::iterator::reference Clan::iterator::operator*() {
  if (ref_.IsEmpty() || index_ >= ref_.elements_.size())
    throw std::out_of_range("iterator");
  return value_;
}
bool Clan::iterator::operator==(const iterator& an) {
  return std::addressof(ref_) == std::addressof(an.ref_) &&
         ((eoc() && an.eoc()) || (!eoc() && !an.eoc() && value_ == an.value_));
}
bool Clan::iterator::operator!=(const iterator& an) {
  return !(*this == an);
}

Clan::Clan(number n) { Insert(n); }
Clan::Clan(number begin, number end) { Insert(begin, end); }
Clan::Clan(std::set<number> const& s) { Insert(s); }

bool Clan::operator==(Clan const& rhs) const {
  return elements_ == rhs.elements_;
}

bool Clan::operator!=(Clan const& rhs) const {
  return elements_ != rhs.elements_;
}

void Clan::Insert(number n) {
  auto const less = (n == 0 ? 0 : n - 1);
  auto const more = (n == 255 ? 255 : n + 1);

  if (elements_.empty()) {
    elements_.push_back(PointTo(n));
    return;
  }

  for (int i = 0; i < static_cast<int>(elements_.size()); i++) {
    auto& ele = elements_[i];
    if (Containes(ele, n)) return;

    const auto end = GetEnd(ele);
    if (less == end) {
      ele = UpperTo(ele, n);
      if (i + 1 < static_cast<int>(elements_.size())) {
        auto& next = elements_[i + 1];
        if (more >= GetBegin(next)) {
          ele = UpperTo(ele, GetEnd(next));
          elements_.erase(elements_.begin() + i + 1);
        }
      }
      return;
    } else if (end > n) {
      const auto begin = GetBegin(ele);
      if (more == begin) {
        ele = LowerTo(ele, n);
        return;
      } else {  // begin > more
        elements_.insert(elements_.begin() + i, PointTo(n));
        return;
      }
    } else {  // end < less
      if (i + 1 == static_cast<int>(elements_.size())) {
        elements_.push_back(PointTo(n));
        return;
      } else {
        continue;
      }
    }
  }
}

void Clan::Insert(number begin, number end) {
  if (begin > end) throw std::invalid_argument("begin > end");
  for (int i = begin; i <= end; i++) Insert(i);
}

void Clan::Insert(std::set<number> const& s) {
  for (auto n : s) Insert(n);
}

Clan& Clan::Merge(Clan const& rhs) {
  for (auto const& ele : rhs.elements_) {
    auto const begin = GetBegin(ele);
    auto const end = GetEnd(ele);
    if (begin == end) {
      Insert(begin);
    } else {
      Insert(begin, end);
    }
  }
  return *this;
}

void Clan::Remove(number n) {
  auto const i = Find(n);
  if (i < 0) return;

  auto& ele = elements_[i];
  auto const end = GetEnd(ele);
  auto const begin = GetBegin(ele);

  if (end == n) {
    if (begin == n) {
      elements_.erase(elements_.begin() + i);
    } else {
      ele = UpperTo(ele, n - 1);
    }
  } else if (begin == n) {
    ele = LowerTo(ele, n + 1);
  } else {
    elements_.insert(elements_.begin() + i + 1, LowerTo(ele, n + 1));
    ele = UpperTo(ele, n - 1);
  }
}

void Clan::Remove(std::set<number> const& s) {
  for (auto n : s) Remove(n);
}

bool Clan::Contains(number n) const { return Find(n) >= 0; }

std::set<Clan::number> Clan::GetChars() const {
  std::set<number> s;
  for (auto const& ele : elements_) {
    auto const end = GetEnd(ele);
    for (int i = GetBegin(ele); i <= end; i++) s.insert(i);
  }
  return s;
}

std::string Clan::Format() const {
  std::string s;
  for (auto const& ele : elements_) {
    auto const begin = GetBegin(ele);
    auto const end = GetEnd(ele);
    if (!s.empty()) {
      s.push_back(',');
    }
    if (begin == end) {
      s += Hex::Format(begin);
    } else {
      s += Hex::Format(begin);
      s.push_back('-');
      s += Hex::Format(end);
    }
  }

  return s;
}
Clan Clan::Parse(std::string const& s) {
  Clan notion;
  number begin = 0;
  number end = 0;
  int state = 1;

  auto p = s.data();
  do {
    const auto& c = *p;

    switch (state) {
      case 1: {
        if (Hex::IsHex(c)) {
          begin = Hex::Parse(c);
          state = 2;
        } else if (c == '\0') {
          state = 0;
        } else {
          state = -1;
        }
      } break;
      case 2: {
        if (Hex::IsHex(c)) {
          if (begin == 0) {
            state = -7;
          } else {
            begin = (begin << 4) + Hex::Parse(c);
            state = 3;
          }
        } else if (c == '\0') {
          if (end != 0 && end >= begin - 1)
            state = -2;
          else {
            notion.Insert(begin);
            state = 0;
          }
        } else if (c == '-') {
          if (end != 0 && end >= begin - 1)
            state = -2;
          else
            state = 4;
        } else if (c == ',') {
          if (end != 0 && end >= begin - 1) {
            state = -2;
          } else {
            end = begin;

            notion.Insert(begin);
            state = 1;
          }
        } else {
          state = -3;
        }
      } break;
      case 3: {
        if (c == '-') {
          if (end != 0 && end >= begin - 1)
            state = -2;
          else
            state = 4;
        } else if (c == '\0') {
          notion.Insert(begin);
          state = 0;
        } else if (c == ',') {
          if (end != 0 && end >= begin - 1) {
            state = -2;
          } else {
            end = begin;

            notion.Insert(begin);
            state = 1;
          }
        } else {
          state = -3;
        }
      } break;
      case 4: {
        if (Hex::IsHex(c)) {
          end = Hex::Parse(c);
          state = 5;
        } else {
          state = -5;
        }
      } break;
      case 5: {
        if (Hex::IsHex(c)) {
          if (end == 0) {
            state = -7;
          } else {
            end = (end << 4) + Hex::Parse(c);
            state = 6;
          }
        } else if (c == '\0') {
          if (end <= begin) {
            state = -6;
          } else {
            notion.Insert(begin, end);
            state = 0;
          }
        } else if (c == ',') {
          if (end <= begin) {
            state = -6;
          } else {
            notion.Insert(begin, end);
            state = 1;
          }
        } else {
          state = -3;
        }
      } break;
      case 6: {
        if (c == '\0') {
          if (end <= begin) {
            state = -6;
          } else {
            notion.Insert(begin, end);
            state = 0;
          }
        } else if (c == ',') {
          if (end <= begin) {
            state = -6;
          } else {
            notion.Insert(begin, end);
            state = 1;
          }
        } else {
          state = -3;
        }
      } break;
    }
  } while (state >= 0 && *p++ != '\0');

  if (state == -1) {
    throw std::runtime_error("Invalid notion: unexpected char before number");
  } else if (state == -2) {
    throw std::runtime_error("Invalid notion: illness order or splitted range");
  } else if (state == -3) {
    throw std::runtime_error("Invalid notion: unexpected char after number");
  } else if (state == -5) {
    throw std::runtime_error("Invalid notion: unexpected char after '-'");
  } else if (state == -6) {
    throw std::runtime_error(
        "Invalid notion: end of range is smaller than begin");
  } else if (state == -7) {
    throw std::runtime_error("Invalid notion: unexpected leading zero");
  } else if (state > 0) {
    throw std::runtime_error("Invalid notion: unexpected end");
  } else if (state != 0) {
    throw std::runtime_error("Invalid notion");
  }

  return notion;
}

int Clan::Find(number n) const {
  long left = 0;
  long right = elements_.size() - 1;

  while (left <= right) {
    auto const mid = (left + right) / 2;
    auto const& ele = elements_[mid];
    if (Containes(ele, n)) return mid;

    auto const end = GetEnd(ele);
    if (end < n) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }

  return -1;
}

size_t Clan::Size() const {
  size_t size = 0;
  for (auto const& e : elements_) {
    size += GetEnd(e) - GetBegin(e) + 1;
  }
  return size;
}

bool Clan::IsEmpty() const { return elements_.empty(); }

Clan::iterator Clan::begin() {
  if (IsEmpty()) return end();
  return iterator(*this, 0, GetBegin(elements_[0]));
}
Clan::iterator Clan::end() {
  return iterator(*this, elements_.size(), 0);
}

Clan::number Clan::GetBegin(element e) {
  if ((e & kMask[1]) == 0xFF00) {
    return static_cast<number>(e & kMask[0]);
  } else {
    return static_cast<number>((e & kMask[1]) >> kOffset[1]);
  }
}

Clan::number Clan::GetEnd(element e) {
  return static_cast<number>(e & kMask[0]);
}

bool Clan::Containes(element e, number n) {
  const auto end = GetEnd(e);
  if (end < n) return false;
  const auto begin = GetBegin(e);
  if (begin > n) return false;

  return true;
}

Clan::element Clan::UpperTo(element e, number end) {
  auto const begin = GetBegin(e);
  if (begin > end) throw std::runtime_error("Invalid range");

  if (begin == end) return PointTo(begin);
  return (begin << kOffset[1]) | (end & kMask[0]);
}

Clan::element Clan::LowerTo(element e, number begin) {
  auto const end = GetEnd(e);
  if (begin > end) throw std::runtime_error("Invalid range");

  if (begin == end) return PointTo(begin);
  return (begin << kOffset[1]) | (end & kMask[0]);
}

Clan::element Clan::PointTo(number n) { return 0xFF00 | (n & kMask[0]); }

std::vector<Clan::element> Clan::GetElements() const { return elements_; }

}  // namespace alioth