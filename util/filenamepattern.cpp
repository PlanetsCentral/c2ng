/**
  *  \file util/filenamepattern.cpp
  */

#include <stdexcept>
#include "util/filenamepattern.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/string/string.hpp"
#include "util/translation.hpp"

// /*! \class FileNameMatcher
//     \brief File Name Pattern Matcher

//     This class provides a means of compiling and applying a file name
//     pattern. This default implementation provides the following rules:
//     - "*" matches any sequence of characters, including nothing at all
//     - "?" matches any single character
//     - "\" quotes the next character, i.e. "\*" matches a single star

//     This class is the internal back-end to FileNamePattern.

//     \todo We should optionally use system rules, i.e. fnmatch(3) on
//     Unix, respective Windows function on Windows. This requires a new
//     function under arch/, and a configure switch (maybe users prefer
//     uniform behaviour on all platforms?). This default implementation
//     does not implement any operating system's rules completely.

//     \todo Right now, this isn't particularily efficient. We use a
//     simple back-tracking matcher. */
class util::FileNamePattern::Impl {
 public:
    Impl();
    ~Impl();

    void compile(String_t arg);
    bool match(String_t arg) const;
    bool empty() const;
    bool hasWildcard() const;
    bool getFileName(String_t& out) const;

 private:
    enum OpType {
        MatchLiteral,         ///< Match the literal specified in the string argument.
        MatchAnyChar,         ///< Match exactly one arbitrary character.
        MatchAnyString        ///< Match any number, including zero, arbitrary characters.
    };
    struct Operation {
        OpType   type;
        String_t arg;

        Operation(OpType type, String_t arg)
            : type(type), arg(arg)
            { }
    };
    std::vector<Operation> m_operations;

    void addOp(OpType op, const String_t arg);
    bool matchRecursive(afl::charset::Utf8Reader arg, size_t matchPos) const;
};

// /** Construct blank object. */
util::FileNamePattern::Impl::Impl()
{
    // ex FileNameMatcher::FileNameMatcher
}

// /** Destroy. */
util::FileNamePattern::Impl::~Impl()
{
    // ex FileNameMatcher::~FileNameMatcher
}

// /** Compile pattern for later use. This builds an internal private
//     representation which simplifies and speeds up later matching.
//     \throw FileNamePatternException if the pattern is syntactically
//     invalid */
void
util::FileNamePattern::Impl::compile(String_t arg)
{
    // ex FileNameMatcher::compile
    /* Given well-formed UTF-8, this actually works without modification.
       UTF-8 never contains any meta-characters, and if a '\' quotes the
       initial character of a rune, the remaining characters will also be
       compiled into MatchLiteral. */
    m_operations.clear();

    size_t pos = 0;
    while (pos < arg.size()) {
        if (arg[pos] == '*') {
            addOp(MatchAnyString, String_t());
            ++pos;
        } else if (arg[pos] == '?') {
            addOp(MatchAnyChar, String_t());
            ++pos;
        } else if (arg[pos] == '\\') {
            ++pos;
            if (pos >= arg.size()) {
                throw std::runtime_error(_("Backslash must be followed by a character"));
            }
            addOp(MatchLiteral, String_t(1, arg[pos]));
            ++pos;
        } else {
            addOp(MatchLiteral, String_t(1, arg[pos]));
            ++pos;
        }
    }
}

// /** Match pattern.
//     \param arg file name
//     \return true iff arg matches the compiled pattern */
bool
util::FileNamePattern::Impl::match(String_t arg) const
{
    // ex FileNameMatcher::match
    return matchRecursive(afl::charset::Utf8Reader(afl::string::toBytes(arg), 0), 0);
}

bool
util::FileNamePattern::Impl::empty() const
{
    return m_operations.empty();
}

// /** Check whether we have a wildcard.
//     \return true if this is a wildcard, false if it is a proper file name. */
bool
util::FileNamePattern::Impl::hasWildcard() const
{
    // ex FileNameMatcher::hasWildcard
    // Due to optimisation, this is very simple:
    return !m_operations.empty()
        && (m_operations.size() != 1
            || m_operations.front().type != MatchLiteral);
}

// /** Get file name.
//     \pre !hasWildcard() */
bool
util::FileNamePattern::Impl::getFileName(String_t& out) const
{
    // ex FileNameMatcher::getFileName (reworked)
    if (m_operations.empty()) {
        out.clear();
        return true;
    } else if (m_operations.size() == 1 && m_operations.front().type == MatchLiteral) {
        out = m_operations.front().arg;
        return true;
    } else {
        return false;
    }
}

// /** Add operation. This does a few trivial optimisations. The goal is
//     to speed up matching ("***x" takes cubic time without
//     optimisation, linear time with), and to otherwise simplify
//     handling. */
void
util::FileNamePattern::Impl::addOp(OpType op, const String_t arg)
{
    // ex FileNameMatcher::addOp
    /* attempt to optimize */
    if (!m_operations.empty()) {
        Operation& lastOp = m_operations.back();
        switch (op) {
         case MatchLiteral:
            if (lastOp.type == MatchLiteral) {
                /* two literals, as in "foo\*", compile into just one check */
                lastOp.arg += arg;
                return;
            }
            break;
         case MatchAnyString:
            if (lastOp.type == MatchAnyString) {
                /* two stars, as in "foo**", equivalent to just "foo*" */
                return;
            }
            break;
         case MatchAnyChar:
            if (lastOp.type == MatchAnyString) {
                /* optimise "*?" into "?*" */
                lastOp.type = MatchAnyChar;
                m_operations.push_back(Operation(MatchAnyString, String_t()));
                return;
            }
            break;
        }
    }

    /* add it */
    m_operations.push_back(Operation(op, arg));
}

