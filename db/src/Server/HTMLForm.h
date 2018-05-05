#pragma once
#include<Poco/Net/HTMLForm.h>
#include<Poco/Net/HTTPRequest.h>
#include<IO/ReadHelper.h>
#include<Poco/URI.h>
#include<istream>
#include<sstream>

namespace DataBase {

class HTMLForm:public Poco::Net::HTMLForm {
public:
    HTMLForm(const Poco::Net::HTTPRequest & request)
    {
        Poco::URI uri(request.getURI());
        std::istringstream istr(uri.getRawQuery());
        readUrl(istr);
    }

    HTMLForm(const Poco::URI & uri)
    {
        std::istringstream istr(uri.getRawQuery());
        readUrl(istr);
    }


    template <typename T>
    T getParsed(const std::string & key, T default_value)
    {
        auto it = find(key);
        return (it != end()) ? IO::parse<T>(it->second) : default_value;
    }

    template <typename T>
    T getParsed(const std::string & key)
    {
        return IO::parse<T>(get(key));
    }

};

}
