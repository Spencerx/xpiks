/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FLAGS
#define FLAGS

#include <type_traits>
#include <QObject>
#include <cstdint>

#if defined(Q_OS_WIN32) && defined(Q_PROCESSOR_X86_32) && (_MSC_VER == 1800)
#define XPIKS_TYPED_ENUMS_WORKAROUND
#endif

namespace Common {
    typedef uint32_t flag_t;

#if !defined(XPIKS_TYPED_ENUMS_WORKAROUND)
    // visual studio 2013 bug

    template<typename FlagType>
    struct enable_bitmask_operators {
        static constexpr bool enable=false;
    };

    template<typename FlagType>
    typename std::enable_if<enable_bitmask_operators<FlagType>::enable, FlagType>::type
    operator|(FlagType a, FlagType b) {
        typedef typename std::underlying_type<FlagType>::type underlying;
        return static_cast<FlagType>(static_cast<underlying>(a) | static_cast<underlying>(b));
    }

    template<typename FlagType>
    typename std::enable_if<enable_bitmask_operators<FlagType>::enable, FlagType>::type
    operator|(FlagType a, typename std::underlying_type<FlagType>::type b) {
        typedef typename std::underlying_type<FlagType>::type underlying;
        return static_cast<FlagType>(static_cast<underlying>(a) | b);
    }

    template<typename FlagType>
    typename std::enable_if<enable_bitmask_operators<FlagType>::enable, typename std::underlying_type<FlagType>::type>::type
    operator~(FlagType a) {
        return ~(static_cast<typename std::underlying_type<FlagType>::type>(a));
    }

    template<typename FlagType>
    typename std::enable_if<enable_bitmask_operators<FlagType>::enable, FlagType>::type
    operator&(FlagType a, FlagType b) {
        typedef typename std::underlying_type<FlagType>::type underlying;
        return static_cast<FlagType>(static_cast<underlying>(a) & static_cast<underlying>(b));
    }

    template<typename FlagType>
    typename std::enable_if<enable_bitmask_operators<FlagType>::enable, FlagType>::type
    operator&(FlagType a, typename std::underlying_type<FlagType>::type b) {
        typedef typename std::underlying_type<FlagType>::type underlying;
        return static_cast<FlagType>(static_cast<underlying>(a) & b);
    }
#endif

    enum struct CombinedEditFlags: flag_t {
        None = 0,
        EditTitle = 1 << 0,
        EditDescription = 1 << 1,
        EditKeywords = 1 << 2,
        AppendKeywords = 1 << 3,
        Clear = 1 << 4,
        EditEverything = EditTitle | EditDescription | EditKeywords
    };

    enum struct SuggestionFlags: flag_t {
        None = 0,
        Title = 1 << 0,
        Description = 1 << 1,
        Keywords = 1 << 2,
        All = Title | Description | Keywords
    };

    enum struct SpellCheckFlags: flag_t {
        Title = 1 << 0,
        Description = 1 << 1,
        Keywords = 1 << 2,
        All = Title | Description | Keywords
    };

    enum struct KeywordReplaceResult: flag_t {
        Succeeded = 0,
        FailedIndex = 1,
        FailedDuplicate = 2,
        Unknown = 1 << 20
    };

    enum struct SearchFlags: flag_t {
        None = 0,
        CaseSensitive = 1 << 0,
        Description = 1 << 1,
        Title = 1 << 2,
        Keywords = 1 << 3,
        ReservedTerms = 1 << 4, // include reserved terms like "x:empty"
        AllTerms = 1 << 5, // all of the search terms included in the result
        Filepath = 1 << 6,
        WholeWords = 1 << 7,
        IncludeSpaces = 1 << 8, // includes spaces inside

        Metadata = Description | Title | Keywords,
        ExactKeywords = WholeWords | Keywords,
        MetadataCaseSensitive = Metadata | CaseSensitive,

        Everything = Metadata | Filepath | ReservedTerms,
        AllTermsEverything = Everything | AllTerms,
        AnyTermsEverything = Everything
    };

    enum struct DirectoryFlags: flag_t {
        None = 0,
        IsSelected = 1 << 0,
        IsAddedAsDirectory = 1 << 1,
        IsRemoved = 1 << 2
    };

    enum struct WarningFlags: flag_t {
        None = 0,
        SizeLessThanMinimum = 1 << 0,
        NoKeywords = 1 << 1,
        TooFewKeywords = 1 << 2,
        TooManyKeywords = 1 << 3,
        DescriptionIsEmpty = 1 << 4,
        DescriptionNotEnoughWords = 1 << 5,
        DescriptionTooBig = 1 << 6,
        TitleIsEmpty = 1 << 7,
        TitleNotEnoughWords = 1 << 8,
        TitleTooManyWords = 1 << 9,
        TitleTooBig = 1 << 10,
        SpellErrorsInKeywords = 1 << 11,
        SpellErrorsInDescription = 1 << 12,
        SpellErrorsInTitle = 1 << 13,
        ImageFileIsTooBig = 1 << 14,
        KeywordsInDescription = 1 << 15,
        KeywordsInTitle = 1 << 16,
        FilenameSymbols = 1 << 17,
        VideoFileIsTooBig = 1 << 18,
        VideoIsTooLong = 1 << 19,
        VideoIsTooShort = 1 << 20
    };

