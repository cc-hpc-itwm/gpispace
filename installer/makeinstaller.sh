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

    -H|--hook    : add a hook
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
hooks=()
apps=()

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
        -H|--hook)
            shift
            if [ -z "$1" ]
            then
                echo >&2 "make: missing argument to --hook"
                exit 1
            fi
            h=$(abspath "$1")
            if [ ! -x "$h" ]
            then
                echo >&2 "make: hook is not executable: '$1'"
                exit 1
            fi
            hooks+=("$h")
            shift
            ;;
        -A|--app)
            shift
            if [ -z "$1" ]
            then
                echo >&2 "make: missing argument to --app"
                exit 1
            fi
            appname=${1%=*}
            apppath=${1#*=}
            if [ -z "$appname" -o -z "${apppath}" ]
            then
                echo >&2 "make: invalid app specification: $1"
                exit 1
            fi

            apps+=("$appname" "$apppath")

            unset appname
            unset apppath
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

pkg_name="sdpa-$version"
exe_name="${pkg_name}.sh"

mydir=$(cd $(dirname "$0") && pwd)

installer_in="${mydir}/installer.sh.in"
if [ ! -e "${installer_in}" ]
then
    echo >&2 "cannot execute '${extract}'"
    exit 2
fi

oldpwd="$(pwd)"
tmpdir=$(mktemp -d 2>&1)
if [ $? -ne 0 ]
then
    echo >&2 "make: could not create temporary directory: $tmpdir"
    exit 2
fi

cleanup()
{
    cd "${oldpwd}"
    rm -rf "${tmpdir}"
}
trap cleanup EXIT TERM

cd "${tmpdir}"

mkdir -p "hooks"

cnt=0
for h in "${hooks[@]}"
do
    base=$(basename "$h")
    hook="$(printf %02d $cnt)-${base}"
    cp "${h}" "hooks/${hook}"
    chmod +x "hooks/${hook}"
    cnt=$((cnt+1))
done

echo "creating '${exe_name}'"

tarflags=(-P)
if [ $verbose -gt 0 ]
then
    tarflags+=(-v)
fi

sed -e "s,@GSPC_DIRECTORY_NAME@,${pkg_name},g" "${installer_in}" > installer.sh
(cat installer.sh ; tar cjf - ${tarflags[@]} --transform "s,.*/$dir_name,$pkg_name," "${dir}" hooks) > "${exe_name}"
chmod +x "${exe_name}"

mv "${exe_name}" "${destdir}/"

rm -rf "${tmpdir}"

echo "created '${exe_name}'"
