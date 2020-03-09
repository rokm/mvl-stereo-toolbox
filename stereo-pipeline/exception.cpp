#include "exception.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


Exception::Exception (const QString &message)
    : std::runtime_error(message.toStdString())
{
}

Exception::Exception (const QString &messagePrefix, const std::exception &e)
    : Exception(QStringLiteral("%1: %2").arg(messagePrefix).arg(QString::fromStdString(e.what())))
{
}

Exception::~Exception ()
{
}


} // Pipeline
} // StereoToolbox
} // MVL
