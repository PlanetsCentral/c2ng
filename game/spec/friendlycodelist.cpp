/**
  *  \file game/spec/friendlycodelist.cpp
  *  \brief Class game::spec::FriendlyCodeList
  */

#include "game/spec/friendlycodelist.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/parse.hpp"
#include "game/registrationkey.hpp"

namespace {
    /** Compare friendly codes.
        Alpha-numerical codes sort before those with other characters, and codes are sorted case-blind. */
    bool compareFriendlyCodes(const game::spec::FriendlyCode& a,
                              const game::spec::FriendlyCode& b)
    {
        const String_t as = a.getCode();
        const String_t bs = b.getCode();
        if (!as.empty() && !bs.empty()) {
            if (afl::string::charIsAlphanumeric(as[0]) && !afl::string::charIsAlphanumeric(bs[0])) {
                return true;
            }
            if (!afl::string::charIsAlphanumeric(as[0]) && afl::string::charIsAlphanumeric(bs[0])) {
                return false;
            }
        }
        int i = afl::string::strCaseCompare(as, bs);
        if (i == 0) {
            return as < bs;
        } else {
            return i < 0;
        }
    }

    /** Add extra friendly code, but avoid duplicates.
        The xtrafcode.txt file usually contains all the codes we already had in fcodes.cc,
        but with lower-quality meta-information.
        Thus, if a definition already exists, ignore the extra code. */
    void addExtraCode(game::spec::FriendlyCodeList& list, const String_t& code, afl::string::Translator& tx)
    {
        if (!list.findIndexByName(code).isValid()) {
            list.addCode(game::spec::FriendlyCode(code, "X,", tx));
        }
    }
}

// Default constructor.
game::spec::FriendlyCodeList::FriendlyCodeList()
    : m_data()
{ }

// Make sublist of some other list.
game::spec::FriendlyCodeList::FriendlyCodeList(const FriendlyCodeList& originalList, const FriendlyCode::Filter& f, const RegistrationKey& key)
    : m_data()
{
    // ex GFCodeList::GFCodeList(const GFCodeList& l, const GObject& o)
    // @change This one filters registration status; PCC2 doesn't.
    for (Iterator_t i = originalList.begin(); i != originalList.end(); ++i) {
        if ((*i)->worksOn(f) && (*i)->isPermitted(key)) {
            addCode(**i);
        }
    }
    sort();
}

// Destructor.
game::spec::FriendlyCodeList::~FriendlyCodeList()
{ }

// Get number of friendly codes.
size_t
game::spec::FriendlyCodeList::size() const
{
    return m_data.size();
}

// Get iterator to first friendly code.
game::spec::FriendlyCodeList::Iterator_t
game::spec::FriendlyCodeList::begin() const
{
    return m_data.begin();
}

// Get iterator to one-past-last friendly code.
game::spec::FriendlyCodeList::Iterator_t
game::spec::FriendlyCodeList::end() const
{
    return m_data.end();
}

// Access a friendly code by index.
const game::spec::FriendlyCode*
game::spec::FriendlyCodeList::at(size_t n) const
{
    // ex GFCodeList::operator[]
    if (n < m_data.size()) {
        return m_data[n];
    } else {
        return 0;
    }
}

// Get index, given a friendly code.
afl::base::Optional<size_t>
game::spec::FriendlyCodeList::findIndexByName(const String_t& fc) const
{
    Iterator_t it = findCodeByName(fc);
    if (it != end()) {
        return it - begin();
    } else {
        return afl::base::Nothing;
    }
}

// Look up friendly code by name.
game::spec::FriendlyCodeList::Iterator_t
game::spec::FriendlyCodeList::findCodeByName(const String_t& fc) const
{
    // ex GFCodeList::getFCodeByName
    Iterator_t i = begin();
    Iterator_t e = end();
    while (i != e && (*i)->getCode() != fc) {
        ++i;
    }
    return i;
}


