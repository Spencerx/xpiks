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

DEFINES += DEPS_DIR=$${DEPS_DIR}

win32|linux {
    TR_DIR = translations

    exists($$OUT_PWD/$$EXE_DIR/$$TR_DIR/) {
        message("Translations dir exists")
    } else {
        createtranslations.commands += $(MKDIR) \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$TR_DIR)\"
        QMAKE_EXTRA_TARGETS += createtranslations
        POST_TARGETDEPS += createtranslations
    }

    exists($$DEPS_DIR/$$TR_DIR/xpiks_*.qm) {
        message("Translations files exists")
        copytranslations.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/$$TR_DIR/)\"xpiks_*.qm \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$TR_DIR/)\"
    } else {
        message("Translations files don't exist, skipping")
        copytranslations.commands = echo "Skip translations"
    }

    AC_SOURCES_DIR = ac_sources

    exists($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR/) {
        message("ac_sources dir exists")
    } else {
        create_ac_sources.commands += $(MKDIR) \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR)\"
        QMAKE_EXTRA_TARGETS += create_ac_sources
        POST_TARGETDEPS += create_ac_sources
    }

    RECOVERTY_DIR = recoverty

    exists($$OUT_PWD/$$EXE_DIR/$$RECOVERTY_DIR/) {
        message("recoverty dir exists")
    } else {
        create_recoverty.commands += $(MKDIR) \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$RECOVERTY_DIR)\"
        QMAKE_EXTRA_TARGETS += create_recoverty
        POST_TARGETDEPS += create_recoverty
    }

    copywhatsnew.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/whatsnew.txt)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"
    copydicts.commands = $(COPY_DIR) \"$$shell_path($$DEPS_DIR/dict)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"
    copyfreqtables.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/$$AC_SOURCES_DIR/en_wordlist.tsv)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$AC_SOURCES_DIR/)\"
    copyrecoverty.commands = $(COPY_DIR) \"$$shell_path($$DEPS_DIR/$$RECOVERTY_DIR)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/$$RECOVERTY_DIR/)\"

    QMAKE_EXTRA_TARGETS += copywhatsnew copydicts copytranslations copyfreqtables copyrecoverty
    POST_TARGETDEPS += copywhatsnew copydicts copytranslations copyfreqtables copyrecoverty
}


