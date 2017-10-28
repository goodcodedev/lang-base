if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    fips_setup()
    fips_project(lang-base)
endif()

fips_begin_module(lang-base cmdline)
    fips_files(
        lex.yy.cpp descr.tab.h descr.tab.cpp
        DescrNode.hpp DescrVisitor.hpp
        LangData.hpp
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_ROOT=\\\"${FIPS_PROJECT_DIR}\\\"")
fips_end_app()

if (NOT FIPS_IMPORT)
    fips_finish()
endif()


# LDFLAGS:  -L/usr/local/opt/flex/lib
# CPPFLAGS: -I/usr/local/opt/flex/include
# LDFLAGS:  -L/usr/local/opt/bison/lib
