#ifndef IO_EXCEPTION_
#define IO_EXCEPTION_

#include <exception>
#include <string>

using namespace std;

/** An IO exception that automatically gets C error number from the moment the
 * exception is created, given a helper message.
 */
class IOException: public exception {
    private:
        string context_;
        int c_errno_;
        string message;
    public:
        /** Constructs the IO Exception with a contextual message */
        IOException(string context__);
        /** Returns the C error number collected during the creation of the
         * exception
         */
        int c_errno() const;
        /** Returns the context message.
         */
        string const& context() const;
        virtual char const *what() const noexcept;
};

#endif
