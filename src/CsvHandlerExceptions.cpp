#include "CsvHandlerExceptions.hpp"

const char UnableToConvertFieldTypeException::_invalidArgumentMsg[] =
        ". Invallid argument exception has been thrown!";
const char UnableToConvertFieldTypeException::_indexOutOfRangeMsg[] =
        ". Out of range exception has been thrown!";
const char UnableToConvertFieldTypeException::_typeNotCorrectMsg[] =
        ". Probably field type is not correct!";

const char UnableToConvertFieldTypeException::_convertIntErrorMsg[] =
        "Error while converting field to int. Field value: ";
const char UnableToConvertFieldTypeException::_convertDoubleErrorMsg[] =
        "Error while converting field to double. Field value: ";
const char UnableToConvertFieldTypeException::_convertDateErrorMsg[] =
        "Error while converting field to date";

const char UserDefinedTypesValidationResult::_columnsNumberNotMatchMsg[] =
        "Numbers of columns does not match. Provided/actual : ";
const char UserDefinedTypesValidationResult::_typesCorrectMsg[] =
        "Types are correct.";
const char UserDefinedTypesValidationResult::_typeConsideredNotCorrect[] =
        "Type provided for column is considered not correct. Column: ";
const char UserDefinedTypesValidationResult::_recommendedType[] =
        " .Recommended type: ";
const char UserDefinedTypesValidationResult::_providedType[] =
        ". Provided type: ";

const char UnableToSplitEntryException::_unableToSplitMsg[] =
        "Error while splitting entry into columns. Affected entry: ";

const char UnableToReadFileInChunks::_unableToReadInChunksMsg[] =
        "Unable to read file in chunks. File size is too small. Read whole file"
        " at once. Input file size is: ";
