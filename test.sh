#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# 0.0.1
# Alexey Potehin <gnuplanet@gmail.com>, http://www.gnuplanet.ru/doc/cv
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
APP='./bin/patch_spliter';
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# run app
function run_app()
{
	local RESULT=0;
	local STDOUT;

	if [ "${FLAG_VALGRIND}" != "1" ];
	then
		STDOUT=$("${APP}" "${@}");
		RESULT="${?}";
	else
		local LOG_ID=0;
		local LOG_NAME;

		while true;
		do
			LOG_NAME=$(printf "valgrind.%03u\n" ${LOG_ID});

			if [ ! -e "${LOG_NAME}" ];
			then
				break;
			fi

			(( LOG_ID++ ));
		done

		STDOUT=$(valgrind --tool=memcheck --leak-check=yes --leak-check=full --show-reachable=yes --log-file="${LOG_NAME}" "${APP}" "${@}");
		RESULT="${?}";
	fi

	if [ "${STDOUT}" != "" ];
	then
		echo "${STDOUT}";
	fi

	return "${RESULT}";
}
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# test1
function test1()
{
	local PATCH_FILE="test.patch";

	if [ ! -e "${PATCH_FILE}" ];
	then
		echo "ERROR: file \"${PATCH_FILE}\" not found";
		exit 1;
	fi


	local TMP;
	TMP="$(mktemp -d)";
	if [ "${?}" != "0" ];
	then
		echo "ERROR: can't make tmp dir";
		exit 1;
	fi


	local TMP2;
	TMP2="$(mktemp)";
	if [ "${?}" != "0" ];
	then
		echo "ERROR: can't make tmp file";
		exit 1;
	fi

	cp "${PATCH_FILE}" "${TMP}/";

	local DIR_CUR="${PWD}";


#	cd -- "${TMP}";


	run_app "${TMP}/${PATCH_FILE}" < /dev/null &> /dev/null;
	if [ "${?}" != "0" ];
	then
		echo "ERROR: broken run";
		rm -rf -- "${TMP}";
		rm -rf -- "${TMP2}";
		exit 1;
	fi


	rm -rf -- "${TMP}/${PATCH_FILE}";

	md5sum "${TMP}/"* | sort > "${TMP2}";

	local COUNT=$(cat "${TMP2}" | wc -l);

	if [ "${COUNT}" != "2" ];
	then
		echo "ERROR: invalid file count (${COUNT})";
		rm -rf -- "${TMP}";
		rm -rf -- "${TMP2}";
		exit 1;
	fi


	while true;
	do
		read -r HASH FILE;

		if [ "${COUNT}" == "2" ] && [ "${HASH}" != "994e9b9b862fd5e540fa97689b6c8135" ];
		then
			echo "ERROR: broken patch";
			rm -rf -- "${TMP}";
			rm -rf -- "${TMP2}";
			exit 1;
		fi

		if [ "${COUNT}" == "1" ] && [ "${HASH}" != "a2fe4c71e93c611227f6c40d0a083f39" ];
		then
			echo "ERROR: broken patch";
			rm -rf -- "${TMP}";
			rm -rf -- "${TMP2}";
			exit 1;
		fi

		(( COUNT -- ));

		if [ "${COUNT}" == "0" ];
		then
			break;
		fi
	done < "${TMP2}";


#	cd -- "${DIR_CUR}";


	rm -rf -- "${TMP}";
	rm -rf -- "${TMP2}";
}
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# check depends
function check_prog()
{
	for i in ${1};
	do
		if [ "$(which ${i})" == "" ];
		then
			echo "FATAL: you must install \"${i}\"...";
			return 1;
		fi
	done

	return 0;
}
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
# general function
function main()
{
	if [ ! -e "${APP}" ];
	then
		echo "ERROR: make it";
		return 1;
	fi


	check_prog "awk cat echo md5sum mktemp rm wc";
	if [ "${?}" != "0" ];
	then
		return 1;
	fi


	test1;


	echo "ok, test passed";
	return 0;
}
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
main "${@}";

exit "${?}";
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------#
