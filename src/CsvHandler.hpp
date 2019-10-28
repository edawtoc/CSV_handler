/*
 * File:   CsvHandler.hpp
 * Author: dawidtoczek
*/

#ifndef CSVHANDLER_HPP
#define CSVHANDLER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include "CsvEntryElement.hpp"
#include "CsvHandlerExceptions.hpp"

namespace csvh {

    typedef std::vector<std::string> csv_entryLine;
    typedef CsvTypedEntryElement<std::string> csv_stringField;
    typedef CsvTypedEntryElement<double> csv_doubleField;
    typedef CsvTypedEntryElement<int> csv_intField;
    typedef CsvTypedEntryElement<std::time_t> csv_timeField;
    typedef std::vector<CsvEntryElement*> csv_column;
    typedef CsvEntryElement* csv_genericField;
    typedef std::vector<csv_entryLine> csv_entryLines;
    /**
     * Enum to represent column data type.
     */
    enum _dataTypes {
        type_double,
        type_int,
        type_string,
        type_date
    };

    enum _headerMode {
        include_header,
        skip_header,
        no_header
    };

    enum _errorHandlingMode {
        ignore_errors,
        stop_on_error
    };

    enum _loadDataMode {
        load_in_chunks,
        load_whole_file
    };

    enum _fileFormat {
        CSV,
        JSON
    };

    class CsvHandler {
    public:

        /**
         * Main constructor for initializing CSV Handler library.
         *
         * @param fileName - data source
         * @param loadDataMode - by default load_whole_file into memory
         * @param fileFormat - CSV / JSON
         * @param delimiter - default delimiter is ','
         * @param headerMode - by default no_header
         */
        CsvHandler(const std::string& inFileName,
                _loadDataMode loadDataMode = load_whole_file,
                _fileFormat fileFormat = CSV,
                char delimiter = ',',
                _headerMode headerMode = no_header);

        ~CsvHandler();

        /**
         * Method is used to print csv in-memory data to standard output.
         */
        void printDataOnScreen();

        /**
         * Method is used to print csv column types
         * in-memory data to standard output.
         */
        void printDataTypesOnScreen();

        /**
         * Method is used to print csv header in-memory data to standard output.
         */
        void printHeaderOnScreen();

        /**
         * Method is used to save csv in-memory data to file.
         *
         * @param outFileName
         * @param outFormat - default CSV
         * @param delimiter - default delimiter is ','.
         *        Parameter is not needed for JSON
         */
        void storeDataInFile(const std::string& outFileName,
                _fileFormat outFormat = CSV, char delimiter = ',');

        /**
         * Method is used to add quotation marks to all string-type
         * fields in storage.
         * Quotation marks will not be added if field is already quotted.
         */
        void surroundStringFieldsWithQuotationMarks();

        /**
         * Method is used to add quotation marks to all fields in vector.
         * Quotation marks will not be added if field is already quotted.
         *
         * @param vector
         */
        void surroundFieldsInVectorWithQuotationMarks(
                std::vector<std::string>& vector);

        /**
         * Method is used to provide user defined data type for all columns.
         * Available types are:
         * - type_double
         * - type_int
         * - type_string
         * - type_date
         *
         * @param amountOfColumns
         * @param dataTypes as vararg (comma separated types)
         */
        void provideTypesForColumns(unsigned int amountOfColumns, _dataTypes dataTypes...);

        /**
         * Method is used to provide user defined data type for all columns.
         * Available types are:
         * - type_double
         * - type_int
         * - type_string
         * - type_date
         *
         * @param amountOfColumns
         * @param dataTypes as a table
         */
        void provideTypesForColumns(unsigned int amountOfColumns, _dataTypes * dataTypes);

        /**
         * Method is used to validate if provided data types
         * for columns are correct.
         *
         * @param amountOfColumns
         * @param dataTypes as vararg (comma separated types)
         * @return UserDefinedTypesValidationResult
         */
        UserDefinedTypesValidationResult validateTypesForColumns(
                int amountOfColumns, _dataTypes dataTypes...);

        /**
         * Method is used to validate if provided data types
         * for columns are correct.
         *
         * @param amountOfColumns
         * @param dataTypes as a table
         * @return UserDefinedTypesValidationResult
         */
        UserDefinedTypesValidationResult validateTypesForColumns(
                unsigned int amountOfColumns, _dataTypes * dataTypes);

