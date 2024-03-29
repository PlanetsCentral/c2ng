/**
  *  \file util/doc/renderoptions.cpp
  *  \brief Class util::doc::RenderOptions
  */

#include <cstring>
#include "util/doc/renderoptions.hpp"
#include "util/string.hpp"

void
util::doc::RenderOptions::setSiteRoot(const String_t& s)
{
    m_siteRoot = s;
}

const String_t&
util::doc::RenderOptions::getSiteRoot() const
{
    return m_siteRoot;
}

void
util::doc::RenderOptions::setAssetRoot(const String_t& s)
{
    m_assetRoot = s;
}

const String_t&
util::doc::RenderOptions::getAssetRoot() const
{
    return m_assetRoot;
}

void
util::doc::RenderOptions::setDocumentRoot(const String_t& s)
{
    m_docRoot = s;
}

const String_t&
util::doc::RenderOptions::getDocumentRoot() const
{
    return m_docRoot;
}

void
util::doc::RenderOptions::setDocumentLinkSuffix(const String_t& s)
{
    m_docLinkSuffix = s;
}

const String_t&
util::doc::RenderOptions::getDocumentLinkSuffix() const
{
    return m_docLinkSuffix;
}

void
util::doc::RenderOptions::setDocumentId(const String_t& s)
{
    m_docId = s;
}

const String_t&
util::doc::RenderOptions::getDocumentId() const
{
    return m_docId;
}

String_t
util::doc::RenderOptions::transformLink(String_t s) const
{
    if (strStartsWith(s, "http:") || strStartsWith(s, "https:") || strStartsWith(s, "mailto:") || strStartsWith(s, "ftp:")
        || strStartsWith(s, "news:") || strStartsWith(s, "nntp:") || strStartsWith(s, "data:"))
    {
        // Verbatim
        return s;
    } else if (const char* p = strStartsWith(s, "site:")) {
        // Site URL ("site:foo", same as "$(html_CGI_RELROOT)foo" in a template)
        return m_siteRoot + p;
    } else if (const char* p = strStartsWith(s, "asset:")) {
        // Asset URL ("asset:foo")
        return m_assetRoot + p;
    } else if (strStartsWith(s, "#")) {
        // Fragment ("#frag")
        return s;
    } else {
        // Document link. Must preserve relative position of fragment.
        String_t frag;
        String_t::size_type p = s.find('#');
        if (p != String_t::npos) {
            frag = s.substr(p);
            s.erase(p);
        }
        if (const char* p = strStartsWith(s, "/")) {
            // Global document URL (e.g. "/pcc2-current/toc")
            return m_docRoot + p + m_docLinkSuffix + frag;
        } else {
            // Local document URL
            return m_docRoot + m_docId + "/" + s + m_docLinkSuffix + frag;
        }
    }
}
