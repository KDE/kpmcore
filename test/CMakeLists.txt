# Tests for KPMcore
#
# These are not so much "tests" as "small example programs". They illustrate
# how to use the library, how to perform common tasks.

set(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(${CMAKE_SOURCE_DIR}/src)  # To get at KPMcore headers
add_compile_options(-fPIC)

###
#
# Helper macro to link to the helper (for initialization of kpmcore)
# and to add a test with the given name.
#
add_library(testhelpers OBJECT helpers.cpp)

macro (kpm_test name)
    add_executable(${name} ${ARGN} $<TARGET_OBJECTS:testhelpers>)
    target_link_libraries(${name} kpmcore)
endmacro()

###
#
# Tests of initialization: try explicitly loading some backends
kpm_test(testinit testinit.cpp)  # Default backend
if(TARGET pmdummybackendplugin)
    add_test(NAME testinit-dummy COMMAND testinit $<TARGET_FILE:pmdummybackendplugin>)
endif()
if(TARGET pmlibpartedbackendplugin)
    add_test(NAME testinit-parted COMMAND testinit $<TARGET_FILE:pmlibpartedbackendplugin>)
else()
    return()  # All the rest really needs a working backend
endif()

set(BACKEND $<TARGET_FILE:pmlibpartedbackendplugin>)

###
#
# Listing devices, partitions
kpm_test(testlist testlist.cpp)
add_test(NAME testlist COMMAND testlist ${BACKEND})

kpm_test(testdevicescanner testdevicescanner.cpp)
add_test(NAME testdevicescanner COMMAND testdevicescanner ${BACKEND})
