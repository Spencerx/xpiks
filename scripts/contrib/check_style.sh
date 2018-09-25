#!/bin/bash

###############################################
#### Find all members not starting with m_ ####
###############################################

SRC_DIR=${1}
TESTS_DIR=${1}/xpiks-tests
XPIKS_QT_DIR=${1}/xpiks-qt
DEPLOYMENT_DIR=${XPIKS_QT_DIR}/deployment
TAGS_FILE=all.tags
BAD_TAGS_FILE=bad.tags

rm ${TAGS_FILE}

find "$SRC_DIR" -type f \( -name '*.h' -and ! -name 'constants.h' -and ! -name 'flags.h' \) | ctags -L - -f - --c++-kinds=-cgtufendsv --fields=ksm --format=2 --language-force=C++ >> ${TAGS_FILE}

find "$SRC_DIR" -type f \( -name '*.cpp' -and ! -name 'moc_*.cpp' -and ! -name 'qrc_*.cpp' -and ! -name 'aes-qt.cpp' -and ! -name 'qmlcache_loader.cpp' \) | ctags -L - -f - --c++-kinds=-cgtufendsv --fields=ksm --format=2 --language-force=C++ >> ${TAGS_FILE}

cat ${TAGS_FILE} | grep -v '^override' | grep -v 'm_' | grep -v ',$/;' > ${BAD_TAGS_FILE}

LINES_COUNT=`cat ${BAD_TAGS_FILE} | wc -l | tr -d '[:space:]'`

if [ $LINES_COUNT -gt 0 ]; then
    echo "Detected members that do not start with m_"
    cat ${BAD_TAGS_FILE}
    exit 1
fi

###############################
##### Find too long lines #####
###############################

LONG_LINES="long.lines"

rm ${LONG_LINES}

find "$SRC_DIR" -path "$TESTS_DIR" -prune -o -type f \( -name '*.cpp' -and ! -name 'moc_*.cpp' -and ! -name 'qrc_*.cpp' \) -exec grep -n '.\{180\}' {} /dev/null \; >> ${LONG_LINES}

find "$SRC_DIR" -path "$TESTS_DIR" -prune -o -type f -name '*.h' -exec grep -n '.\{180\}' {} /dev/null \; >> ${LONG_LINES}

find "$XPIKS_QT_DIR" -path "$DEPLOYMENT_DIR" -prune -o -type f \( -name '*.qml' -and ! -name 'TermsAndConditionsDialog.qml' \) -exec grep -n '.\{180\}' {} /dev/null \; >> ${LONG_LINES}

sed -i -e '/Q_PROPERTY/d' ${LONG_LINES}
sed -i -e '/[*]ignorestyle[*]/d' ${LONG_LINES}

LINES_COUNT=`cat ${LONG_LINES} | wc -l | tr -d '[:space:]'`

if [ "$LINES_COUNT" -gt 0 ]; then
    echo "Detected too long lines"
    cat ${LONG_LINES}
    exit 2
fi

##################################
#### Check empty double lines ####
##################################

DBL_LINES_OK=1

while read -r file; do
    STRIPPED_WC=`cat -s ${file} | wc -l | tr -d '[:space:]'`
    USUAL_WC=`cat ${file} | wc -l | tr -d '[:space:]'`

    if [ "$STRIPPED_WC" -ne "$USUAL_WC" ]; then
	echo "Detected double empty lines in file $file"
        echo "Stripped WC: ${STRIPPED_WC} | Usual WC: ${USUAL_WC}"
	DBL_LINES_OK=0
    fi
done < <(find "$SRC_DIR" -path "$DEPLOYMENT_DIR" -prune -o -type f \( -name '*.cpp' -and ! -name 'moc_*.cpp' -and ! -name 'qrc_*.cpp' -and ! -name 'qmlcache_loader.cpp' -o  -name '*.h' -o -name '*.qml' \))

if [ "$DBL_LINES_OK" -eq 0 ]; then
    echo "Double lines check failed"
    exit 3
fi

echo "Style checking finished."
