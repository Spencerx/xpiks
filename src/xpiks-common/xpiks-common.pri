win32 {
    CONFIG(debug, debug|release) {
        EXE_DIR = debug
    } else {
        EXE_DIR = release
    }
}

linux {
    EXE_DIR = .
}

DEPS_DIR = $$PWD/../xpiks-qt/deps

win32|linux {
    TR_DIR = translations

    exists($$OUT_PWD/$$EXE_DIR/$$TR_DIR/) {
        message("Translations dir exists")
    } else {
        createtranslations.commands += $(MKDIR) \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$TR_DIR)\"
        QMAKE_EXTRA_TARGETS += createtranslations
        POST_TARGETDEPS += createtranslations
    }

    AC_SOURCES_DIR = ac_sources

    exists($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR/) {
        message("ac_sources dir exists")
    } else {
        create_ac_sources.commands += $(MKDIR) \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR)\"
        QMAKE_EXTRA_TARGETS += create_ac_sources
        POST_TARGETDEPS += create_ac_sources
    }

    copywhatsnew.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/whatsnew.txt)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"
    copyterms.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/terms_and_conditions.txt)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"
    copydicts.commands = $(COPY_DIR) \"$$shell_path($$DEPS_DIR/dict)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"

    appveyor|travis-ci {
        copytranslations.commands = echo "Skip translations"
    } else {
        copytranslations.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/$$TR_DIR/)\"xpiks_*.qm \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$TR_DIR/)\"
    }

    copyfreqtables.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/$$AC_SOURCES_DIR/en_wordlist.tsv)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR/)\"

    QMAKE_EXTRA_TARGETS += copywhatsnew copyterms copydicts copytranslations copyfreqtables
    POST_TARGETDEPS += copywhatsnew copyterms copydicts copytranslations copyfreqtables
}
