#!/bin/bash

usage ()
{
    cat <<EOF
usage: $(basename "$0") [options] <directory>

options:

    -h|--help    : print this usage information
    -v|--verbose : be more verbose
    -V|--version : set the version of the created package
    -d|--destdir : place output file into this directory
EOF
}

abspath()
{
    case "$1" in
        /*)
            echo "$1"
            ;;
        *)
            echo "$(pwd)/$1"
            ;;
    esac
}

version=
destdir="$(pwd)"
verbose=0

while [ $# -gt 0 ]
do
    case "$1" in
        -h|--help)
            usage
            shift
            exit 0
            ;;
        -v|--verbose)
            verbose=$((verbose+1))
            shift
            ;;
        -V|--version)
            shift
            if [ -z "$1" ]
            then
                echo >&2 "make: missing argument to --version"
                exit 1
            fi
            version="$1"
            shift
            ;;
        -d|--destdir)
            shift
            if [ ! -d "$1" ]
            then
                echo >&2 "make: missing/invalid argument to --destdir"
                exit 1
            fi
            destdir="$1"
            shift
            ;;
        *)
            break
            ;;
    esac
done

if [ $# -lt 1 ]
then
    echo >&2 "make: directory is missing"
    exit 1
fi
dir=$(abspath "$1")
dir_name=$(basename "${dir}")

if [ ! -r "${dir}" ]
then
    echo >&2 "make: cannot access '${dir}' for reading"
    exit 1
fi

if [ ! -x "${dir}/bin/gspc" ]
then
    echo >&2 "make: '${dir}' does not contain a valid GPISpace version"
    exit 1
fi
if [ -z "$version" ]
then
    version=$("${dir}/bin/gspc" --dumpversion)
fi

if [ ! -e "${dir}/etc/sdpa" ]
then
    echo >&2 "make: '${dir}' does not contain a valid SDPA directory"
    exit 1
fi

name="sdpa-$version"
exe_name="${name}.sh"

mydir=$(cd $(dirname "$0") && pwd)

installer_in="${mydir}/installer.sh.in"
if [ ! -e "${installer_in}" ]
then
    echo >&2 "cannot execute '${extract}'"
    exit 2
fi

oldpwd="$(pwd)"
tmpdir=$(mktemp -d)

cleanup()
{
    cd "${oldpwd}"
    rm -rf "${tmpdir}"
}
trap cleanup EXIT TERM

cd "${tmpdir}"

echo "creating '${exe_name}'"

tarflags=()
if [ $verbose -gt 0 ]
then
    tarflags+=(-v)
fi

sed -e "s,@GSPC_DIRECTORY_NAME@,${name},g" "${installer_in}" > installer.sh
(cat installer.sh ; tar cjf - ${tarflags[@]} --transform "s,$dir_name,$name," -C $(dirname "${dir}") "${dir_name}") > "${exe_name}"
chmod +x "${exe_name}"

mv "${exe_name}" "${destdir}/"

rm -rf "${tmpdir}"

echo "created '${exe_name}'"