        /**
         * Method is used to validate if provided data types
         * for columns are correct.
         *
         * @param amountOfColumns
         * @param dataTypes as csv_entryLine (vector of strings)
         * @return UserDefinedTypesValidationResult
         */
        UserDefinedTypesValidationResult validateTypesForColumns(
                unsigned int amountOfColumns, csv_entryLine & dataTypes);

        /**
         * Method is used to read entries from file.
         * It can be used in while loop to read and modify whole file by chunks.
         *
         * @param errorHandlingMode - to decide if loading should proceed even
         * if errors occur. By default it stops on conversion errors.
         * @return true till end of file is reached.
         */
        bool loadEntries(_errorHandlingMode errorHandlingMode =
                stop_on_error);

        /**
         * Method is used to fetch selected field from csv file.
         *
         * @param columnIndex
         * @param rowIndex
         * @return CsvEntryElement - field
         */
        CsvEntryElement * getField(int columnIndex, int rowIndex);

        /**
         * Method is used to fetch selected field from csv file.
         *
         * @param columnCaption
         * @param rowIndex
         * @return CsvEntryElement - field
         */
        CsvEntryElement * getField(std::string columnCaption, int rowIndex);

        /**
         * Method is used to fetch selected row from csv file.
         *
         * @param rowIndex
         * @return csv_entryLine - row as vector of strings
         */
        csv_entryLine getRow(int rowIndex);


        /**
         * Method is used to fetch selected column from csv file.
         *
         * @param columnIndex
         * @return vector of column elements.
         */
        std::vector<CsvEntryElement*> getColumn(int columnIndex);

        /**
         * Method is used to fetch selected column from csv file.
         *
         * @param columnCaption
         * @return vector of column elements.
         */
        std::vector<CsvEntryElement*> getColumn(std::string columnCaption);

        /**
         * Method is used to get column ID by caption
         *
         * @param columnCaption
         * @return column ID
         */
        int getColumnId(std::string columnCaption);

        /**
         * Method is used to remove selected column.
         *
         * @param columnIndex
         */
        void removeColumn(int columnIndex);

        /**
         * Method is used to remove selected column.
         *
         * @param columnCaption
         */
        void removeColumn(std::string columnCaption);

        /**
         * Method is used to remove row.
         *
         * @param rowIndex
         */
        void removeRow(int rowIndex);

        /**
         * Method is used to instert row on selected position.
         * By default row will be added at the end.
         * Data conversion will be done automatically.
         *
         * @param entry as vector of strings
         * @param pos - position where row should be added. By default the last.
         * @param errorHandlingMode - to decide if loading should proceed even
         * if errors occur. By default it stops on conversion errors
         */
        void insertRow(csv_entryLine entry, int pos = -1,
                _errorHandlingMode errorHandlingMode = stop_on_error);

        /**
         * Method is used to instert row on selected position.
         * By default row will be added at the end.
         * Data conversion will be done automatically.
         *
         * @param entry as delimiter-separated string
         * @param pos - position where row should be added. By default the last
         * @param errorHandlingMode - to decide if loading should proceed even
         * if errors occur. By default it stops on conversion errors
         */
        void insertRow(std::string entry, int pos = -1,
                _errorHandlingMode errorHandlingMode = stop_on_error);

        /**
         * Method is used to add column filled with data.
         * By default column will be added at the end.
         * Default type of data is string.
         *
         * @param columnVector - vector that contains data
         * @param type - type of the data in the column
         * @param pos - position where new cpolumn should be added. By default
         * the last
         */
        void insertColumn(std::vector<CsvEntryElement*>& columnVector,
                _dataTypes type = type_string, int pos = -1);

        /**
         * Method is used to add column filled with data.
         * By default column will be added at the end.
         * Default type of data is string.
         *
         * @param columnVector - vector that contains data
         * @param caption - will be included in header
         * @param type - type of the data in the column
         * @param pos - position where new cpolumn should be added. By default
         * the last
         */
        void insertColumn(std::vector<CsvEntryElement*>& columnVector,
                const std::string& caption, _dataTypes type = type_string,
                int pos = -1);