    enum struct WarningsCheckFlags: flag_t {
        None = 0,
        Keywords = 1 << 0,
        Title = 1 << 1,
        Description = 1 << 2,
        Spelling = 1 << 3,
        FileProperties = 1 << 4,
        Metadata = Keywords | Title | Description | Spelling,
        All = Metadata | FileProperties
    };

    enum struct WordAnalysisFlags: flag_t {
        None = 0,
        Spelling = 1 << 0,
        Stemming = 1 << 1,
        All = Spelling | Stemming
    };

    enum struct PluginNotificationFlags: Common::flag_t {
        None = 0,
        CurrentEditableChanged = 1 << 0,
        ActionUndone = 1 << 1,
        PresetsUpdated = 1 << 2
    };

    enum struct AddFilesFlags: Common::flag_t {
        FlagAutoFindVectors = 1 << 0,
        FlagIsFullDirectory = 1 << 1,
        FlagIsSessionRestore = 1 << 2,
        FlagAutoImport = 1 << 3,
        FlagIsRemoveUndo = 1 << 4
    };

#if !defined(XPIKS_TYPED_ENUMS_WORKAROUND)
    // visual studio 2013 bug

    template<>
    struct enable_bitmask_operators<CombinedEditFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<PluginNotificationFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<SuggestionFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<SpellCheckFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<KeywordReplaceResult> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<SearchFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<DirectoryFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<WarningFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<WarningsCheckFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<WordAnalysisFlags> {
        static constexpr bool enable = true;
    };

    template<>
    struct enable_bitmask_operators<AddFilesFlags> {
        static constexpr bool enable = true;
    };
#else
#define ENUM_OR(ENUM_TYPE) inline ENUM_TYPE operator | (ENUM_TYPE a, ENUM_TYPE b) { \
    using T = std::underlying_type_t <ENUM_TYPE>; \
    return (ENUM_TYPE)(static_cast<T>(a) | static_cast<T>(b)); \
}

#define ENUM_AND(ENUM_TYPE) inline ENUM_TYPE operator & (ENUM_TYPE a, ENUM_TYPE b) { \
    using T = std::underlying_type_t <ENUM_TYPE>; \
    return (ENUM_TYPE)(static_cast<T>(a) & static_cast<T>(b)); \
    } \
    inline ENUM_TYPE operator & (ENUM_TYPE a, std::underlying_type_t <ENUM_TYPE> b) { \
        using T = std::underlying_type_t <ENUM_TYPE>; \
        return (ENUM_TYPE)(static_cast<T>(a) & b); \
    }

#define ENUM_NOT(ENUM_TYPE) inline std::underlying_type_t <ENUM_TYPE> operator ~ (ENUM_TYPE a) { \
    using T = std::underlying_type_t <ENUM_TYPE>; \
    return ~(static_cast<T>(a)); \
}

    ENUM_OR(SuggestionFlags)
    ENUM_OR(CombinedEditFlags)
    ENUM_OR(SearchFlags)
    ENUM_OR(DirectoryFlags)
    ENUM_OR(WordAnalysisFlags)
    ENUM_OR(WarningFlags)

    ENUM_AND(CombinedEditFlags)
    ENUM_AND(SearchFlags)
    ENUM_AND(DirectoryFlags)
    ENUM_AND(WordAnalysisFlags)
    ENUM_AND(SpellCheckFlags)
    ENUM_AND(SuggestionFlags)
    ENUM_AND(WarningFlags)
    ENUM_AND(WarningsCheckFlags)
    ENUM_AND(PluginNotificationFlags)

    ENUM_NOT(DirectoryFlags)
    ENUM_NOT(SearchFlags)
    ENUM_NOT(CombinedEditFlags)
    ENUM_NOT(WarningFlags)
#endif

    // --------------------------------------------

    template<typename FlagValue, typename FlagType>
    constexpr bool HasFlag(FlagValue value, FlagType flag) {
        return (value & flag) == flag;
    }

    template<typename FlagValue, typename FlagType>
    void SetFlag(FlagValue &value, FlagType flag) {
        value = value | flag;
    }

    template<typename FlagValue, typename FlagType>
    void UnsetFlag(FlagValue &value, FlagType flag) {
        // Xpiks should not have any tricky operations
        // that might require unsettings some weird flags first
        //Q_ASSERT(HasFlag(value, flag));
        value = value & ~(flag);
    }

    template<typename FlagValue, typename FlagType>
    void ApplyFlag(FlagValue &value, bool applySwitch, FlagType flag) {
        if (applySwitch) {
            SetFlag(value, flag);
        } else {
            UnsetFlag(value, flag);
        }
    }
}

Q_DECLARE_METATYPE(Common::SpellCheckFlags)

#endif // FLAGS

