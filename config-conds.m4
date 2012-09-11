
AM_CONDITIONAL([ENABLE_BABEL],
                [test "$enable_babel" != "no"])

AM_CONDITIONAL([ENABLE_STRICT_Z],
                [test "$enable_strict_z" = "yes"])

AM_CONDITIONAL([ENABLE_SEGFAULT_ON_ERROR],
                [test "$enable_segfault-on-error" = "yes"])

AM_CONDITIONAL([ENABLE_FILELIST],
                [test "$enable_filelist" != "no"])

AM_CONDITIONAL([ENABLE_BLOCKBUFFER],
                [test "$enable_blockbuffer" != "no"])

AM_CONDITIONAL([ENABLE_COMMAND_HISTORY],
                [test "$enable_command_history" != "no"])

AM_CONDITIONAL([ENABLE_OUTPUT_HISTORY],
                [test "$enable_output_history" != "no"])

AM_CONDITIONAL([ENABLE_CONFIG_FILES],
                [test "$enable_config_files" != "no"])

AM_CONDITIONAL([ENABLE_PREFIX_COMMANDS],
                [test "$enable_prefix_commands" != "no"])

AM_CONDITIONAL([ENABLE_DEBUGGER],
                [test "$enable_debugger" = "yes"])

