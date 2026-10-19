cmake_minimum_required(VERSION 3.21)

project(MonthMint
    VERSION 1.0.0
    DESCRIPTION "MonthMint - Your money. One simple monthly view."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(MONTHMINT_BUILD_TESTS "Build unit tests" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets Sql)

if(MONTHMINT_BUILD_TESTS)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Test)
    enable_testing()
endif()

add_library(monthmint_core STATIC
    src/core/money.cpp
    src/core/date_utils.cpp
    src/core/formatting.cpp
    src/core/csv.cpp
    src/models/transaction.cpp
    src/models/category.cpp
    src/models/budget.cpp
    src/models/savings.cpp
    src/models/recurring.cpp
    src/models/settings.cpp
    src/database/connection.cpp
    src/database/migrations.cpp
    src/database/transaction_repo.cpp
    src/database/category_repo.cpp
    src/database/budget_repo.cpp
    src/database/savings_repo.cpp
    src/database/recurring_repo.cpp
    src/database/settings_repo.cpp
    src/services/transaction_service.cpp
    src/services/budget_service.cpp
    src/services/finance_summary_service.cpp
    src/services/recurring_service.cpp
    src/services/savings_service.cpp
    src/services/export_service.cpp
    src/services/backup_service.cpp
)

target_include_directories(monthmint_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(monthmint_core PUBLIC
    Qt6::Core
    Qt6::Sql
)

target_compile_definitions(monthmint_core PUBLIC
    MONTHMINT_VERSION="${PROJECT_VERSION}"
)

if(MSVC)
    target_compile_options(monthmint_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(monthmint_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(MonthMint
    src/main.cpp
    src/ui/main_window.cpp
    src/ui/dashboard_page.cpp
    src/ui/transactions_page.cpp
    src/ui/budgets_page.cpp
    src/ui/savings_page.cpp
    src/ui/settings_dialog.cpp
    src/ui/add_transaction_dialog.cpp
    src/ui/first_run_dialog.cpp
    src/ui/recurring_dialog.cpp
    src/ui/month_selector.cpp
    src/ui/money_edit.cpp
    src/ui/progress_widget.cpp
    src/ui/theme.cpp
    include/monthmint/ui/main_window.h
    include/monthmint/ui/dashboard_page.h
    include/monthmint/ui/transactions_page.h
    include/monthmint/ui/budgets_page.h
    include/monthmint/ui/savings_page.h
    include/monthmint/ui/settings_dialog.h
    include/monthmint/ui/add_transaction_dialog.h
    include/monthmint/ui/first_run_dialog.h
    include/monthmint/ui/recurring_dialog.h
    include/monthmint/ui/month_selector.h
    include/monthmint/ui/money_edit.h
    include/monthmint/ui/progress_widget.h
    include/monthmint/ui/theme.h
    assets/resources.qrc
)

target_link_libraries(MonthMint PRIVATE
    monthmint_core
    Qt6::Gui
    Qt6::Widgets
)

if(MONTHMINT_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS MonthMint RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32)
    set_target_properties(MonthMint PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
