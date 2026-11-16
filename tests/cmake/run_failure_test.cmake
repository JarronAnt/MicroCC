
execute_process(
    COMMAND
        "${MICROCC}"
        "${SOURCE}"
        -S
        -o
        "${OUTPUT}"

    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)

if(compile_result EQUAL 0)
    message(FATAL_ERROR
        "Expected compilation to fail: ${SOURCE}"
    )
endif()

string(
    FIND
    "${compile_stderr}"
    "${EXPECTED_DIAGNOSTIC}"
    match_index
)

if(match_index EQUAL -1)
    message(FATAL_ERROR
        "Compiler failed, but diagnostic was wrong.\n"
        "Expected: ${EXPECTED_DIAGNOSTIC}\n"
        "Actual:\n${compile_stderr}"
    )
endif()

message(STATUS
    "Expected failure diagnosed: ${EXPECTED_DIAGNOSTIC}"
)

