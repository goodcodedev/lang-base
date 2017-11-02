if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    fips_setup()
    fips_project(lang-base)
endif()

fips_include_directories(code)
fips_add_subdirectory(code)

if (NOT FIPS_IMPORT)
    fips_add_subdirectory(playground)
    fips_finish()
endif()


# LDFLAGS:  -L/usr/local/opt/flex/lib
# CPPFLAGS: -I/usr/local/opt/flex/include
# LDFLAGS:  -L/usr/local/opt/bison/lib
