#!/bin/bash
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# 0.0.1
# Alexey Potehin http://www.gnuplanet.ru/doc/cv
#
# git clone git://github.com/progman/patch_spliter
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
function main()
{
    if [ ! -e "${FILE}" ];
    then
	echo "example: ${0} file.patch";
	exit 1;
    fi

    TOTAL_LINE=$(cat ${FILE} | wc -l);

    if [ "${TOTAL_LINE}" == "0" ];
    then
	echo "ERROR: file is empty";
	exit 1;
    fi

    (( TOTAL_LINE++ ));

    TMP=$(mktemp);
    grep -n diff "${FILE}" | sed -e 's/\:.*//g' >> "${TMP}";
    echo "${TOTAL_LINE}" >> "${TMP}";

#    cat "${TMP}";

    COUNT=1;

    FLAG_FIRST='1';
    while read -r B;
    do
	if [ "${FLAG_FIRST}" == "1" ];
	then
	    if [ "${B}" != "1" ];
	    then
		echo "ERROR: strange file";
		exit 1;
	    fi
	    FLAG_FIRST='0';
	    A="${B}";
	    continue;
	fi


	(( C = B - A + 0 ));

	(( D = TOTAL_LINE - A + 0 ));

#	echo "A: ${A}";
#	echo "B: ${B}";
#	echo "C: ${C}";
#	echo "D: ${D}";

	tail -n ${D} "${FILE}" | head -n ${C} > ${FILE}-$(printf "%04u" ${COUNT}).patch;

	(( COUNT++ ));

#	break;

	A="${B}";


    done < "${TMP}";


    rm -rf "${TMP}" &> /dev/null < /dev/null;
}
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
FILE="${1}";
main;
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