// Add a friendly code.
void
game::spec::FriendlyCodeList::addCode(const FriendlyCode& code)
{
    // ex GFCodeList::addFCode
    m_data.pushBackNew(new FriendlyCode(code));
}

// Sort list in-place.
void
game::spec::FriendlyCodeList::sort()
{
    // ex GFCodeList::sort
    // FIXME: use stable_sort instead?
    m_data.sort(compareFriendlyCodes);
}

// Clear.
void
game::spec::FriendlyCodeList::clear()
{
    // ex GFCodeList::clear, GFCode::clearExtraFC
    m_data.clear();
}

// Load friendly code list from a file.
void
game::spec::FriendlyCodeList::load(afl::io::Stream& in, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex GFCodeList::loadFromFile, ccmain.pas:LoadFCodesFile
    const char LOG_NAME[] = "game.spec.fc";
    afl::io::TextFile tf(in);
    String_t line;
    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);
        if (line.empty() || line[0] == ';') {
            continue;
        }

        String_t::size_type p = line.find_first_of("=,");
        if (p == String_t::npos) {
            log.write(log.Error, LOG_NAME, in.getName(), tf.getLineNumber(), tx("missing delimiter"));
        } else if (line[p] == ',') {
            String_t fc = afl::string::strTrim(line.substr(0, p));
            line.erase(0, p+1);
            if (fc.length() > 3) {
                log.write(log.Warn, LOG_NAME, in.getName(), tf.getLineNumber(), tx("friendly code too long; truncated"));
                fc.erase(3);
            }
            try {
                addCode(FriendlyCode(fc, line, tx));
            }
            catch(std::exception& e) {
                log.write(log.Error, LOG_NAME, in.getName(), tf.getLineNumber(), e.what());
            }
        } else {
            // FIXME: additional assignments. These still need to be implemented.
        }
    }
    sort();
}


// Load extra friendly codes list.
void
game::spec::FriendlyCodeList::loadExtraCodes(afl::io::Stream& in, afl::string::Translator& tx)
{
    // ex GFCode::loadExtraFC
    uint8_t tmp[4096];
    String_t code;
    while (size_t n = in.read(tmp)) {
        for (size_t i = 0; i < n; ++i) {
            if (std::isspace(tmp[i])) {
                if (!code.empty()) {
                    addExtraCode(*this, code, tx);
                    code.clear();
                }
            } else {
                code += char(tmp[i]);
            }
        }
    }

    if (!code.empty()) {
        addExtraCode(*this, code, tx);
    }
}

// Pack friendly-code list into standalone info object.
void
game::spec::FriendlyCodeList::pack(Infos_t& out, const PlayerList& players, afl::string::Translator& tx) const
{
    for (Iterator_t it = begin(), e = end(); it != e; ++it) {
        if (const FriendlyCode* p = *it) {
            if (!(p->getFlags().contains(FriendlyCode::PrefixCode))) {
                out.push_back(Info(p->getCode(), p->getDescription(players, tx)));
            }
        }
    }
}

// Check whether the specified friendly code is numeric.
bool
game::spec::FriendlyCodeList::isNumeric(const String_t& fc, const HostSelection host)
{
    // ex GFCode::isNumeric
    // ex fcode.pas:IsNumerical
    String_t::size_type i = 0, end = fc.size();
    if (host.hasSpacePaddedFCodes()) {
        // Host allows space padding, so trim whitespace.
        while (end > 0 && fc[end-1] == ' ') {
            --end;
        }
        while (i < end && fc[i] == ' ') {
            ++i;
        }
        if (i == end) {
            return false;
        }
    } else {
        // Must be three digits
        if (fc.length() != 3) {
            return false;
        }
    }

    if (host.hasNegativeFCodes() && fc[i] == '-') {
        // Host allows sign, and we got one
        ++i;
        if (i == end) {
            return false;
        }
    }

    //  Remainder must be digits
    while (i < end) {
        if (fc[i] < '0' || fc[i] > '9') {
            return false;
        }
        ++i;
    }
    return true;
}