// /** Recursive matching. Check whether tail of input string matches
//     tail of pattern.
//     \param arg       [in] String to match
//     \param matchPos [in] Position in pattern to start matching at
//     \return true iff match ok

//     \todo We could recognize some common patterns to speed up
//     matching, e.g. "*x" = directly compare string end, "*x*" = use
//     a single s.find() call. */
bool
util::FileNamePattern::Impl::matchRecursive(afl::charset::Utf8Reader arg, size_t matchPos) const
{
    // ex FileNameMatcher::matchRecursive
    while (matchPos < m_operations.size()) {
        const Operation& me = m_operations[matchPos];
        switch (me.type) {
         case MatchLiteral:
            if (arg.getRemainder().size() < me.arg.size()) {
                /* pattern longer than string */
                return false;
            }
            if (afl::string::strCaseCompare(afl::string::fromBytes(arg.getRemainder().subrange(0, me.arg.size())), me.arg) != 0) {
                /* pattern does not match string */
                return false;
            }
            /* success */
            arg = afl::charset::Utf8Reader(arg.getRemainder().subrange(me.arg.size()), 0);
            ++matchPos;
            break;

         case MatchAnyChar:
            if (!arg.hasMore()) {
                /* pattern longer than string */
                return false;
            }
            arg.eat();
            ++matchPos;
            break;

         case MatchAnyString:
            /* short-cut: "foo*" */
            if (matchPos + 1 == m_operations.size()) {
                return true;
            }
            /* simple, slow way */
            /* FIXME: possible optimisations:
               - optimise "*foo*" to use strstr / find / ...
               - optimise "*foo" to just look at the end of the text */
            while (1) {
                if (matchRecursive(arg, matchPos+1)) {
                    return true;
                }
                if (!arg.hasMore()) {
                    return false;
                }
                arg.eat();
            }
            break;
        }
    }

    return !arg.hasMore();
}



// /** Construct blank pattern. Call setPattern() before use. */
util::FileNamePattern::FileNamePattern()
    : m_pImpl(new Impl())
{
    // ex FileNamePattern::FileNamePattern
}

// /** Construct pattern.
//     \param name [in] Pattern
//     \see FileNameMatcher */
util::FileNamePattern::FileNamePattern(const String_t name)
    : m_pImpl(new Impl())
{
    // ex FileNamePattern::FileNamePattern
    m_pImpl->compile(name);
}

// /** Copy constructor. */
util::FileNamePattern::FileNamePattern(const FileNamePattern& other)
    : m_pImpl(new Impl(*other.m_pImpl))
{
    // ex FileNamePattern::FileNamePattern
}

// /** Assignment operator. */
util::FileNamePattern&
util::FileNamePattern::operator=(const FileNamePattern& other)
{
    *m_pImpl = *other.m_pImpl;
    return *this;
}

// /** Destructor. */
util::FileNamePattern::~FileNamePattern()
{
    // ex FileNamePattern::~FileNamePattern
}

// /** Set pattern. Discards the old one and sets a new one.
//     \param pattern [in] The new pattern */
void
util::FileNamePattern::setPattern(const String_t pattern)
{
    // ex FileNamePattern::setPattern
    m_pImpl->compile(pattern);
}

// /** Check whether we have a wildcard. If it is not a wildcard, it is a
//     proper file name which can be used directly (see getFileName()).
//     \return true if this is a wildcard, false if it is a proper file name. */
bool
util::FileNamePattern::hasWildcard() const
{
    // ex FileNamePattern::hasWildcard
    return m_pImpl->hasWildcard();
}

// /** Get file name. Note that this file name can differ from the name
//     used with the constructor in case quoting is used. For example,
//     with "foo\\*bar", hasWildcard() is true and getFileName() returns
//     "foo*bar"; the backslash quotes.
//     \pre hasWildcard() */
bool
util::FileNamePattern::getFileName(String_t& out) const
{
    // ex FileNamePattern::getFileName
    return m_pImpl->getFileName(out);
}

// /** Check for match.
//     \param filename [in] file name to test
//     \return true if filename matches pattern. */
bool
util::FileNamePattern::match(const String_t filename) const
{
    // ex FileNamePattern::match
    return m_pImpl->match(filename);
}

bool
util::FileNamePattern::empty() const
{
    return m_pImpl->empty();
}

// /** Get pattern that matches all files.
//     \post match(x) <=> x names a user-visible file */
String_t
util::FileNamePattern::getAllFilesPattern()
{
    // ex FileNamePattern::getAllFilesPattern
    return "*";
}

// /** Get pattern that matches all files with a particular extension.
//     \param ext [in] Extension, not including the dot */
String_t
util::FileNamePattern::getAllFilesWithExtensionPattern(String_t ext)
{
    // ex FileNamePattern::getAllFilesWithExtensionPattern
    return String_t("*.") + getSingleFilePattern(ext);
}

// /** Get pattern that matches a single file.
//     \param name [in] The name to match
//     \post match(x) <=> x and \c name name the same file */
String_t
util::FileNamePattern::getSingleFilePattern(String_t name)
{
    // ex FileNamePattern::getSingleFilePattern
    size_t n = 0;
    while ((n = name.find_first_of("\\?*", n)) != name.npos) {
        name.insert(n, "\\");
        n += 2;
    }
    return name;
}
