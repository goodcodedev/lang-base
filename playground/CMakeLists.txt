fips_begin_app(lang-pg cmdline)
    fips_vs_warning_level(3)
    fips_files(lang-pg.cpp)
    fips_include_directories(gen)
    fips_dir(gen)
    fips_files(
        TestLang.hpp TestLang.tab.cpp 
        TestLang.yy.cpp TestLang.tab.h
        TestLangVisitor.hpp TestLangToSource.hpp
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_ROOT=\\\"${FIPS_PROJECT_DIR}\\\"")
fips_end_app()

fips_begin_app(lang-gen cmdline)
    fips_vs_warning_level(3)
    fips_files(lang-gen.cpp)
    fips_deps(lang-base)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_ROOT=\\\"${FIPS_PROJECT_DIR}\\\"")
fips_end_app()
