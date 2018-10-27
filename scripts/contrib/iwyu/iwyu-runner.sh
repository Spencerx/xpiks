#!/bin/bash

### some config

INCORRECT_FILES=0  #temporary disabled [FIXME]
DIR_TO_CHECK="${1}"
THREADS_NUMBER=$(nproc)
SOURCES_TO_CHECK=()

# compilation flags
DEFINES=""
CXXFLAGS=""
INCPATH=""

# iwyu flags
MAPPING_FILE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )/mapping-file.imp"
IWYUFLAGS="-xc++ -Xiwyu --transitive_includes_only -Xiwyu --mapping_file=${MAPPING_FILE}"

function check_env() {
    # check if iwyu is installed
    if ! which iwyu > /dev/null; then
        echo >&2 "IWYU is not installed. Aborting."
        exit 1
    fi

    # check if Makefile exist
    if [ ! -f ${PWD}/Makefile ]; then
        echo >&2 "Makefile doesn't exist in current directory. Please run qmake first! Aborting."
        exit 1
    fi
}

function get_compilation_flags() {
    DEFINES=$(make -pn | egrep ^DEFINES)
    DEFINES=${DEFINES#*=}

    CXXFLAGS=$(make -pn | egrep ^CXXFLAGS)
    CXXFLAGS=${CXXFLAGS#*=}
    CXXFLAGS=${CXXFLAGS%\$\(DEFINES\)}

    INCPATH=$(make -pn | egrep ^INCPATH)
    INCPATH=${INCPATH#*=}
}

function find_files() {
    while IFS=  read -r -d $'\0'; do
        SOURCES_TO_CHECK+=("$REPLY")
    done < <(find "${DIR_TO_CHECK}" \( -name "*.cpp" -o -name "*.cxx" \) -print0)

    HEADERS_TO_CHECK=()
    while IFS=  read -r -d $'\0'; do
        # iwyu checks source file together with its header file,
        # we need to skip header if corresponding cpp file exist
        case "${SOURCES_TO_CHECK[@]}" in
            *"${REPLY%.*}.c"*) continue ;;
        esac
        HEADERS_TO_CHECK+=("$REPLY")
    done < <(find "${DIR_TO_CHECK}" \( -name "*.h" -o -name "*.hpp" \) -print0)

    # sort for grouping by directories
    FILES_TO_CHECK=($(printf '%s\n' "${SOURCES_TO_CHECK[@]}" "${HEADERS_TO_CHECK[@]}" | sort))
}

function run-iwyu() {
    OUTPUT="$(iwyu ${IWYUFLAGS} ${CXXFLAGS} ${DEFINES} ${INCPATH} "${1}" -o /dev/null 2>&1)"

    if (( $? != 2 )); then
        echo ""
        echo "${OUTPUT}"
#        ((INCORRECT_FILES++))  # temporary disabled
    fi
}

function check_includes() {
    for FILE in "${FILES_TO_CHECK[@]}"; do
        # simple round-robin parallelization
        ((i=i%THREADS_NUMBER)); ((i++==0)) && wait
        run-iwyu "${FILE}" &
    done

    # wait for last paraller jobs [FIXME]
    sleep 5
}

function print_summary() {
    echo ""  #new line

    ## summary message temporary disabled, because of problem with getting output from paraller jobs [FIXME]
    #if (( ${INCORRECT_FILES} != 0 )); then
    #    echo "You have ${INCORRECT_FILES} files with incorrect includes."
    #else
    #    echo "Congratulations, all files in \"${DIR_TO_CHECK}\" have clean includes!"
    #fi
}

###################################################

check_env
get_compilation_flags
find_files
check_includes
print_summary

exit ${INCORRECT_FILES}
