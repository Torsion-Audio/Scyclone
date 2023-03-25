include_guard(GLOBAL)

find_program(SCCACHE_PROGRAM sccache)
if (SCCACHE_PROGRAM)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${SCCACHE_PROGRAM}")
endif()
