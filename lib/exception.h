// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef _BOINC_EXCEPTIONS_
#define _BOINC_EXCEPTIONS_

#ifndef _WIN32
#include <string>
#endif

#include <exception>

// BOINC Base Exception Class
//

class boinc_base_exception : public std::exception
{

private:
    char    m_szConversionBuffer[66];   // convert numbers to strings largest
                                        //   datatype is 64bits plus the
                                        //   possible sign character and the
                                        //   NULL terminator
    std::string  m_strErrorBuffer;           // the formatted error message when
                                        //   asked

    std::string  m_strErrorType;             // the error type
    long    m_lErrorValue;              // the error value
    std::string  m_strErrorMessage;          // the error message
    std::string  m_strErrorData;             // any relevant data associated with
                                        //   the error
    std::string  m_strFilename;              // the file in which the error occurred
    long    m_lLineNumber;              // the line number in which the error
                                        //   occurred

public:

    boinc_base_exception(const char *s=0)
        : m_strErrorType(ErrorType()), m_lErrorValue(ErrorValue()), m_strErrorMessage(ErrorMessage()), m_strErrorData(s){};

    boinc_base_exception(const char *f, int l, const char *s=0)
        : m_strErrorType(ErrorType()), m_lErrorValue(ErrorValue()), m_strErrorMessage(ErrorMessage()), m_strErrorData(s), m_strFilename(f), m_lLineNumber(l){};

    boinc_base_exception(const char *et, long ev, const char *em, const char *s=0)
        : m_strErrorType(et), m_lErrorValue(ev), m_strErrorMessage(em), m_strErrorData(s){};

    boinc_base_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : m_strErrorType(et), m_lErrorValue(ev), m_strErrorMessage(em), m_strErrorData(s), m_strFilename(f), m_lLineNumber(l){};

    ~boinc_base_exception() throw () {};

    virtual const char * ErrorType();
    virtual const char * ErrorMessage();
    virtual long         ErrorValue();

    virtual const char * what();
};


// BOINC Base Runtime Exception Class
//

class boinc_runtime_base_exception : public boinc_base_exception
{
public:

    boinc_runtime_base_exception(const char *s=0)
        : boinc_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), s){};
    boinc_runtime_base_exception(const char *f, int l, const char *s=0)
        : boinc_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), f, l, s){};
    boinc_runtime_base_exception(const char *et, long ev, const char *em, const char *s=0)
        : boinc_base_exception(et, ev, em, s){};
    boinc_runtime_base_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : boinc_base_exception(et, ev, em, f, l, s){};

    ~boinc_runtime_base_exception() throw () {};

    virtual const char * ErrorType();
    virtual const char * ErrorMessage();
};


// BOINC Runtime Exceptions
//

class boinc_out_of_memory_exception : public boinc_runtime_base_exception
{
public:
    boinc_out_of_memory_exception(const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), s){};
    boinc_out_of_memory_exception(const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), f, l, s){};
    boinc_out_of_memory_exception(const char *et, long ev, const char *em, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, s){};
    boinc_out_of_memory_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, f, l, s){};

    ~boinc_out_of_memory_exception() throw () {};

    virtual const char * ErrorMessage();
    virtual long         ErrorValue();
};


class boinc_invalid_parameter_exception : public boinc_runtime_base_exception
{
public:
    boinc_invalid_parameter_exception(const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), s){};
    boinc_invalid_parameter_exception(const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), f, l, s){};
    boinc_invalid_parameter_exception(const char *et, long ev, const char *em, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, s){};
    boinc_invalid_parameter_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, f, l, s){};

    ~boinc_invalid_parameter_exception() throw () {};

    virtual const char * ErrorMessage();
    virtual long         ErrorValue();
};


class boinc_file_operation_exception : public boinc_runtime_base_exception
{
public:
    boinc_file_operation_exception(const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), s){};
    boinc_file_operation_exception(const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), f, l, s){};
    boinc_file_operation_exception(const char *et, long ev, const char *em, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, s){};
    boinc_file_operation_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, f, l, s){};

    ~boinc_file_operation_exception() throw () {};

    virtual const char * ErrorMessage();
    virtual long         ErrorValue();
};


class boinc_signal_operation_exception : public boinc_runtime_base_exception
{
public:
    boinc_signal_operation_exception(const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), s){};
    boinc_signal_operation_exception(const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(ErrorType(), ErrorValue(), ErrorMessage(), f, l, s){};
    boinc_signal_operation_exception(const char *et, long ev, const char *em, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, s){};
    boinc_signal_operation_exception(const char *et, long ev, const char *em, const char *f, int l, const char *s=0)
        : boinc_runtime_base_exception(et, ev, em, f, l, s){};

    ~boinc_signal_operation_exception() throw () {};

    virtual const char * ErrorMessage();
    virtual long         ErrorValue();
};

#endif
