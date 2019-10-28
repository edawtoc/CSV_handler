/*
 * File:   CsvEntryElement.hpp
 * Author: dawidtoczek
 *
 * Created on 3 stycznia 2019, 16:52
 */
#ifndef CSVENTRYELEMENT_HPP
#define CSVENTRYELEMENT_HPP

#include <sstream>
#include <string.h>

class CsvEntryElement {
public:

    CsvEntryElement() {
        _isSet = false;
    }

    virtual ~CsvEntryElement() {
    }

    virtual std::string getStringValue() {
        return nullptr;
    }

    bool isSet() {
        return _isSet;
    }

    void notSet() {
        _isSet = false;
    }

protected:
    bool _isSet;
};

template< typename T>
class CsvTypedEntryElement : public CsvEntryElement {
public:

    CsvTypedEntryElement() : CsvEntryElement() {
        _isSet = false;
    }

    virtual ~CsvTypedEntryElement() {
    }

    void setValue(const T& value) {
        _value = value;
        _isSet = true;
    }

    void setValue(const char* value) {
        _value = value;
        _isSet = true;
    }

    T getValue() {
        return _value;
    }

    virtual std::string getStringValue() override {
        std::stringstream typess;
        typess << _value;
        return typess.str();
    }

private:
    T _value;
};

#endif /* CSVENTRYELEMENT */

