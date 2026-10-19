find_package(Qt6 6.2 REQUIRED COMPONENTS Test)

function(monthmint_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE monthmint_core Qt6::Test Qt6::Sql)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

monthmint_add_test(test_money
    test_money.cpp
)

monthmint_add_test(test_date_utils
    test_date_utils.cpp
)

monthmint_add_test(test_formatting
    test_formatting.cpp
)

monthmint_add_test(test_transactions
    test_transactions.cpp
    test_support.h
)

monthmint_add_test(test_summary
    test_summary.cpp
    test_support.h
)

monthmint_add_test(test_budgets
    test_budgets.cpp
    test_support.h
)

monthmint_add_test(test_savings
    test_savings.cpp
    test_support.h
)

monthmint_add_test(test_recurring
    test_recurring.cpp
    test_support.h
)

monthmint_add_test(test_migrations
    test_migrations.cpp
)

monthmint_add_test(test_csv_export
    test_csv_export.cpp
    test_support.h
)
