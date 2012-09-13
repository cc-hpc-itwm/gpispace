#
# load sdpa into the environment
#
function ldsdpa()
{
    path="$1"; shift
    if [ -z "$path" ] ; then
        if [ -n "$SDPA_HOME" ] ; then
            path="$SDPA_HOME"
        else
            echo >&2 "ldsdpa: need the path to the SDPA installation or SDPA_HOME"
            return 1
        fi
    fi
    if [ -e "$path/etc/sdpa/sdpa.env" ] ; then
        unset SDPA_LOADED
        export SDPA_HOME="$path"
        source "$SDPA_HOME"/etc/sdpa/sdpa.env
    else
        echo >&2 "ldsdpa: $path does not contain a valid SDPA installation"
        return 2
    fi
}
