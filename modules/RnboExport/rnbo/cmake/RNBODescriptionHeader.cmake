function(rnbo_write_description_header DESCRIPTION_JSON OUTPUT_DIR)
	set(PATCHER_DESCRIPTION_JSON ${DESCRIPTION_JSON})
	set(PATCHER_PRESETS_JSON "{}")

	#check for optional args
	set (optional_args ${ARGN})
	list(LENGTH optional_args optional_count)

	if (${optional_count} GREATER 0)
		list(GET optional_args 0 PATCHER_PRESETS_JSON)
	endif ()

	configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/rnbo_description.h.in ${OUTPUT_DIR}/rnbo_description.h)
endfunction()

function(rnbo_write_description_header_if_exists RNBO_DESCRIPTION_FILE DESCRIPTION_INCLUDE_DIR)

	if (EXISTS ${RNBO_DESCRIPTION_FILE})
		file(READ ${RNBO_DESCRIPTION_FILE} DESCRIPTION_JSON)

		#check for optional args
		set (optional_args ${ARGN})
		list(LENGTH optional_args optional_count)

		set(PRESETS_JSON "{}")
		if (${optional_count} GREATER 0)
			list(GET optional_args 0 PRESETS_JSON_FILE)
			if (EXISTS ${PRESETS_JSON_FILE})
				file(READ ${PRESETS_JSON_FILE} PRESETS_JSON)
			endif()
		endif ()

		rnbo_write_description_header("${DESCRIPTION_JSON}" ${DESCRIPTION_INCLUDE_DIR} "${PRESETS_JSON}")
		add_definitions(-DRNBO_INCLUDE_DESCRIPTION_FILE)
	endif()
endfunction()

