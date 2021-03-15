#include "basics.h"


/**
 * Source: https://thispointer.com/find-and-replace-all-occurrences-of-a-sub-string-in-c/
 */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr) {
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while( pos != std::string::npos) {
      // Replace this occurrence of Sub String
      data.replace(pos, toSearch.size(), replaceStr);
      // Get the next occurrence from the current position
      pos =data.find(toSearch, pos + replaceStr.size());
    }
}


std::string tabbed(int tab_size, std::string const &val) {
  std::string ret;
  for (int i = (int) val.size(); i < tab_size; ++i ) {
    ret << " ";
  }

  ret << val;
  return ret;
}


std::string tabbed(int tab_size, int val) {
  std::string tmp;
  tmp << val;
  return tabbed(tab_size, tmp);
}
