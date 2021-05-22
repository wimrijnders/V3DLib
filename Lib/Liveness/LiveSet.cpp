#include "LiveSet.h"
#include "Support/Platform.h"
#include "Liveness.h"

namespace V3DLib {

///////////////////////////////////////////////////////////////////////////////
// Class LiveSet
///////////////////////////////////////////////////////////////////////////////

void LiveSet::add(LiveSet const &rhs) {
  insert(rhs.begin(), rhs.end());
}


void LiveSet::add(Set<RegId> const &set) {
  for (int j = 0; j < set.size(); j++) {
    insert(set[j]);
  }
}


/**
 * Remove the values in set 'use' from 'set' and store result in current instance
 */
void LiveSet::remove(LiveSet const &set, UseDef const &use ) {
  (*this) = set;

  for (int i = 0; i < use.def.size(); i++) {
    erase(use.def[i]);
  }
}


std::string LiveSet::dump() const {
  std::string ret;

  ret << "(";

  for (auto j : *this) {
    ret << j << ", ";
  }

  ret << ")";

  return ret;
}


bool LiveSet::member(RegId rhs) const {
  return find(rhs) != end();
}


///////////////////////////////////////////////////////////////////////////////
// Class LiveSets
///////////////////////////////////////////////////////////////////////////////

LiveSets::LiveSets(int size) : m_size(size) {
  assert(size > 0);
  m_sets = new LiveSet[size];
}


LiveSets::~LiveSets() {
  delete [] m_sets;
}


void LiveSets::init(Instr::List &instrs, Liveness &live) {
  LiveSet liveOut;

  for (int i = 0; i < instrs.size(); i++) {
    live.computeLiveOut(i, liveOut);
    useDefSet.set_used(instrs[i]);

    for (auto rx : liveOut) {
      for (auto ry : liveOut) {
        if (rx != ry) (*this)[rx].insert(ry);
      }

      for (int k = 0; k < useDefSet.def.size(); k++) {
        RegId rd = useDefSet.def[k];
        if (rd != rx) {
          (*this)[rx].insert(rd);
          (*this)[rd].insert(rx);
        }
      }
    }
  }
}


LiveSet &LiveSets::operator[](int index) {
  assert(index >=0 && index < m_size);
  return m_sets[index];
}


/**
 * Determine the available register in the register file, to use for variable 'index'.
 *
 * @param index  index of variable
 */
std::vector<bool> LiveSets::possible_registers(int index, RegUsage &alloc, RegTag reg_tag) {
  assert(reg_tag == REG_A || reg_tag == REG_B);

  const int NUM_REGS = Platform::size_regfile();
  std::vector<bool> possible(NUM_REGS);

  for (int j = 0; j < NUM_REGS; j++)
    possible[j] = true;

  LiveSet &set = (*this)[index];

  // Eliminate impossible choices of register for this variable
  for (auto j : set) {
    Reg neighbour = alloc[j].reg;
    if (neighbour.tag == reg_tag) possible[neighbour.regId] = false;
  }

  return possible;
}


/**
 * Debug function to output the contents of the possible-vector
 *
 * Returns a string of 0's and 1's for each slot in the possible-vector.
 * - '0' - in use, not available for assignment for variable with index 'index'.
 * - '1' - not in use, available for assignment
 *
 * This falls under the category "You probably don't need it, but when you need it, you need it bad".
 *
 * @param index - index value of current variable displayed. If `-1`, don't show. For display purposes only.
 */
void LiveSets::dump_possible(std::vector<bool> &possible, int index) {
  std::string buf = "possible: ";

  if (index >= 0) {
    if (index < 10) buf << "  ";
    else if (index < 100) buf << " ";

    buf << index;
  }
  buf << ": ";

  for (int j = 0; j < (int) possible.size(); j++) {
    buf << (possible[j]?"1":"0");
  }
  debug(buf.c_str());
}


/**
 * Find possible register in each register file
 */
RegId LiveSets::choose_register(std::vector<bool> &possible, bool check_limit) {
  assert(!possible.empty());
  RegId chosenA = -1;

  for (int j = 0; j < (int) possible.size(); j++)
    if (possible[j]) { chosenA = j; break; }

  if (check_limit && chosenA < 0) {
    error("LiveSets::choose_register(): register allocation failed, insufficient capacity", true);
  }

  return chosenA;
}


std::string LiveSets::dump() const {
  std::string ret;

  for (int j = 0; j < m_size; j++) {
    if (m_sets[j].empty()) continue;
    ret << j << ": " << m_sets[j].dump() << "\n";
  }

  return ret;
}

}  // namespace V3DLib
