#include "templates_cout.h"

QDebug operator <<(QDebug debug, const std::string &s)
{
    debug << s.c_str();
    return debug.maybeSpace();
}

std::ostream& operator <<(std::ostream& os, QString const &s)
{
    return os << s.toStdString();
}

//std::ostream& operator<<(std::ostream& os, const QByteArray& c) {
//    return os << "[" << Join(c, ',') << "]";
//}