#!/bin/bash

pnet_dir=${KDM_PNET_DIR:="$HOME/pnet"}

if [ ! -d "$pnet_dir" ] ; then
    echo "please set the KDM_PNET_DIR environment variable to a directory containing the kdm workflows" >&2
fi

workflows="full simple_bounded simple_unbounded"

ec=0
for w in $workflows ; do
    if [ ! -e "$pnet_dir/kdm_$w.pnet" ] ; then
	echo "$pnet_dir does not contain kdm_$w.pnet, sorry!" >&2
	ec=$(( ec + 1 ))
    fi
done

if [ $ec -ne 0 ] ; then
    echo "at least one workflow was missing in $pnet_dir, giving up" >&2
    exit $ec
fi

function normpath ()
{
    local p="$1" ; shift
    case "$p" in
	/*)
	    echo "$p" | sed -e 's,//,/,g'
	    ;;
	*)
	    echo "$(pwd)/$p" | sed -e 's,//,/,g'
	    ;;
    esac
    return 0
}

function usage ()
{
    cat >&2 <<EOF
usage: $(basename $0) {full|simple_bounded|simple_unbounded} <config-xml>
EOF
}

temp_wf=$(mktemp)
function cleanup()
{
    test -e "$temp_wf" && rm -f "$temp_wf"
}
trap cleanup EXIT

function submit_kdm ()
{
    local kdm_style="$1" ; shift
    local config="$1" ; shift

    if [ ! -r "$pnet_dir/kdm_${kdm_style}.pnet" ] ; then
	echo "kdm workflow $kdm_version does not exist" >&2
	return 1
    fi

    if [ ! -r "$config" ] ; then
	echo "config file \"$config\" does not exist" >&2
	return 2
    fi

    config=$(normpath "$config")

    echo "placing tokens..."
    pnetput --if "$pnet_dir/kdm_${kdm_style}.pnet" \
	    --of ${temp_wf} \
	    --put config_file=\""${config}"\"
    echo "submitting job..."
    sdpa submit "$temp_wf"
    sdpa unload-modules >/dev/null 2>&1
}


kind="$1" ; shift

case "$kind" in
    help)
	usage
	exit 0
	;;
    ?*)
	submit_kdm "$kind" $@
	;;
    *)
	usage
	exit 1
esac
