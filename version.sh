# Script to generate SVN version information
id=`svn info |grep 'Revision:'`
id=`echo $id |sed 's/.* Revision: \(.*\) .* Exp .*/\1/'`

if test -r version.h
then
	idold=`sed -n 's/^#define NEOSTATS_REVISION \"\(.*\)\"/\1/p' < version.h`
	if [ "$idold" = "$id" ] 
	then 
		echo "version.h up to date"
	else
echo "Updating version.h..."

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
