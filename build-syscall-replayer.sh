#!/bin/bash
#
# Build the program reanimator-replayer, installing any required dependencies if
# requested.

####################
# Script variables #
####################
readonly numberOfCores="$(nproc --all)"
install=false
installPackages=false
buildType="Debug"
configArgs=""
replayerDir="$(pwd)"
installDir="${replayerDir}/reanimator_replayer_release"
buildDir="${replayerDir}/build"
repositoryDir="${buildDir}/repositories"

readonly programDependencies=("autoconf" "automake" "cmake" "gcc" "g++" "perl" "git" "libaio-dev")
missingPrograms=()

####################
# Script functions #
####################
function runcmd
{
    echo "CMD: $*"
    sleep 0.2
    "$@"
    ret=$?
    if test $ret -ne 0 ; then
        exit $ret
    fi
}

function printUsage
{
    (
    cat << EOF
Usage: $0 [options...]
Options:
     --build-dir DIR        Download repositories and place build files in DIR
     --config-args ARGS     Append ARGS to every ./configure command
     --install              Install libraries and binaries under /usr/local
     --install-dir DIR      Install libraries and binaries under DIR
     --install-packages     Automatically use apt-get to install missing packages
     --release              Build an optimized release version of reanimator replayer
     -h, --help             Print this help message
EOF
    ) >&2
    exit 0
}

##################
# Script startup #
##################

# Parse script arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case "${key}" in
        --build-dir)
            shift # past argument
            repositoryDir="$(realpath "$1" || exit $?)"
            shift # past value
            ;;
        --config-args)
            shift # past argument
            configArgs="$1"
            shift # past value
            ;;
        --install)
            install=true
            installDir="/usr/local"
            shift # past argument
            ;;
        --install-dir)
            shift # past argument
            installDir="$(realpath "$1" || exit $?)"
            shift # past value
            ;;
        --release)
            buildType="Release"
            shift # past argument
            ;;
        --install-packages)
            if command -v apt-get >/dev/null; then
                installPackages=true
            else
                echo "Could not find apt-get. Missing packages must be \
                installed manually." >&2
            fi
            shift # past argument
            ;;
        -h|--help)
            printUsage
            shift # past argument
            ;;
        *)
            shift # past argument
            ;;
    esac
done

# Check system for sudo command
if $install; then
    if ! command -v sudo &>/dev/null; then
        echo "Script could not find 'sudo' command. Cannot install." >&2
        exit 1
    fi
fi

# Check whether program dependencies are installed
for program in "${programDependencies[@]}"; do
    programPath=$(command -v "${program}")
    if [[ $? == 0 ]]; then
        echo "${program}: Located at ${programPath}"
    elif [[ "${installPackages}" == true ]]; then
        echo "${program}: Not found. Queuing for installation..."
        missingPrograms+=("${program}")
    else
        echo "${program}: Not found."
        missingPrograms+=("${program}")
    fi
done

# Check whether the user has all required programs for building
if [[ "${#missingPrograms[@]}" -gt 0 ]]; then
    if [[ "${installPackages}" == true ]]; then
        echo "Installing missing programs."
        runcmd sudo apt-get install -y "${missingPrograms[*]}"
    else
        echo "Could not find all required programs. Not found:"
        for program in "${missingPrograms[@]}"; do
            echo "  ${program}"
        done

        echo "To install on a Debian-based system, run the command"
        echo "  sudo apt-get install ${missingPrograms[*]}"
        exit 1
    fi
fi

#################
# Build process #
#################

# Cloning all repositories
runcmd mkdir -p "${repositoryDir}"
runcmd cd "${repositoryDir}"
[[ -d "oneTBB" ]] || runcmd git clone https://github.com/oneapi-src/oneTBB.git
[[ -d "reanimator-library" ]] || runcmd git clone https://github.com/SNIA/reanimator-library.git

# Build reanimator-library
# ------------------------
runcmd cd reanimator-library
runcmd chmod +x build-reanimator-library.sh
if $install; then
    runcmd ./build-reanimator-library.sh --config-args "${configArgs}" --install
else
    runcmd ./build-reanimator-library.sh --config-args "${configArgs}" --install-dir "${installDir}"
fi
runcmd cd "${repositoryDir}"

# Building tbb
# ------------
runcmd cd oneTBB
runcmd git fetch --all --tags --prune
runcmd git checkout tags/v2020.3
if $install; then
    runcmd sudo cp -r ./include/. "${installDir}/include"
    runcmd sudo make tbb_build_dir="${installDir}/lib" tbb_build_prefix=one_tbb -j"${numberOfCores}"
    runcmd sudo cp "${installDir}/lib/one_tbb_release/"*".so"* "${installDir}/lib"
else
    runcmd cp -r ./include/. "${installDir}/include"
    runcmd make tbb_build_dir="${installDir}/lib" tbb_build_prefix=one_tbb -j"${numberOfCores}"
    runcmd cp "${installDir}/lib/one_tbb_release/"*".so"* "${installDir}/lib"
fi
runcmd cd "${repositoryDir}"

# Building syscall-replayer
# -------------------------
runcmd cd "${replayerDir}"
runcmd cd build
runcmd cmake -DCMAKE_INSTALL_PREFIX="${installDir}" -DCMAKE_BUILD_TYPE="${buildType}" ..
runcmd make -j"${numberOfCores}"
runcmd make -j"${numberOfCores}" install
