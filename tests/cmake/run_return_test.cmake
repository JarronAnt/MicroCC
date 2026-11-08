if(NOT DEFINED EXPECTED_EXIT)
    set(EXPECTED_EXIT 42)
endif()

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
        "MicroCC compilation failed.\n"
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

if(NOT run_result EQUAL EXPECTED_EXIT)
    message(FATAL_ERROR
        "Expected exit code ${EXPECTED_EXIT}, "
        "but got ${run_result}.\n"
        "stdout:\n${program_stdout}\n"
        "stderr:\n${program_stderr}"
    )
endif()

message(STATUS
    "Test passed with expected result ${EXPECTED_EXIT}"
)