        /**
         * Method is used to add an empty column.
         * By default column will be added at the end.
         * Default type of data is string.
         *
         * @param type - type of the data in the column
         * @param pos - position where new cpolumn should be added. By default
         * the last
         */
        void insertColumn(_dataTypes type = type_string, int pos = -1);

        /**
         * Method is used to add an empty column.
         * By default column will be added at the end.
         * Default type of data is string.
         *
         * @param caption - will be included in header
         * @param type - type of the data in the column
         * @param pos - position where new cpolumn should be added. By default
         * the last
         */
        void insertColumn(const std::string& caption,
                _dataTypes type = type_string, int pos = -1);

        /**
         * Method is used to evaluate values regarding provided regular expression.
         * Groups can be used.
         *
         * @param columnPos - column position [0 - (getAmountOfColumns()-1)]
         * @param regex - regular expression
         * @param replacement - groups can be used
         * @return number of changed entries
         */
        long long replaceAll(int columnPos,
                std::string regex, std::string replacement);

        /**
         * Method is used to evaluate values regarding provided regular expression.
         * Groups can be used.
         *
         * @param columnCaption
         * @param regex - regular expression
         * @param replacement - groups can be used
         * @return number of changed entries
         */
        long long replaceAll(std::string columnCaption,
                std::string regex, std::string replacement);

        /**
         * Method is used to find all fields matching regular expression.
         *
         * @param columnPos
         * @param regex - regular expression
         * @return all matching fields as csv_Column
         */
        csv_column findAll(int columnPos, std::string regex);

        /**
         * Method is used to find all fields matching regular expression.
         *
         * @param columnCaption
         * @param regex - regular expression
         * @return all matching fields as csv_Column
         */
        csv_column findAll(std::string columnCaption, std::string regex);

        /**
         * Method is used to find all rows where column value matches
         * regular expression.
         *
         * @param columnPos
         * @param regex - regular expression
         * @return all matching fields as csv_entryLines
         */
        csv_entryLines findAllRows(int columnPos, std::string regex);

        /**
         * Method is used to find all rows where column value matches
         * regular expression.
         *
         * @param columnCaption
         * @param regex - regular expression
         * @return all matching fields as csv_entryLines
         */
        csv_entryLines findAllRows(std::string columnCaption, std::string regex);

        /**
         * Method is used to split line by delimiter.
         *
         * @param lineToSplit
         * @param splittedLine
         */
        void splitEntryByDelimiter(std::string& lineToSplit,
                csv_entryLine & splittedLine, char delimiter);

        /**
         * Method returns number of currently loaded entries.
         *
         * @return number of entries
         */
        long long getAmountOfEntries();

        /**
         * Method returns number of columns in loaded chunk.
         *
         * @return number of columns
         */
        int getAmountOfColumns();


    private:
        // ========== Input file properties ====================================
        std::string _inFileName;
        char _csvDelimiter;
        char _inFileLineEnding;
        long long _entriesInCurrentChunk;
        std::vector<std::string> _inFileColumnTypes;
        std::vector<std::string> _inFileHeader;

        _headerMode _headerModeFlag;
        _loadDataMode _loadDataModeFlag;
        _fileFormat _inFileFormatFlag;
        size_t sizeType;
        //======================================================================

        // ========== Properities used for chunk file read =====================
        /**
         * Flag is set when latst file chunk is read.
         */
        bool _eofFlag;

        /**
         * Stream size for whole file.
         */
        long long _inFileStreamSize;

        /**
         * Position in file when reading was stopped.
         */
        long _inFileReadLastPosition;

        /**
         * Number of line starting the current chunk.
         */
        long long _absoluteBeginningIndex;

        /**
         * Number of last line that was read.
         */
        long long _absoluteEndingIndex;

        /**
         * Number of file chunks that was already read.
         */
        long _chunksCount;

        /**
         * Buffer leftovers is used while entry starts
         * at the end of one chunk and ends at the end of the second chunk.
         */
        std::string _buffLeftovers;

        /**
         * Buffer size for file chunks.
         * Should consider user defined buffer size.
         */
        long long _readBufferSize = 1024 * 1024 * 32;

        // =====================================================================


        // ========== JSON handling ============================================

        const int _jsonProperty = 0;
        const int _jsonValue = 1;

        // =====================================================================

