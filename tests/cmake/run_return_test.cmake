execute_process(
    COMMAND
        "${MICROCC}"
        "${SOURCE}"
        -o
        "${OUTPUT}"

    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)

if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR
        "MicroCC failed to compile test program.\n"
        "stdout:\n${compile_stdout}\n"
        "stderr:\n${compile_stderr}"
    )
endif()

execute_process(
    COMMAND "${OUTPUT}"

    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE program_stdout
    ERROR_VARIABLE program_stderr
)

if(NOT run_result EQUAL 42)
    message(FATAL_ERROR
        "Expected program to return 42, "
        "but it returned ${run_result}.\n"
        "stdout:\n${program_stdout}\n"
        "stderr:\n${program_stderr}"
    )
endif()

message(STATUS
    "MicroCC successfully compiled a program returning 42"
)
