//
// Created by pierr on 06/08/2023.
//
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

namespace Astra::UI::Core::Editor {

    enum class PaletteIndex
    {
        Default,
        Keyword,
        Number,
        String,
        CharLiteral,
        Punctuation,
        Preprocessor,
        Identifier,
        KnownIdentifier,
        PreprocIdentifier,
        Comment,
        MultiLineComment,
        Background,
        Cursor,
        Selection,
        ErrorMarker,
        Breakpoint,
        LineNumber,
        CurrentLineFill,
        CurrentLineFillInactive,
        CurrentLineEdge,
        Max
    };

    // Represents a character coordinate from the user's point of view,
    // i. e. consider an uniform grid (assuming fixed-width font) on the
    // screen as it is rendered, and each cell has its own coordinate, starting from 0.
    // Tabs are counted as [1..mTabSize] count empty spaces, depending on
    // how many space is necessary to reach the next tab stop.
    // For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
    // because it is rendered as "    ABC" on the screen.
    struct Coordinates
    {
        int mLine = 0;
        int mColumn = 0;

        Coordinates() = default;

        Coordinates(int aLine, int aColumn)
                : mLine(aLine), mColumn(aColumn) {
            assert(aLine >= 0);
            assert(aColumn >= 0);
        }

        static Coordinates Invalid() {
            static Coordinates invalid(-1, -1);
            return invalid;
        }

        bool operator==(const Coordinates& o) const {
            return
                    mLine == o.mLine &&
                    mColumn == o.mColumn;
        }

        bool operator!=(const Coordinates& o) const {
            return
                    mLine != o.mLine ||
                    mColumn != o.mColumn;
        }

        bool operator<(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn < o.mColumn;
        }

        bool operator>(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn > o.mColumn;
        }

        bool operator<=(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn <= o.mColumn;
        }

        bool operator>=(const Coordinates& o) const {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn >= o.mColumn;
        }
    };

    struct Identifier
    {
        Coordinates mLocation;
        std::string mDeclaration;
    };

    using Identifiers = std::unordered_map<std::string, Identifier>;
    using Keywords = std::unordered_set<std::string>;

    struct LanguageDefinition
    {
        using TokenRegexString = std::pair<std::string, PaletteIndex>;
        using TokenRegexStrings = std::vector<TokenRegexString>;

        using TokenizeCallback = bool (*)(const char* in_begin, const char* in_end, const char*& out_begin,
                                          const char*& out_end, PaletteIndex& paletteIndex);

        std::string mName;
        Keywords mKeywords;
        Identifiers mIdentifiers;
        Identifiers mPreprocIdentifiers;
        std::string mCommentStart;
        std::string mCommentEnd;
        std::string mSingleLineComment;
        char mPreprocChar = '#';
        bool mAutoIndentation = true;

        TokenizeCallback mTokenize = nullptr;

        TokenRegexStrings mTokenRegexStrings;

        bool mCaseSensitive = true;

        LanguageDefinition() = default;

        static const LanguageDefinition& CPlusPlus();

        static const LanguageDefinition& ASM();

        static const LanguageDefinition& HLSL();

        static const LanguageDefinition& GLSL();

        static const LanguageDefinition& C();

        static const LanguageDefinition& SQL();

        static const LanguageDefinition& AngelScript();

        static const LanguageDefinition& Lua();
    };

}