        /**
         * Storage for source file data
         */
        std::vector<std::vector < CsvEntryElement*>> _sourceFileVector;
        std::vector<std::string> _sourceFileHeader;
        std::vector<std::string> _sourceFileColumnTypes;

        /**
         * Const values used in text parsing.
         */
        const char* _emptyString = "";
        const char* _zeroChar = "0";
        const char* _whitespace = " \t";
        const char _quotationMark = '"';
        const char _space = ' ';
        const char _tab = '\t';
        const char _CR = '\r';
        const char _LF = '\n';
        bool _CRLF;

        /**
         * For JSON
         */
        const char _rightBrace = '}';
        const char _leftBrace = '{';
        const char _rightSquare = ']';
        const char _leftSquare = '[';
        const char _comma = ',';
        const char _colon = ':';
        const char* _tabSpaces = "    ";

        /**
         * Const values used in type checks.
         */
        const std::string _tString = "type_string";
        const std::string _tDouble = "type_double";
        const std::string _tInt = "type_int";
        const std::string _tDate = "type_date";
        const char* _defaultDTFormat = "%Y-%m-%d %H:%M:%S";

        /**
         * std::map used to map data types strings to enum
         */
        std::map<std::string, _dataTypes> _dataTypesMap;

        /**
         * Method is used to initialize object variables with default values.
         */
        void classInitializer();

        /**
         * Methods are used to clear all created field objects.
         */
        void clearStorage();
        void clearHeader();
        void clearDataTypes();

        /**
         * Method is used to fetch the numer of entries in csv file.
         *
         * @return numer of entries.
         */
        int fetchNumberOfCSVEntries();

        /**
         * Method is used to fetch entry containing data types
         * for whole columns.
         *
         * @return std::vector containing data types for columns as std::string
         */
        csv_entryLine fetchDataTypesForColumns();

        /**
         * Method is used to automatic fetch of data type for all columns.
         */
        void autoDetectTypesForColumns();

        /**
         * Method is used to automatic fetch of data type for all columns
         * from provided reference line
         *
         * @param firstLineElements
         */
        void autoDetectTypesForColumns(csv_entryLine& referenceLine);

        /**
         * Method is used to fetch streamsize of source file
         *
         * @return streamsize
         */
        long long fetchFileStreamSize();

        std::vector<std::string> convertCharBufferIntoCSVentryStrings(
                std::vector<char> partialData);
        std::vector<std::string> convertCharBufferIntoJSONentryStrings(
                std::vector<char> partialData);
        /**
         * Method is used to parse first line of CSV file as a header.
         */
        bool loadHeader(std::string & line);
        bool loadHeader(std::vector<std::string> entryLine);
        /**
         * Method is used to get next chunk of entries specified by:
         * const size_t _readBufferSize
         */
        void loadChunkOfFile(std::vector<char>& data);

        void loadEntries_CSV(std::vector<char>& data,
                _errorHandlingMode errMode);

        void loadEntries_JSON(std::vector<char>& data,
                _errorHandlingMode errMode);

        /**
         * Method is used to prepare storage for holding source file in memory.
         */
        void initializeStorage();

        /**
         * Method is used to initialize column vector for given columnId.
         *
         * @return std::vector containig initialized column.
         */
        template<class T >
        std::vector<CsvEntryElement*> initializeEntriesForColumn();

        /**
         * Method is used to initialize storage for new row.
         *
         * @param newEntryPos
         */
        void initializeNewEntry(int newEntryPos);

        /**
         * Method is used to initialize storage for new column.
         *
         * @param type
         */
        std::vector<CsvEntryElement*> initializeNewColumn(_dataTypes type);

        /**
         * Method is used to initialize storage for new column.
         *
         * @param type
         * @param newEntryPos
         * @param caption
         */
        void initializeNewColumn(_dataTypes type, int newColumnPos,
                const std::string & caption);

        /**
         * Method is used to determine line ending character.
         *
         * @return line ending character
         */
        char determineLineEnding();

        /**
         * Method is used to emplace entries in storage.
         *
         * @param entryLines
         * @param fileFormat - CSV / JSON
         * @param errorHandlingMode
         */
        void emplaceEntriesInStorage(std::vector<std::string>& entryLines,
                _fileFormat fileFormat, _errorHandlingMode errorHandlingMode);

