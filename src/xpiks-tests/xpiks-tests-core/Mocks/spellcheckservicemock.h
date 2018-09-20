#ifndef SPELLCHECKSERVICEMOCK_H
#define SPELLCHECKSERVICEMOCK_H

#include <Services/SpellCheck/spellcheckservice.h>
#include <Common/isystemenvironment.h>

namespace Mocks {
    class SpellCheckServiceMock:
        public SpellCheck::SpellCheckService
    {
    Q_OBJECT

    public:
        SpellCheckServiceMock(Common::ISystemEnvironment &environment,
                              Common::IFlagsProvider<Common::WordAnalysisFlags> &analysisFlagsProvider):
            SpellCheck::SpellCheckService(environment, analysisFlagsProvider)
        {
        }

        // SpellCheckerService interface

    public:
        virtual QStringList suggestCorrections(const QString &word) const {
            Q_UNUSED(word);
            return QStringList() << "item1" << "item2" << "item3";
        }
    };
}

#endif // SPELLCHECKSERVICEMOCK_H
