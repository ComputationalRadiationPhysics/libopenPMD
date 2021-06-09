#pragma once

#include <exception>
#include <string>
#include <utility>

namespace openPMD
{
class Error : public std::exception
{
private:
    std::string m_what;

public:
    Error( std::string what ) : m_what( what )
    {
    }

    virtual const char * what() const noexcept final;
};

namespace error
{
class OperationUnsupportedInBackend : public Error
{
public:
    std::string backend;
    OperationUnsupportedInBackend( std::string backend_in, std::string what );
};
}
}
