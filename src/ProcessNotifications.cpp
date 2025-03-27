// This file is part of ControlledAutoIndent.
// Copyright 2025 by Randall Joseph Fellmy <software@coises.com>, <http://www.coises.com/software/>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "CommonData.h"
#include "Host/SciLexer.h"
#include <set>


struct SyntaxInfo {
    enum IndentType { Text, CLike, Python } indentType = Text;
    std::set<int> styleComment;
    int styleBrace = 0;
    int styleBreak = 0;  // Use -1 for C-Like languages in which statements do not span lines when not terminated by a semicolon
    int styleColon = 0;
};

namespace {

// When a buffer is activated or the language is changed, updateIndentType sets data.syntaxInfo to point
// to the appropriate SyntaxInfo structure, or to 0 if custom indentation is specified for the (external) lexer.

const SyntaxInfo syntaxText { SyntaxInfo::Text, {}, 0, 0, 0 };

const SyntaxInfo syntaxC
    { SyntaxInfo::CLike, { SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC }, SCE_C_OPERATOR, SCE_C_OPERATOR, 0 };

const SyntaxInfo syntaxCSS
    { SyntaxInfo::CLike, { SCE_CSS_COMMENT }, SCE_CSS_OPERATOR, SCE_CSS_OPERATOR, 0 };

const SyntaxInfo syntaxJSON
    { SyntaxInfo::CLike, { SCE_JSON_LINECOMMENT, SCE_JSON_BLOCKCOMMENT }, SCE_JSON_OPERATOR, -1, 0 };

const SyntaxInfo syntaxJSP
    { SyntaxInfo::CLike, { SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC }, SCE_HJA_SYMBOLS, SCE_HJA_SYMBOLS, 0 };

const SyntaxInfo syntaxPerl
    { SyntaxInfo::CLike, { SCE_PL_COMMENTLINE }, SCE_PL_OPERATOR, SCE_PL_OPERATOR, 0 };

const SyntaxInfo syntaxPHP
    { SyntaxInfo::CLike, { SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE }, SCE_HPHP_OPERATOR, SCE_HPHP_OPERATOR, 0 };

const SyntaxInfo syntaxPwrshl
    { SyntaxInfo::CLike, { SCE_POWERSHELL_COMMENT }, SCE_POWERSHELL_OPERATOR, -1, 0 };

const SyntaxInfo syntaxPython
    { SyntaxInfo::Python, { SCE_P_COMMENTLINE, SCE_P_COMMENTBLOCK }, 0, 0, SCE_P_OPERATOR };

const SyntaxInfo syntaxRust
    { SyntaxInfo::CLike, { SCE_RUST_COMMENTBLOCK, SCE_RUST_COMMENTLINE, SCE_RUST_COMMENTBLOCKDOC, SCE_RUST_COMMENTLINEDOC },
      SCE_RUST_OPERATOR, SCE_RUST_OPERATOR, 0 };

const std::map<NPP::LangType, const SyntaxInfo*> languageSyntax = {
    {NPP::L_C          , &syntaxC     },
    {NPP::L_CPP        , &syntaxC     },
    {NPP::L_CS         , &syntaxC     },
    {NPP::L_GOLANG     , &syntaxC     },
    {NPP::L_JAVA       , &syntaxC     },
    {NPP::L_JS         , &syntaxC     },
    {NPP::L_JAVASCRIPT , &syntaxC     },
    {NPP::L_OBJC       , &syntaxC     },
    {NPP::L_SWIFT      , &syntaxC     },
    {NPP::L_TYPESCRIPT , &syntaxC     },
    {NPP::L_CSS        , &syntaxCSS   },
    {NPP::L_JSON       , &syntaxJSON  },
    {NPP::L_JSON5      , &syntaxJSON  },
    {NPP::L_JSP        , &syntaxJSP   },
    {NPP::L_PERL       , &syntaxPerl  },
    {NPP::L_PHP        , &syntaxPHP   },
    {NPP::L_POWERSHELL , &syntaxPwrshl},
    {NPP::L_PYTHON     , &syntaxPython},
    {NPP::L_RUST       , &syntaxRust  },
};

void updateIndentType() {
    NPP::LangType languageType;
    npp(NPPM_GETCURRENTLANGTYPE, 0, &languageType);

    if (languageType >= NPP::L_EXTERNAL) {
        data.syntaxInfo = &syntaxText;
        auto n = npp(NPPM_GETLANGUAGENAME, languageType, 0);
        if (n) {
            std::wstring languageName(n, 0);
            npp(NPPM_GETLANGUAGENAME, languageType, &languageName);
            NPP::ExternalLexerAutoIndentMode aim;
            if (npp(NPPM_GETEXTERNALLEXERAUTOINDENTMODE, languageName.data(), &aim)) {
                if (aim == NPP::ExternalLexerAutoIndentMode::Custom) data.syntaxInfo = 0;
                else if (aim == NPP::ExternalLexerAutoIndentMode::C_Like) data.syntaxInfo = &syntaxC;
            }
        }
    }
    else data.syntaxInfo = languageSyntax.contains(languageType) ? languageSyntax.at(languageType) : &syntaxText;
}


// LineType and the determineLineType function are used for C-Like languages

struct LineType {
    Scintilla::Position position = -1;
    bool empty     = false;  // The line contains only blanks and tabs, or is zero-length
    bool content   = false;  // The line contains characters that are not blanks, tabs or comments
    bool left      = false;  // The last character that isn't a blank, a tab or a comment is a left brace
    bool right     = false;  // The last character that isn't a blank, a tab, a semicolon or a comment is a right brace
    bool pure      = false;  // Nothing other than blanks or tabs preceeds the left or right brace
    bool statement = false;  // The last character that isn't a blank, a tab or a comment is a semi-colon
    bool incomplete() const { return content && !statement && !left && !right && data.syntaxInfo->styleBreak >= 0; }
};

inline LineType determineLineType(Scintilla::Line lineNumber) {
    if (lineNumber < 0 || lineNumber > sci.LineCount()) return {-1, true, false, false, false, false, false };
    Scintilla::Position start = sci.PositionFromLine(lineNumber);
    Scintilla::Position end = sci.LineEndPosition(lineNumber);
    if (start >= end) return {end, true, false, false, false, false, false };
    for (Scintilla::Position p = end - 1; p >= start; --p) {
        char c = sci.CharacterAt(p);
        if (c == ' ' || c == '\t') continue;
        int s = sci.StyleAt(p);
        if (data.syntaxInfo->styleComment.contains(s)) continue;
        switch (c) {
        case '{':
            if (s == data.syntaxInfo->styleBrace)
                return { p, false, true, true, false, sci.LineIndentPosition(lineNumber) == p ? true : false, false };
            break;
        case '}':
            if (s == data.syntaxInfo->styleBrace)
                return { p, false, true, false, true, sci.LineIndentPosition(lineNumber) == p ? true : false, false };
            break;
        case ';':
            if (s == data.syntaxInfo->styleBreak) {
                for (Scintilla::Position q = p - 1; q >= start; --q) {
                    c = sci.CharacterAt(q);
                    if (c == ' ' || c == '\t') continue;
                    s = sci.StyleAt(q);
                    if (data.syntaxInfo->styleComment.contains(s)) continue;
                    if (c == '}' && s == data.syntaxInfo->styleBrace)
                        return { q, false, true, false, true, sci.LineIndentPosition(lineNumber) == q ? true : false, false };
                    break;
                }
                return { p, false, true, false, false, false, true };
            }
        }
        return { p, false, true, false, false, false, false };
    }
    return {end, true, false, false, false, false, false };
}


// indentSame(reference, line) and indentMore(reference, line) indent the specified line to have either the same indentation,
// or one level more indentation, than the reference line.  When line is omitted, the line containing the cursor is indented,
// maintaining the cursor in the correct location and respecting the setting as to whether to retain following white space.

inline std::string getIndentString(Scintilla::Line reference) {
    Scintilla::Position cpMin = sci.PositionFromLine(reference);
    Scintilla::Position cpMax = sci.LineIndentPosition(reference);
    return cpMin == cpMax ? std::string() : sci.StringOfRange(Scintilla::Span(cpMin, cpMax));
}

inline void indentBy(const std::string& indentString, Scintilla::Line line = -1) {
    if (line < 0) {
        Scintilla::Position cp = sci.CurrentPos();
        Scintilla::Line     cl = sci.LineFromPosition(cp);
        Scintilla::Position li = sci.LineIndentPosition(cl);
        if (cp <= li) {
            if (!data.retainFollowingSpace) sci.SetLineIndentation(cl, 0);
            if (indentString.length()) sci.AddText(indentString.length(), indentString.data());
        }
        else {
            sci.SetLineIndentation(cl, 0);
            if (indentString.length()) sci.InsertText(sci.PositionFromLine(cl), indentString.data());
        }
        sci.ScrollCaret();
    }
    else {
        sci.SetLineIndentation(line, 0);
        if (indentString.length()) sci.InsertText(sci.PositionFromLine(line), indentString.data());
    }
}

inline void indentBy(int indentation, Scintilla::Line line = -1) {
    int tw = sci.TabWidth();
    indentBy(sci.UseTabs() ? std::string(indentation / tw, '\t') + std::string(indentation % tw, ' ')
                           : std::string(indentation, ' '), line);
}

inline void indentSame(Scintilla::Line reference, Scintilla::Line line = -1) {
    if (data.matchIndentation) indentBy(getIndentString(reference), line);
                          else indentBy(sci.LineIndentation(reference), line);
}

inline void indentMore(Scintilla::Line reference, Scintilla::Line line = -1) {
    if (data.matchIndentation) {
        std::string indentString = getIndentString(reference);
        bool indentBlanks = indentString.find_first_of(' ')  != std::string::npos;
        bool indentTabs   = indentString.find_first_of('\t') != std::string::npos;
        if      (indentTabs && !indentBlanks) return indentBy(indentString + "\t", line);
        else if (indentBlanks && !indentTabs) return indentBy(indentString + std::string(sci.TabWidth(), ' '), line);
    }
    indentBy(sci.LineIndentation(reference) + sci.TabWidth(), line);
}

}


