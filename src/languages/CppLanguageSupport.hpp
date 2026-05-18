#pragma once

#include "languages/LanguageClient.hpp"

namespace MalloyWriter::Languages {

class CppLanguageSupport : public LanguageClient {
    Q_OBJECT

public:
    explicit CppLanguageSupport(QObject *parent = nullptr);

    QString languageId() const override;
    LanguageServerDefinition serverDefinition() const override;
};

}
