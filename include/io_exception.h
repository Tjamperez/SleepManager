#ifndef IO_EXCEPTION_
#define IO_EXCEPTION_

#include <exception>
#include <string>

using namespace std;

class IOException: public exception {
    private:
        string context_;
        int c_errno_;
        string message;
    public:
        IOException(string context__);
        int c_errno() const;
        string const& context() const;
        virtual char const *what() const noexcept;
};

#endif
