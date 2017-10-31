fips_begin_module(lang-base)
    fips_dir(LangBase)
    fips_files(
        Ast.hpp Ast.cpp
        DescrNode.hpp DescrVisitor.hpp
        GrammarRule.hpp GrammarRule.cpp
        LangData.hpp LangData.cpp
        RuleAction.hpp RuleAction.cpp
        TypedPart.hpp TypedPart.cpp
    )
    fips_dir(LangBase/process)
    fips_files(
        RegisterKeysVisitor.hpp
        RegisterListKeysVisitor.hpp
        AddBuiltInTokens.hpp
        BuildRulesVisitor.hpp
        BuildAstVisitor.hpp
        ToSourceGenVisitor.hpp
        SourceGenerator.hpp
    )
    fips_dir(LangBase/gen)
    fips_files(
        lex.yy.cpp descr.tab.h descr.tab.cpp
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROJECT_ROOT=\\\"${FIPS_PROJECT_DIR}\\\"")
fips_end_module()