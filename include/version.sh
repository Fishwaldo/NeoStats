# Script to generate SVN version information

id=`svn info ..|grep 'Revision:'`
id=`echo $id |sed 's/.* Revision: \(.*\) .* Exp .*/\1/'`
if [ x"$PULSE_BUILD_REVISION" != "x" ]
then
	id=$PULSE_BUILD_REVISION
fi

if test -r version.h
then
	idold=`sed -n 's/^#define NEOSTATS_REVISION \"\(.*\)\"/\1/p' < version.h`
	if [ "$idold" = "$id" ] 
	then 
		echo "Not Updating version.h" >&2
	else
echo "Updating version.h..." >&2

cat >version.h <<EOF
/*
 * This file is generated by version.sh. DO NOT EDIT. 
 */

#define NEOSTATS_REVISION "$id"

EOF
	fi
	else
echo "Creating version.h..."

cat >version.h <<EOF
/*
 * This file is generated by version.sh. DO NOT EDIT.
 */

#define NEOSTATS_REVISION "$id"

EOF
fi
