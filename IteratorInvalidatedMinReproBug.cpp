// This is a small code sample which is a cut down repro of the bug.
// Comment out the #define at line #38 below to see a working version.
// The flat_map was chosen to give a speed optimisation for a large in memory object.
//
// It should produce on console (using std::map instead of flatmap):
//     { 1, 0 } : ONE_ZERO
//     { 1, 1 } : ONE_ONE
//     { 1, 2 } : ONE_TWO
//     { 1, 3 } : one_three
//     { 1, 4 } : ONE_FOUR
//     { 2, 1 } : two_one
//     { 2, 2 } : two_two
//     { 2, 3 } : two_three
//     { 2, 4 } : two_four
//     { 3, 1 } : THREE_ONE
//     { 3, 2 } : THREE_TWO
//     { 3, 4 } : THREE_FOUR

#include "stdafx.h"

#include <vector>
#include <string>
#include <iostream>


using Stuff = std::string;
using FromTo = struct {
  int from;
  int to;
};
bool operator==(const FromTo& lhs, const FromTo& rhs) {
  return lhs.from == rhs.from && lhs.to == rhs.to;
}
bool operator<(const FromTo& lhs, const FromTo& rhs) {
  return lhs.from < rhs.from || (lhs.from == rhs.from && lhs.to < rhs.to);
}

#define BUG void
#if defined(BUG)
#include <boost/container/flat_map.hpp>
using Map = boost::container::flat_map<FromTo, Stuff>;
#else
#include <map>
using Map = std::map<FromTo, Stuff>;
#endif


using MapIt = Map::iterator;
using VectorOfMapIterators = std::vector<MapIt>;
using mysize_t = VectorOfMapIterators::size_type;

int main() {
  Map master = {
      {{2, 1}, "two_one"},  {{2, 2}, "two_two"},   {{2, 3}, "two_three"},
      {{1, 2}, "one_two"},  {{1, 3}, "one_three"}, {{1, 4}, "one_four"},
      {{2, 4}, "two_four"},
  };

  Map changes = {
      {{1, 1}, "ONE_ONE"},   {{1, 0}, "ONE_ZERO"},   {{1, 2}, "ONE_TWO"},
      {{1, 4}, "ONE_FOUR"},  {{3, 4}, "THREE_FOUR"}, {{3, 1}, "THREE_ONE"},
      {{3, 2}, "THREE_TWO"},
  };

  VectorOfMapIterators vec_iterators;
  vec_iterators.reserve(changes.size());

  // Can we apply all changes?
  for (const auto& change : changes) {
    vec_iterators.push_back(master.find(change.first));

    if (vec_iterators.back() != master.end() &&
        vec_iterators.back()->first == change.first &&
        vec_iterators.back()->second == change.second) {
      throw std::runtime_error("AlreadyUpdated");
    }
  }

  // We can apply (haven't thrown so go ahead...)
  mysize_t i = 0;
  for (const auto& change : changes) {
    auto& rIterator = vec_iterators[i];
    if (rIterator == master.end()) {
      // new so insert
      master.insert(change);
    } else {
      // update so change Stuff
      rIterator->second = change.second;
    }

    ++i;
  }

   // dump to console
   for (const auto& elem : master) {
     std::cout
     << "{ "
     << elem.first.from
     << ", "
     << elem.first.to
     << " } : "
     << elem.second
     << '\n';
  }

  return 0;
}