// Check whether a friendly code is a special code.
bool
game::spec::FriendlyCodeList::isSpecial(const String_t& fc, bool ignoreCase) const
{
    // ex fcode.pas:IsSpecial, GFCode::isExtraFC
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        const FriendlyCode& t = **i;
        bool match;
        if (t.getFlags().contains(FriendlyCode::UnspecialCode)) {
            // Never matches
            match = false;
        } else {
            const String_t& tc = t.getCode();
            if (t.getFlags().contains(FriendlyCode::PrefixCode)) {
                // Match prefix
                match = tc.size() <= fc.size()
                    && (ignoreCase
                        ? afl::string::strCaseCompare(fc.substr(0, tc.size()), tc) == 0
                        : fc.compare(0, tc.size(), tc) == 0);
            } else {
                // Match entire
                match = (ignoreCase
                         ? afl::string::strCaseCompare(fc, tc) == 0
                         : fc == tc);
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

// Check whether a friendly code is a universal minefield friendly code.
bool
game::spec::FriendlyCodeList::isUniversalMinefieldFCode(const String_t& fc, bool tolerant, const HostSelection host)
{
    // ex GFCode::isUniversalMinefieldFCode
    if (host.hasCaseInsensitiveUniversalMinefieldFCodes()) {
        tolerant = true;
    }

    return fc.size() == 3
        && (fc[0] == 'm' || (tolerant && fc[0] == 'M'))
        && (fc[1] == 'f' || (tolerant && fc[1] == 'F'));
}

// Get friendly code's numeric value.
int
game::spec::FriendlyCodeList::getNumericValue(const String_t& fc, const HostSelection host)
{
    // ex GFCode::getNumericValue
    int n = 0;
    if (isNumeric(fc, host) && afl::string::strToInteger(fc, n)) {
        return n;
    } else {
        return 1000;
    }
}

// Check whether a friendly code is permitted as random friendly code.
bool
game::spec::FriendlyCodeList::isAllowedRandomCode(const String_t& fc, const HostSelection host) const
{
    // ex GFCode::isAllowedRandom
    return fc.length() == 3
        && !isUniversalMinefieldFCode(fc, true, host)
        && fc[0] != 'X' && fc[0] != 'x'
        && fc[1] != fc[0] && fc[2] != fc[0] && fc[1] != fc[2]
        && fc[0] != '#' && fc[1] != '#' && fc[2] != '#'
        && fc[0] != '?' && fc[1] != '?' && fc[2] != '?'
        && !isSpecial(fc, true)
        && !isNumeric(fc, host);
}

// Generate a random friendly code.
String_t
game::spec::FriendlyCodeList::generateRandomCode(util::RandomNumberGenerator& rng, const HostSelection host) const
{
    // ex GFCode::generateRandomFCode()
    // ex fcode.pas:RandomFCode
    // To guarantee termination, this function bails out if it did not find a good enough code after a while.
    // I have never seen this happen in real life.
    // However, lusers can configure their system to trigger the termination guard
    // (by defining every numeric character special in xtrafcode.txt), so we prefer degradation over crash.
    int paranoiaCounter = 200;
    String_t rv(3, ' ');
    do {
        rv[0] = static_cast<char>(33 + rng(90));
        rv[1] = static_cast<char>(33 + rng(90));
        rv[2] = static_cast<char>(33 + rng(90));
    } while (!isAllowedRandomCode(rv, host) && --paranoiaCounter);
    return rv;
}

// Check permission to use friendly code.
bool
game::spec::FriendlyCodeList::isAcceptedFriendlyCode(const String_t& fc, const FriendlyCode::Filter& f, const RegistrationKey& key, DefaultAcceptance dflt) const
{
    const Iterator_t fci = findCodeByName(fc);
    if (fci != end()) {
        return (*fci)->isPermitted(key)
            && (*fci)->worksOn(f);
    } else {
        return dflt == DefaultAvailable
            || (dflt == DefaultRegistered && key.getStatus() != RegistrationKey::Unregistered);
    }
}
