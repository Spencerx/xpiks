#ifndef SPELLCHECKSERVICEMOCK_H
#define SPELLCHECKSERVICEMOCK_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "Common/flags.h"
#include "Services/SpellCheck/spellcheckservice.h"

namespace Common {
    class ISystemEnvironment;
    template <class T> class IFlagsProvider;
}

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
