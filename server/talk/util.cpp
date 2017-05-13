/**
  *  \file server/talk/util.cpp
  */

#include "server/talk/util.hpp"
#include "afl/string/char.hpp"

String_t
server::talk::simplifyUserName(const String_t s)
{
    String_t result;
    bool needsp = false;
    for (String_t::size_type i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (afl::string::charIsAlphanumeric(ch)) {
            if (needsp) {
                result += '_';
            }
            result += afl::string::charToLower(ch);
            needsp = false;
        } else {
            needsp = (result.size() != 0);
        }
    }
    return result;
}
