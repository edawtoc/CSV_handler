/*
 * File:   CsvEntryElement.hpp
 * Author: dawidtoczek
 *
 * Created on 3 stycznia 2019, 16:52
 */

#ifndef CSVHANDLEREXCEPTIONS_HPP
#define CSVHANDLEREXCEPTIONS_HPP

#include <string>
#include <locale>
#include <sstream>
#include <iostream>

class InvalidColumnCaptionException : public std::exception {
public:

    const char* _msg;

    virtual const char * what() const throw () {
        return _msg;
    }

    InvalidColumnCaptionException(const char * msg) {
        _msg = msg;
    }

    InvalidColumnCaptionException() {
        _msg = "Column caption is not valid";
    }

};

class HeaderNotAvailableException : public std::exception {
public:

    virtual const char * what() const throw () {
        return "Header is not available!";
    }
};

class UnableToOpenFileException : public std::exception {
public:

    virtual const char * what() const throw () {
        return "File can not be opened. Ensure that provided file is correct.";
    }
};

class UnableToConvertFieldTypeException : public std::exception {
private:

    std::string _msg;

public:

    virtual const char * what() const throw () {
        return _msg.c_str();
    }

    static const char _invalidArgumentMsg[];
    static const char _indexOutOfRangeMsg[];
    static const char _typeNotCorrectMsg[];

    static const char _convertIntErrorMsg[];
    static const char _convertDoubleErrorMsg[];
    static const char _convertDateErrorMsg[];

    UnableToConvertFieldTypeException(const std::string& msg) {
        _msg = msg;
    }

    UnableToConvertFieldTypeException() {
        _msg = "Type conversion can not be performed";
    }

};

class UnableToSplitEntryException : public std::exception {
private:

    std::string _msg;
    static const char _unableToSplitMsg[];

public:

    virtual const char * what() const throw () {
        return _msg.c_str();
    }

    UnableToSplitEntryException(long long entryIndex) {
        std::stringstream ei;
        ei << entryIndex;
        _msg += _unableToSplitMsg;
        _msg += ei.str();
    }
};

class UnableToReadFileInChunks : public std::exception {
private:

    std::string _msg;
    static const char _unableToReadInChunksMsg[];

public:

    virtual const char * what() const throw () {
        return _msg.c_str();
    }

    UnableToReadFileInChunks(long fileStreamSize) {
        std::stringstream ei;
        ei << fileStreamSize;
        _msg += _unableToReadInChunksMsg;
        _msg += ei.str();
    }
};

class UserDefinedTypesValidationResult {
private:

    bool _isValid;
    std::string _msg;

public:

    static const char _columnsNumberNotMatchMsg[];
    static const char _typesCorrectMsg[];
    static const char _typeConsideredNotCorrect[];
    static const char _recommendedType[];
    static const char _providedType[];

    UserDefinedTypesValidationResult(bool isValid, std::string msg) {
        _msg = msg;
        _isValid = isValid;
    }

    bool isValid() {
        return _isValid;
    }

    std::string getMessage() {
        return _msg;
    }
};

#endif /* CSVHANDLEREXCEPTIONS */
