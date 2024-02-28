//
// Created by pierr on 26/03/2023.
//
#pragma once

#include "TextEditorLangDef.h"

#include <array>
#include <memory>
#include <map>
#include <regex>

#include "imgui.h"

namespace Astra::UI::Core {

    class TextEditor
    {
    public:
        enum class SelectionMode
        {
            Normal,
            Word,
            Line
        };

        struct Breakpoint
        {
            int mLine = -1;
            bool mEnabled = false;
            std::string mCondition;

            Breakpoint() = default;
        };

        using String = std::string;

        using ErrorMarkers = std::map<int, std::string>;
        using Breakpoints = std::unordered_set<int>;
        using Palette = std::array<ImU32, (unsigned int) Editor::PaletteIndex::Max>;
        using Char = uint8_t;

        struct Glyph
        {
            Char mChar;
            Editor::PaletteIndex mColorIndex = Editor::PaletteIndex::Default;
            bool mComment: 1;
            bool mMultiLineComment: 1;
            bool mPreprocessor: 1;

            Glyph(Char aChar, Editor::PaletteIndex aColorIndex)
                    : mChar(aChar), mColorIndex(aColorIndex),
                      mComment(false), mMultiLineComment(false), mPreprocessor(false) {}
        };

        using Line = std::vector<Glyph>;
        using Lines = std::vector<Line>;

        TextEditor();

        ~TextEditor() = default;

        void SetLanguageDefinition(const Editor::LanguageDefinition& aLanguageDef);

        const Editor::LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

        const Palette& GetPalette() const { return mPaletteBase; }

        void SetPalette(const Palette& aValue);

        void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }

        void SetBreakpoints(const Breakpoints& aMarkers) { mBreakpoints = aMarkers; }

        void Render(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);

        void SetText(const std::string& aText);

        std::string GetText() const;

        void SetTextLines(const std::vector<std::string>& aLines);

        std::vector<std::string> GetTextLines() const;

        std::string GetSelectedText() const;

        std::string GetCurrentLineText() const;

        int GetTotalLines() const { return (int) mLines.size(); }

        bool IsOverwrite() const { return mOverwrite; }

        void SetReadOnly(bool aValue);

        bool IsReadOnly() const { return mReadOnly; }

        bool IsTextChanged() const { return mTextChanged; }

        bool IsCursorPositionChanged() const { return mCursorPositionChanged; }

        bool IsColorizerEnabled() const { return mColorizerEnabled; }

        void SetColorizerEnable(bool aValue);

