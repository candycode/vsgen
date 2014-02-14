#pragma once
#include <typeinfo>
#include <string>

struct BadCast : std::bad_cast {
    BadCast(const std::type_info& from,
            const std::type_info& to) 
    : msg_("Trying to cast " + std::string(from.name()) 
           + " to " + std::string(to.name())) {}
    virtual const char* what() const {
        return msg_.c_str();    
    }
    std::string msg_;
};
