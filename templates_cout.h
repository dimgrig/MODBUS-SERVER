//
// Created by Dima on 14.05.2021.
//

#ifndef TEMPLATES_COUT_H
#define TEMPLATES_COUT_H

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <QtDebug>

QDebug operator <<(QDebug debug, const std::string &s);
std::ostream& operator <<(std::ostream& os, QString const &s);
//std::ostream& operator <<(std::ostream& os, QByteArray const &c);


/* COUT */

template <typename Collection>
std::string Join(const Collection& c, char separator);

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& c) {
    return os << "[" << Join(c, ',') << "]";
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& c) {
    return os << "(" << Join(c, ',') << ")";
}

template <class K, class V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& c) {
    return os << "{" << Join(c, ',') << "}";
}

template <class F, class S>
std::ostream& operator<<(std::ostream& os, const std::pair<F, S>& p) {
    std::string quotes;
    if (typeid (std::string) == typeid(F)) {
        quotes = "\"";
    } else {
        quotes = "";
    }
    return os << "{" << quotes << p.first << quotes << ", " << p.second << "}";
}

/* COUT STRINGSTREAM */
template <class T>
std::ostream& operator<<(std::stringstream& os, const QVector<T>& c) {
    return os << "[" << Join(c, ',') << "]";
}

template <class T>
std::ostream& operator<<(std::stringstream& os, const std::vector<T>& c) {
    return os << "[" << Join(c, ',') << "]";
}

template <class T>
std::ostream& operator<<(std::stringstream& os, const std::set<T>& c) {
    return os << "(" << Join(c, ',') << ")";
}

template <class K, class V>
std::ostream& operator<<(std::stringstream& os, const std::map<K, V>& c) {
    return os << "{" << Join(c, ',') << "}";
}

template <class F, class S>
std::ostream& operator<<(std::stringstream& os, const std::pair<F, S>& p) {
    std::string quotes;
    if (typeid (std::string) == typeid(F)) {
        quotes = "\"";
    } else {
        quotes = "";
    }
    return os << "{" << quotes << p.first << quotes << ", " << p.second << "}";
}

template <class ... T>
std::ostream& operator<<(std::ostream& os, const std::vector<std::tuple<T ... >>& v) {
    os << "[";
    std::string separator = ",";
    bool first = true;
    for (const auto& t : v) {
        Q_UNUSED(t);
        if (!first) {
            os << separator;
        }
        first = false;
        //ss << tuple_print(t);
//        std::apply([&os, separator](auto&& farg, auto&& ... args) {
//            //ss << tuple_print(args);
//            os << "[";
//            os << farg;
//            ((os << "," << args ), ...);
//            os << "]";
//        }, t);

    }
    os << "]";
    return os;
}


/* QDEBUG */

template <class T>
QDebug operator<<(QDebug os, const std::vector<T>& c) {
    return os << "[" << QString::fromStdString(Join(c, ',')) << "]";
}

template <class T>
QDebug operator<<(QDebug os, const std::set<T>& c) {
    return os << "(" << QString::fromStdString(Join(c, ',')) << ")";
}

template <class K, class V>
QDebug operator<<(QDebug os, const std::map<K, V>& c) {
    return os << "{" << QString::fromStdString(Join(c, ',')) << "}";
}

/* JOIN */

template <typename Collection>
std::string Join(const Collection& c, char separator) {
    std::stringstream ss;
    bool first = true;
    for (const auto& x : c) {
        if (!first) {
            ss << separator;
        }
        first = false;
        ss << x;
    }
    return ss.str();
}


/* TRASH */

//template <class T>
//std::ostream& operator<<(std::ostream& os, const std::variant<T, std::vector<T>>& v) {
//    if (std::holds_alternative<std::string>(v)) {
//        return os << "[" << std::get<std::string>(v) << "]";
//    } else {
//        return os << "[" << Join(std::get<std::vector<std::string>>(v), ',') << "]";
//    }
//}

//template <class K, class V>
//std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& c) {
//    return os << "{" << Join(c, ',') << "}";
//}

//std::ostream& operator<<(std::ostream& os, const QByteArray& ba) {
//    std::string s;
//    for(int i = 0; i < ba.length(); ++i) {
//        s += (ba[i]);
//        s += ", ";
//    }
//    return os << "[" << s << "]";
//}

//template <class K, class V>
//QDebug operator<<(QDebug os, const std::unordered_map<K, V>& c) {
//    return os << "{" << QString::fromStdString(Join(c, ',')) << "}";
//}
/* defined in QtDebug -> qdebug.h
template <class F, class S>
QDebug operator<<(QDebug os, const std::pair<F, S>& p) {
    return os << "{" << p.first << ", " << p.second << "}";
}
*/

//template <class T>
//QDebug operator<<(QDebug os, const std::variant<T, std::vector<T>>& v) {
//    if (std::holds_alternative<std::string>(v)) {
//        return os << "[" << std::get<std::string>(v) << "]";
//    } else {
//        return os << "[" << Join(std::get<std::vector<std::string>>(v), ',') << "]";
//    }
//}

//template <class ... T>
//QDebug operator<<(QDebug os, const std::vector<std::tuple<T ... >>& v) {
//    os << "[";
//    std::string separator = ",";
//    bool first = true;
//    for (const auto& t : v) {
//        if (!first) {
//            os << separator;
//        }
//        first = false;
//        //ss << tuple_print(t);
//        std::apply([&os, separator](auto&& farg, auto&& ... args) {
//            //ss << tuple_print(args);
//            os << "[";
//            os << farg;
//            ((os << "," << args ), ...);
//            os << "]";
//        }, t);

//    }
//    os << "]";
//    return os;
//}

#endif //TEMPLATES_COUT_H