// scnCharAdded is called whenever a character is typed.  This routine only acts on the last or only character from the Enter key.

void scnCharAdded(const Scintilla::NotificationData* scnp) {

    // Preliminaries -- collect some data, and return quickly if this character can't cause indentation adjustment

    if (!data.syntaxInfo || sci.Selections() > 1) return;
    if (sci.EOLMode() == Scintilla::EndOfLine::Cr ? scnp->ch != '\r' : scnp->ch != '\n') return;

    const SyntaxInfo&   syn     = *data.syntaxInfo;
    Scintilla::Position curPos  = sci.CurrentPos();
    Scintilla::Line     curLine = sci.LineFromPosition(curPos);
    if (curLine == 0) return;

    Scintilla::Line     prevLine = curLine - 1;
    Scintilla::Position prevStart = sci.PositionFromLine(prevLine);
    Scintilla::Position prevEnd = sci.LineEndPosition(prevLine);

    if (prevStart == prevEnd) return;  // Don't change indent if Enter was pressed at beginning of line

    bool trimPreviousLine = (data.removeTrailingSpace && curPos != sci.LineEndPosition(curLine))
                         || (data.trimEmptyLines && prevEnd == sci.LineIndentPosition(prevLine));

    // Python indentation

    if (syn.indentType == SyntaxInfo::Python) {
        if (data.pythonColon) {
            sci.SetSearchFlags(Scintilla::FindOption::RegExp | Scintilla::FindOption::Posix);
            sci.SetTargetRange(prevStart, prevEnd);
            auto posColon = sci.SearchInTarget(":[ \t]*(#|$)");
            if ((posColon >= 0) && (sci.StyleIndexAt(posColon) == syn.styleColon)) indentMore(prevLine);
            else indentSame(prevLine);
        }
        else indentSame(prevLine);
    }

    // C-like indentation

    else if (syn.indentType == SyntaxInfo::CLike) {

        auto prevType = determineLineType(prevLine);

        if (data.cBraces && prevType.pure) {
            if (prevType.right) {
                Scintilla::Position matchingPosition = sci.BraceMatch(prevType.position, 0);
                if (matchingPosition >= 0) indentSame(sci.LineFromPosition(matchingPosition), prevLine);
            }
            else if (prevType.left && data.cContinuation && syn.styleBreak >= 0) {
                Scintilla::Line n = prevLine - 1;
                if (determineLineType(n).incomplete()) {
                    while (--n >= 0 && determineLineType(n).incomplete());
                    indentSame(n + 1, prevLine);
                }
            }
            // positions may have changed -- simplest to get them all again
            curPos    = sci.CurrentPos();
            prevStart = sci.PositionFromLine(prevLine);
            prevEnd   = sci.LineEndPosition(prevLine);
            prevType  = determineLineType(prevLine);
        }

        if (prevType.left) {
            if (sci.CharacterAt(curPos) == '}') {
                if (data.cAddEmpty) {
                    Scintilla::EndOfLine eolMode = sci.EOLMode();
                    sci.InsertText(curPos, eolMode == Scintilla::EndOfLine::CrLf ? "\r\n"
                                         : eolMode == Scintilla::EndOfLine::Cr   ? "\r"
                                                                                 : "\n");
                    indentSame(prevLine, curLine + 1);
                }
            }
            if (data.cBraces) indentMore(prevLine);
                         else indentSame(prevLine);
        }

        else if (prevType.right) {
            Scintilla::Position matchingPosition = sci.BraceMatch(prevType.position, 0);
            if (matchingPosition < 0) indentSame(prevLine);
            else {
                Scintilla::Line n = sci.LineFromPosition(matchingPosition);
                if (data.cContinuation && syn.styleBreak >= 0) {
                    while (--n >= 0 && determineLineType(n).incomplete());
                    indentSame(n + 1);
                }
                else indentSame(n);
            }
        }

        else if (data.cContinuation && syn.styleBreak >= 0) {
            if (prevType.incomplete()) {
                if (determineLineType(prevLine - 1).incomplete()) indentSame(prevLine);
                                                             else indentMore(prevLine);
            }
            else if (prevType.statement) {
                Scintilla::Line n = prevLine - 1;
                while (n >= 0 && determineLineType(n).incomplete()) --n;
                indentSame(n + 1);
            }
            else indentSame(prevLine);
        }

        else indentSame(prevLine);

    }

    // Basic indentation

    else indentSame(prevLine);

    // Trim previous line if needed

    if (trimPreviousLine) {
        Scintilla::Position p = prevEnd - 1;
        for (; p >= prevStart; --p) {
            char c = sci.CharacterAt(p);
            if (c != ' ' && c != '\t') break;
        }
        Scintilla::Position n = prevEnd - (++p);
        if (n > 0) sci.DeleteRange(p, n);
    }

}


void bufferActivated() {
    updateIndentType();
}

void languageChanged(const NMHDR*) {
    updateIndentType();
}