        /**
         * Method is used to parse JSON entry to entryLine
         *
         * @param jsonEntry i.e. { "property1" : value1, "property2" : value2 }
         * @param entryLine
         * @param jsonFieldType - property name / value
         * @param hm - header mode
         */
        void buildEntryLineFromJSONentry(
                std::string& jsonEntry, csv_entryLine& entryLine,
                int jsonFieldType, _headerMode hm = include_header);

        /**
         * Method is used to fetch all properties names from JSON entry.
         *
         * @param jsonEntry
         * @param entryLine
         */
        void buildPropertyLineFromJSONentry(
                std::string& jsonEntry, csv_entryLine& entryLine);

        /**
         * Method is used to set values for whole entry.
         *
         * @param entry - containing values for all columns
         * @param entryIndex
         * @param errorHandlingMode
         */
        void setColumnsForEntry(csv_entryLine entry, int entryIndex,
                _errorHandlingMode errorHandlingMode);

        /**
         * Method is used to set value for single entry element.
         *
         * @param operateOn - entry element to be set.
         * @param value - new value of entry element
         */
        template <class V>
        void setSingleEntryElementValue(CsvEntryElement* operateOn, const V value);

        /**
         * Method is used to determine column type by parsing given
         * std::string value.
         *
         * @param value - to be parsed
         * @return recognized data type name as std::string
         */
        std::string determineColumnType(const std::string & value);

        /**
         * Method is used to store header to file.
         *
         * @param outFileName
         * @param delimiter
         * @param openMode
         */
        void storeHeaderInFile_CSV(const std::string& outFileName,
                char delimiter, std::ios::openmode openMode);

        /**
         * Method is used to store all currently loaded fields to file.
         *
         * @param outFileName
         * @param delimiter
         * @param openMode
         */
        void storeFieldsInFile_CSV(const std::string& outFileName,
                char delimiter, std::ios::openmode openMode);

        /**
         * Method is used to store all currently loaded fields to file.
         *
         * @param outFileName
         * @param openMode
         */
        void storeFieldsInFile_JSON(const std::string& outFileName,
                std::ios::openmode openMode);

        /**
         * Method is used to add quotation marks to string-type vector fields
         *
         * @param vector
         */
        void surroundFieldsInVectorWithQuotationMarks(
                std::vector<CsvEntryElement*>& vector);

        /**
         * Method is used to determine ending of double quotted field.
         *
         * @param lineToSplit
         * @param poss - starting position of quotted field
         * @param delimiter
         * @return ending position of double quotted field
         */
        int getDoubleQuottedFieldEnding(std::string& lineToSplit, int poss,
                char delimiter);

        /**
         * Method is used to emplace substring in provided vector
         *
         * @param lineToSplit
         * @param splittedLine - place holder for result
         * @param poss - starting position of the field
         * @param pose - ending position of the field (delimiter)
         */
        void emplaceSpacesFreeFieldInVector(std::string& lineToSplit,
                csv_entryLine& splittedLine, int& poss, int& pose);

        /**
         * Method is used to convert std::string to selected type.
         * Type could be selected by template argument.
         *
         * Supported types: int, double
         * Method throws invalid_argument exception while parsing
         * is not possible.
         *
         * @param toBeParsed
         * @param entryIndex
         * @return value of toBeParsed in selected type.
         */
        template <class T >
        T convertString(const std::string & toBeParsed, long long entryIndex);

        /**
         * Method is used to check if field is double quotted
         *
         * @param field
         * @return true if field is double quotted
         */
        inline bool isFieldDoubleQuotted(const std::string & field);

        /**
         * Method is used to check if CsvEntryElement is std::string type.
         *
         * @param columnId
         * @return true if column is std::string type.
         */
        inline bool isColumnStringType(int columnId);

        /**
         * Method is used to surround provided string with quotation marks.
         *
         * @param field
         */
        inline std::string addQuotationMarks(const std::string & field);

        /**
         * Method for printing parsing error message on cerr stream.
         *
         * @param type - destination type of conversion
         * @param col
         * @param entry
         */
        void printParsingErrorMessage(const std::string& type, int col, int entry);

        /**
         * Method is used to fetch data type string from _dataTypes.
         *
         * @param dataType
         * @return data type string
         */
        std::string getDataTypeAsString(_dataTypes dataType);
    };

}

#endif /* CSVHANDLER_HPP */