        Editor::Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }

        void SetCursorPosition(const Editor::Coordinates& aPosition);

        inline void SetHandleMouseInputs(bool aValue) { mHandleMouseInputs = aValue; }

        inline bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetHandleKeyboardInputs(bool aValue) { mHandleKeyboardInputs = aValue; }

        inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

        inline void SetImGuiChildIgnored(bool aValue) { mIgnoreImGuiChild = aValue; }

        inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

        inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }

        inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

        void SetTabSize(int aValue);

        inline int GetTabSize() const { return mTabSize; }

        void InsertText(const std::string& aValue);

        void InsertText(const char* aValue);

        void MoveUp(int aAmount = 1, bool aSelect = false);

        void MoveDown(int aAmount = 1, bool aSelect = false);

        void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);

        void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);

        void MoveTop(bool aSelect = false);

        void MoveBottom(bool aSelect = false);

        void MoveHome(bool aSelect = false);

        void MoveEnd(bool aSelect = false);

        void SetSelectionStart(const Editor::Coordinates& aPosition);

        void SetSelectionEnd(const Editor::Coordinates& aPosition);

        void
        SetSelection(const Editor::Coordinates& aStart, const Editor::Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);

        void SelectWordUnderCursor();

        void SelectAll();

        bool HasSelection() const;

        void Copy();

        void Cut();

        void Paste();

        void Delete();

        bool CanUndo() const;

        bool CanRedo() const;

        void Undo(int aSteps = 1);

        void Redo(int aSteps = 1);

        static const Palette& GetDarkPalette();

        static const Palette& GetLightPalette();

        static const Palette& GetRetroBluePalette();

    private:
        using RegexList = std::vector<std::pair<std::regex, Editor::PaletteIndex>>;

        struct EditorState
        {
            Editor::Coordinates mSelectionStart;
            Editor::Coordinates mSelectionEnd;
            Editor::Coordinates mCursorPosition;
        };

        class UndoRecord
        {
        public:
            UndoRecord() = default;

            ~UndoRecord() = default;

            UndoRecord(
                    std::string aAdded,
                    const Editor::Coordinates& aAddedStart,
                    const Editor::Coordinates& aAddedEnd,
                    std::string aRemoved,
                    const Editor::Coordinates& aRemovedStart,
                    const Editor::Coordinates& aRemovedEnd,
                    const TextEditor::EditorState& aBefore,
                    const TextEditor::EditorState& aAfter);

            void Undo(TextEditor* aEditor) const;

            void Redo(TextEditor* aEditor) const;

            std::string mAdded;
            Editor::Coordinates mAddedStart;
            Editor::Coordinates mAddedEnd;

            std::string mRemoved;
            Editor::Coordinates mRemovedStart;
            Editor::Coordinates mRemovedEnd;

            EditorState mBefore;
            EditorState mAfter;
        };

        using UndoBuffer = std::vector<UndoRecord>;

        void ProcessInputs() const;

        void Colorize(int aFromLine = 0, int aCount = -1);

        void ColorizeRange(int aFromLine = 0, int aToLine = 0);

        void ColorizeInternal();

        float TextDistanceToLineStart(const Editor::Coordinates& aFrom) const;

        void EnsureCursorVisible();

        int GetPageSize() const;

        std::string GetText(const Editor::Coordinates& aStart, const Editor::Coordinates& aEnd) const;

        Editor::Coordinates GetActualCursorCoordinates() const;

        Editor::Coordinates SanitizeCoordinates(const Editor::Coordinates& aValue) const;

        void Advance(Editor::Coordinates& aCoordinates) const;

        void DeleteRange(const Editor::Coordinates& aStart, const Editor::Coordinates& aEnd);

        int InsertTextAt(Editor::Coordinates& aWhere, const char* aValue);

        void AddUndo(const UndoRecord& aValue);

        Editor::Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;

        Editor::Coordinates FindWordStart(const Editor::Coordinates& aFrom) const;

        Editor::Coordinates FindWordEnd(const Editor::Coordinates& aFrom) const;

        Editor::Coordinates FindNextWord(const Editor::Coordinates& aFrom) const;

        int GetCharacterIndex(const Editor::Coordinates& aCoordinates) const;

        int GetCharacterColumn(int aLine, int aIndex) const;

        int GetLineCharacterCount(int aLine) const;

        int GetLineMaxColumn(int aLine) const;

        bool IsOnWordBoundary(const Editor::Coordinates& aAt) const;

        void RemoveLine(int aStart, int aEnd);

        void RemoveLine(int aIndex);

        Line& InsertLine(int aIndex);

        void EnterCharacter(ImWchar aChar, bool aShift);

        void Backspace();

        void DeleteSelection();

        std::string GetWordUnderCursor() const;

        std::string GetWordAt(const Editor::Coordinates& aCoords) const;

        ImU32 GetGlyphColor(const Glyph& aGlyph) const;

        void HandleKeyboardInputs();

        void HandleMouseInputs();

        void Render();

        float mLineSpacing = 1.0f;
        Lines mLines;
        EditorState mState;
        UndoBuffer mUndoBuffer;
        int mUndoIndex = 0;

        int mTabSize = 4;
        bool mOverwrite = false;
        bool mReadOnly = false;
        bool mWithinRender = false;
        bool mScrollToCursor = false;
        bool mScrollToTop = false;
        bool mTextChanged = false;
        bool mColorizerEnabled = true;
        float mTextStart = 20.0f; // position (in pixels) where a code line starts relative to the left of the TextEditor
        int mLeftMargin = 10;
        bool mCursorPositionChanged = false;
        int mColorRangeMin = 0;
        int mColorRangeMax = 0;
        SelectionMode mSelectionMode = SelectionMode::Normal;
        bool mHandleKeyboardInputs = true;
        bool mHandleMouseInputs = true;
        bool mIgnoreImGuiChild = false;
        bool mShowWhitespaces = true;

        Palette mPaletteBase;
        Palette mPalette;
        Editor::LanguageDefinition mLanguageDefinition;
        RegexList mRegexList;

        bool mCheckComments = true;
        Breakpoints mBreakpoints;
        ErrorMarkers mErrorMarkers;
        ImVec2 mCharAdvance;
        Editor::Coordinates mInteractiveStart;
        Editor::Coordinates mInteractiveEnd;
        std::string mLineBuffer;
        uint64_t mStartTime = 0;

        float mLastClick = -1.0f;

        float
        renderLine(const ImVec2& contentSize, ImDrawList* drawList, float longest, const ImVec2& cursorScreenPos,
                   float scrollX, int lineNo, int lineMax);
    };
}