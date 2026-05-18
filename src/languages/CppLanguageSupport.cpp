#include "languages/CppLanguageSupport.hpp"

namespace MalloyWriter::Languages {

CppLanguageSupport::CppLanguageSupport(QObject *parent)
    : LanguageClient(parent)
{
}

QString CppLanguageSupport::languageId() const
{
    return "cpp";
}

LanguageServerDefinition CppLanguageSupport::serverDefinition() const
{
    return {
        "clangd",
        "C++ Language Server",
        "clangd",
        {"--background-index", "--clang-tidy"}
    };
}

}
