/*
 * File:   CsvEntryElement.hpp
 * Author: dawidtoczek
 *
 * Created on 21 grudnia 2018, 16:52
 */

#include "CsvHandler.hpp"
#include <fstream>
#include <iomanip>
#include <cstdarg>
#include <fstream>
#include <limits>
#include <regex>

using namespace csvh;
extern std::ostream cerr;

CsvHandler::CsvHandler(const std::string& csvFileName,
        _loadDataMode loadDataMode,
        _fileFormat fileFormat,
        char delimiter,
        _headerMode headerMode) {
    _inFileName = csvFileName;
    _csvDelimiter = delimiter;

    _inFileFormatFlag = fileFormat;
    _headerModeFlag = headerMode;
    _loadDataModeFlag = loadDataMode;

    _entriesInCurrentChunk = 0;
    _inFileReadLastPosition = 0;
    _absoluteBeginningIndex = 0;
    _absoluteEndingIndex = 0;
    _CRLF = false;
    classInitializer();
}

void CsvHandler::classInitializer() {
    _dataTypesMap.insert(std::make_pair(_tDouble, type_double));
    _dataTypesMap.insert(std::make_pair(_tInt, type_int));
    _dataTypesMap.insert(std::make_pair(_tString, type_string));
    _dataTypesMap.insert(std::make_pair(_tDate, type_date));

    _inFileLineEnding = determineLineEnding();
    _inFileStreamSize = fetchFileStreamSize();

    if (_loadDataModeFlag == load_in_chunks) {
        _eofFlag = false;
        if (_readBufferSize > _inFileStreamSize)
            throw UnableToReadFileInChunks(_inFileStreamSize);
        _chunksCount = 0;
    } else {
        _eofFlag = true;
        _readBufferSize = _inFileStreamSize;
        _chunksCount = 1;
    }
}

CsvHandler::~CsvHandler() {
    clearStorage();
    clearHeader();
    clearDataTypes();
}

void CsvHandler::clearStorage() {
    while (!_sourceFileVector.empty()) {
        while (!_sourceFileVector.back().empty()) {
            delete _sourceFileVector.back().back();
            _sourceFileVector.back().pop_back();
        }
        _sourceFileVector.pop_back();
    }
}

void CsvHandler::clearHeader() {
    while (!_sourceFileHeader.empty()) {
        _sourceFileHeader.pop_back();
    }
}

void CsvHandler::clearDataTypes() {
    while (!_sourceFileColumnTypes.empty()) {
        _sourceFileColumnTypes.pop_back();
    }
}

namespace csvh {

	template<>
	int CsvHandler::convertString<int>(const std::string & toBeParsed, long long entryIndex) {
		std::stringstream msg;
		msg << "Row " << entryIndex << _colon << _space;
		msg << UnableToConvertFieldTypeException::_convertIntErrorMsg << toBeParsed;
		try {
			int parsedValue = std::stoi(toBeParsed, &sizeType);
			if (sizeType == toBeParsed.size()) {
				return parsedValue;
			}
		} catch (const std::invalid_argument &exc) {
			msg << UnableToConvertFieldTypeException::_invalidArgumentMsg;
			throw UnableToConvertFieldTypeException(msg.str());
		} catch (const std::out_of_range &exc) {
			msg << UnableToConvertFieldTypeException::_indexOutOfRangeMsg;
			throw UnableToConvertFieldTypeException(msg.str());
		}
		msg << UnableToConvertFieldTypeException::_typeNotCorrectMsg;
		throw UnableToConvertFieldTypeException(msg.str());
	}

	template<>
	double CsvHandler::convertString<double>(const std::string & toBeParsed, long long entryIndex) {
		std::stringstream msg;
		msg << "Row " << entryIndex << _colon << _space;
		msg << UnableToConvertFieldTypeException::_convertDoubleErrorMsg
				<< toBeParsed;
		try {
			double parsedValue = std::stod(toBeParsed, &sizeType);
			if (sizeType == toBeParsed.size()) {
				return parsedValue;
			}
		} catch (const std::invalid_argument &exc) {
			msg << UnableToConvertFieldTypeException::_invalidArgumentMsg;
			throw UnableToConvertFieldTypeException(msg.str());
		} catch (const std::out_of_range &exc) {
			msg << UnableToConvertFieldTypeException::_indexOutOfRangeMsg;
			throw UnableToConvertFieldTypeException(msg.str());
		}
		msg << UnableToConvertFieldTypeException::_typeNotCorrectMsg;
		throw UnableToConvertFieldTypeException(msg.str());
	}

	template<>
	std::time_t CsvHandler::convertString<std::time_t>(const std::string & toBeParsed,
													   long long entryIndex) {
		std::tm parsedValue;
		std::istringstream dateStream{toBeParsed};

		dateStream >> std::get_time(&parsedValue, _defaultDTFormat);
		std::time_t dt = std::mktime(&parsedValue);
		if (dt != -1) {
			return dt;
		}
		throw UnableToConvertFieldTypeException(
				UnableToConvertFieldTypeException::_convertDateErrorMsg);
	}
}

bool CsvHandler::loadHeader(std::string & line) {
    std::vector<std::string> splittedLine;
    splitEntryByDelimiter(line, splittedLine, _csvDelimiter);
    return loadHeader(splittedLine);
}

bool CsvHandler::loadHeader(std::vector<std::string> entryLine) {
    if (_chunksCount == 1) {
        if (_headerModeFlag == include_header) {
            _sourceFileHeader = entryLine;
            _inFileHeader = _sourceFileHeader;
            return true;
        } else if (_headerModeFlag == skip_header) {
            return true;
        }
    }
    return false;
}

void CsvHandler::autoDetectTypesForColumns() {
    std::string line;
    csv_entryLine firstLineElements;
    std::ifstream csvFileStrem(_inFileName);

    if (_headerModeFlag == include_header || _headerModeFlag == skip_header) {
        std::getline(csvFileStrem, line, _inFileLineEnding);
    }

    if (std::getline(csvFileStrem, line, _inFileLineEnding)) {
        splitEntryByDelimiter(line, firstLineElements, _csvDelimiter);
    }
    csvFileStrem.close();
    autoDetectTypesForColumns(firstLineElements);
}

void CsvHandler::autoDetectTypesForColumns(csv_entryLine & firstLineElements) {
    _sourceFileColumnTypes.reserve(firstLineElements.size());
    for (std::string entryElement : firstLineElements) {
        _sourceFileColumnTypes.emplace_back(determineColumnType(entryElement));
    }
    _inFileColumnTypes = _sourceFileColumnTypes;
}

void CsvHandler::provideTypesForColumns(unsigned int amountOfColumns, _dataTypes dataTypes...) {
    va_list types;
    va_start(types, dataTypes);
    int dt = dataTypes;
    _sourceFileColumnTypes.reserve(amountOfColumns);

    for (unsigned int columnId = 0; columnId < amountOfColumns; ++columnId) {
        _dataTypes dtE = static_cast<_dataTypes> (dt);
        _sourceFileColumnTypes.emplace_back(getDataTypeAsString(dtE));
        dt = va_arg(types, int);
    }
    va_end(types);
    _inFileColumnTypes = _sourceFileColumnTypes;
}

void CsvHandler::provideTypesForColumns(unsigned int amountOfColumns, _dataTypes * dataTypes) {
    for (unsigned int columnId = 0; columnId < amountOfColumns; ++columnId) {
        _sourceFileColumnTypes.emplace_back(getDataTypeAsString(dataTypes[columnId]));
    }
    _inFileColumnTypes = _sourceFileColumnTypes;
}

UserDefinedTypesValidationResult CsvHandler::validateTypesForColumns(
        int amountOfColumns, _dataTypes dataTypes...) {
    std::vector<std::string> tmpTypes;

    va_list types;
    va_start(types, dataTypes);
    int dt = dataTypes;
    tmpTypes.reserve(amountOfColumns);

    for (int columnId = 0; columnId < amountOfColumns; ++columnId) {
        _dataTypes dtE = static_cast<_dataTypes> (dt);
        tmpTypes.emplace_back(getDataTypeAsString(dtE));
        dt = va_arg(types, int);
    }
    va_end(types);

    return validateTypesForColumns(amountOfColumns, tmpTypes);
}

UserDefinedTypesValidationResult CsvHandler::validateTypesForColumns(
        unsigned int amountOfColumns, _dataTypes * dataTypes) {
    std::vector<std::string> tmpTypes;

    for (unsigned int columnId = 0; columnId < amountOfColumns; ++columnId) {
        tmpTypes.emplace_back(getDataTypeAsString(dataTypes[columnId]));
    }

    return validateTypesForColumns(amountOfColumns, tmpTypes);
}

UserDefinedTypesValidationResult CsvHandler::validateTypesForColumns(
        unsigned int amountOfColumns, csv_entryLine & dataTypes) {
    std::stringstream msg;
    bool isValid = true;
    autoDetectTypesForColumns();
    if (dataTypes.size() != _sourceFileColumnTypes.size()
            || amountOfColumns != _sourceFileColumnTypes.size()) {
        msg << UserDefinedTypesValidationResult::_columnsNumberNotMatchMsg
                << amountOfColumns << "/" << _sourceFileColumnTypes.size() << std::endl;
        isValid = false;
    }

    for (unsigned int colID = 0; colID < _sourceFileColumnTypes.size(); ++colID) {
        if (dataTypes[colID] != _sourceFileColumnTypes[colID]) {
            msg << UserDefinedTypesValidationResult::_typeConsideredNotCorrect
                    << colID
                    << UserDefinedTypesValidationResult::_recommendedType
                    << _sourceFileColumnTypes[colID]
                    << UserDefinedTypesValidationResult::_providedType
                    << dataTypes[colID]
                    << std::endl;
            isValid = false;
        }
    }
    _sourceFileColumnTypes.erase(_sourceFileColumnTypes.begin(), _sourceFileColumnTypes.end());
    if (isValid) {
        msg << UserDefinedTypesValidationResult::_typesCorrectMsg;
    }
    return UserDefinedTypesValidationResult(isValid, msg.str());
}

void CsvHandler::initializeStorage() {
    _sourceFileVector.reserve(_sourceFileColumnTypes.size());
    csv_entryLine::iterator typeIt = _sourceFileColumnTypes.begin();

    for (unsigned int columnId = 0; columnId < _sourceFileColumnTypes.size(); ++columnId) {
        _dataTypes dataColumnType = _dataTypesMap[*typeIt];

        switch (dataColumnType) {
            case type_double:
                _sourceFileVector.emplace_back(
                        initializeEntriesForColumn<csv_doubleField>());
                break;
            case type_int:
                _sourceFileVector.emplace_back(
                        initializeEntriesForColumn<csv_intField>());
                break;
            default:
                _sourceFileVector.emplace_back(
                        initializeEntriesForColumn<csv_stringField>());
                break;
        }
        ++typeIt;
    }
}

template<class EntryType >
std::vector<CsvEntryElement*> CsvHandler::initializeEntriesForColumn() {
    std::vector<CsvEntryElement*> columnEntriesVector;
    columnEntriesVector.reserve(_entriesInCurrentChunk);

    for (int entryId = 0; entryId < _entriesInCurrentChunk; ++entryId) {
        columnEntriesVector.emplace_back(new EntryType());
    }
    return columnEntriesVector;
}

char CsvHandler::determineLineEnding() {
    char currentChar;
    std::ifstream file(_inFileName, std::ios::binary);
    while (file >> std::noskipws >> currentChar)
        if (currentChar == _CR) {
            file >> std::noskipws >> currentChar;
            if (currentChar == _LF) {
                _CRLF = true;
                return _LF;
            }
            else return _CR;
        } else if (currentChar == _LF) {
            return _LF;
        }
    return _LF;
}

int CsvHandler::fetchNumberOfCSVEntries() {
    int counter = 0;
    std::ifstream csvFileStrem(_inFileName);
    std::string line;

    while (std::getline(csvFileStrem, line, _inFileLineEnding)) {
        if (line.size()) {
            ++counter;
        }
    }
    csvFileStrem.close();
    return counter;
}

long long CsvHandler::fetchFileStreamSize() {
    std::ifstream fstream(_inFileName.c_str());
    if(!fstream.good()) {
        throw UnableToOpenFileException();
    }

    fstream.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = fstream.gcount();
    fstream.clear();
    return length;
}

long long CsvHandler::getAmountOfEntries() {
    return _entriesInCurrentChunk;
}

int CsvHandler::getAmountOfColumns() {
    return _sourceFileColumnTypes.size();
}

std::string CsvHandler::determineColumnType(const std::string & value) {
    if (value.length() > 1 && value.at(0) == _quotationMark)
        return _tString;

    try {
        convertString<int>(value, -1);
        return _tInt;
    } catch (const UnableToConvertFieldTypeException &exc) {
        try {
            convertString<double>(value, -1);
            return _tDouble;
        } catch (const UnableToConvertFieldTypeException &exc) {
            try {
                convertString<std::time_t>(value, -1);
                return _tDate;
            } catch (const UnableToConvertFieldTypeException &exc) {
                return _tString;
            }
        }
    }
    return _tString;
}

bool CsvHandler::loadEntries(_errorHandlingMode errorHandlingMode) {
    if (_loadDataModeFlag == load_in_chunks) {
        if (_eofFlag) {
            _inFileReadLastPosition = _absoluteEndingIndex = _chunksCount = 0;
            return false;
        }
    } else _inFileReadLastPosition = _absoluteEndingIndex = 0;

    clearStorage();

    if (_loadDataModeFlag == load_in_chunks
            && (_chunksCount + 1) * _readBufferSize > _inFileStreamSize) {
        _readBufferSize = _inFileStreamSize - (_readBufferSize * _chunksCount);
        _eofFlag = true;
    }
    std::vector<char> data(_readBufferSize);
    loadChunkOfFile(data);
    if (_loadDataModeFlag == load_in_chunks) ++_chunksCount;
    if (_inFileFormatFlag == CSV) loadEntries_CSV(data, errorHandlingMode);
    else if (_inFileFormatFlag == JSON) loadEntries_JSON(data, errorHandlingMode);

    return true;
}

void CsvHandler::loadChunkOfFile(std::vector<char>& data) {
    std::ifstream fstream(_inFileName);

    if (fstream) {
        fstream.seekg(_inFileReadLastPosition);
        if (fstream.read(&data[0], _readBufferSize)) {
            _inFileReadLastPosition = fstream.tellg();
            return;
        }
    }
    throw UnableToOpenFileException();
}

void CsvHandler::loadEntries_CSV(std::vector<char>& data,
        _errorHandlingMode errMode) {

    if (_sourceFileColumnTypes.empty()) autoDetectTypesForColumns();
    std::vector<std::string> entryLines = convertCharBufferIntoCSVentryStrings(data);

    if (!entryLines.empty()) {
        if (loadHeader(entryLines.front())) entryLines.erase(entryLines.begin());
        _sourceFileColumnTypes = _inFileColumnTypes;
        _sourceFileHeader = _inFileHeader;
        _absoluteEndingIndex += entryLines.size();
        _absoluteBeginningIndex = _absoluteEndingIndex - entryLines.size();
        _entriesInCurrentChunk = entryLines.size();
        initializeStorage();
        emplaceEntriesInStorage(entryLines, CSV, errMode);
    }
}

void CsvHandler::loadEntries_JSON(std::vector<char>& data,
        _errorHandlingMode errMode) {

    std::vector<std::string> entryLines =
            convertCharBufferIntoJSONentryStrings(data);
    if (!entryLines.empty()) {

        std::vector<std::string> firstLine;

        if (_headerModeFlag == include_header) {
            buildPropertyLineFromJSONentry(entryLines.front(), firstLine);
            loadHeader(firstLine);
            firstLine.erase(firstLine.begin(), firstLine.end());
        }

        if (_sourceFileColumnTypes.empty()) {
            buildEntryLineFromJSONentry(entryLines.front(), firstLine,
                    _jsonValue, include_header);
            autoDetectTypesForColumns(firstLine);
            firstLine.erase(firstLine.begin(), firstLine.end());

        }

        _sourceFileColumnTypes = _inFileColumnTypes;
        _sourceFileHeader = _inFileHeader;
        _absoluteEndingIndex += entryLines.size();
        _absoluteBeginningIndex = _absoluteEndingIndex - entryLines.size();
        _entriesInCurrentChunk = entryLines.size();
        initializeStorage();
        emplaceEntriesInStorage(entryLines, JSON, errMode);
    }

}

std::vector<std::string> CsvHandler::convertCharBufferIntoCSVentryStrings(
        std::vector<char> partialData) {
    std::vector<std::string> lineBuff;
    std::string buff = _buffLeftovers;
    _buffLeftovers.clear();
    for (const char character : partialData) {
        if (character != _LF && character != _CR ) buff += character;
        else if (buff.size() && (buff.size() > _sourceFileColumnTypes.size() - 1)) {
            lineBuff.push_back(buff);
            buff.clear();
        }
    }
    if (buff.size()) {
        _buffLeftovers = buff;
        if (_eofFlag && (buff.size() > _sourceFileColumnTypes.size() - 1))
            lineBuff.push_back(buff);
    }
    return lineBuff;
}

std::vector<std::string> CsvHandler::convertCharBufferIntoJSONentryStrings(
        std::vector<char> partialData) {
    std::vector<std::string> lineBuff;
    std::regex rEntry(R"(\{[^\{]*\})");
    std::string buff = _buffLeftovers;
    std::smatch matches;

    for (const char character : partialData) {
        if (character != _LF && character != _CR )
            buff += character;
    }

    std::string::const_iterator searchStart(buff.cbegin());
    while (std::regex_search(searchStart, buff.cend(), matches, rEntry)) {
        std::string m = matches[0].str();
        m = m.substr(1, m.length() - 2);
        lineBuff.push_back(m);
        searchStart = matches.suffix().first;
    }

    _buffLeftovers = _emptyString;
    for (std::string::const_iterator it = searchStart; it != buff.cend(); ++it) {
        _buffLeftovers += *it;
    }
    return lineBuff;
}

void CsvHandler::emplaceEntriesInStorage(std::vector<std::string>& entryLines,
        _fileFormat fileFormat, _errorHandlingMode errorHandlingMode) {
    csv_entryLine splittedEntryHolder;
    splittedEntryHolder.reserve(_inFileColumnTypes.size());

    for (unsigned long cEntry = 0; cEntry < entryLines.size(); ++cEntry) {
        splittedEntryHolder.erase(splittedEntryHolder.begin(),
                splittedEntryHolder.end());
        if (fileFormat == CSV) {
            splitEntryByDelimiter(entryLines[cEntry], splittedEntryHolder,
                    _csvDelimiter);
        } else if (fileFormat == JSON) {
            buildEntryLineFromJSONentry(entryLines[cEntry], splittedEntryHolder,
                    _jsonValue, include_header);
        }
        if (splittedEntryHolder.size() == _inFileColumnTypes.size()) {
            setColumnsForEntry(splittedEntryHolder, cEntry, errorHandlingMode);
        } else if (errorHandlingMode == stop_on_error) {
            std::cerr << "Error in line: " << entryLines[cEntry] << "\nSplitted size: " << splittedEntryHolder.size() << std::endl;
            throw UnableToSplitEntryException(_absoluteBeginningIndex + cEntry);
        } else {
            std::cerr << "Rejected line: " << entryLines[cEntry] << std::endl;
            entryLines.erase(entryLines.begin() + cEntry);
            for (auto cCol = _sourceFileVector.begin(); cCol < _sourceFileVector.end(); ++cCol) {
                delete (*cCol)[cEntry];
                (*cCol).erase((*cCol).begin() + cEntry);
            }
            _entriesInCurrentChunk--;
            _absoluteEndingIndex--;
        }
    }
}

void CsvHandler::buildEntryLineFromJSONentry(
        std::string& jsonEntry, csv_entryLine& entryLine,
        int jsonFieldType, _headerMode hm) {
    std::vector<std::string> properties;
    std::vector<std::string> propertyToValue;
    propertyToValue.reserve(2);

    splitEntryByDelimiter(jsonEntry, properties, _comma);
    entryLine.reserve(properties.size());

    for (unsigned int i = 0; i < properties.size(); ++i)
        entryLine.emplace_back(_emptyString);
    int i = 0;
    for (std::string p : properties) {
        splitEntryByDelimiter(p, propertyToValue, ':');
        if (propertyToValue.size() == 2) {
            if (hm == no_header) {
                entryLine.at(i) += (propertyToValue.at(jsonFieldType));
            } else {
                int colPos = getColumnId(propertyToValue.at(_jsonProperty));
                entryLine.at(colPos) += propertyToValue.at(jsonFieldType);
            }
            propertyToValue.erase(propertyToValue.begin(), propertyToValue.end());
        }
        ++i;
    }
}

void CsvHandler::buildPropertyLineFromJSONentry(
        std::string& jsonEntry, csv_entryLine & entryLine) {
    buildEntryLineFromJSONentry(jsonEntry, entryLine, _jsonProperty, no_header);
}

void CsvHandler::setColumnsForEntry(csv_entryLine entry, int entryIndex,
        _errorHandlingMode errorHandlingMode) {
    int columnIndex = 0;
    for (std::string entryElement : entry) {
        _dataTypes dt =
                _dataTypesMap[_sourceFileColumnTypes[columnIndex]];

        switch (dt) {
            case type_double:
            {

                try {
                    setSingleEntryElementValue<double>(
                            _sourceFileVector[columnIndex][entryIndex],
                            convertString<double>(entryElement, entryIndex));
                } catch (const UnableToConvertFieldTypeException &exc) {
                    if (errorHandlingMode == ignore_errors) {
                        setSingleEntryElementValue<double>(
                                _sourceFileVector[columnIndex][entryIndex], 0.0);
                        _sourceFileVector[columnIndex][entryIndex]->notSet();
                    } else {
                        throw exc;
                    }
                }
                break;
            }
            case type_int:
            {
                try {
                    setSingleEntryElementValue<int>(
                            _sourceFileVector[columnIndex][entryIndex],
                            convertString<int>(entryElement, entryIndex));
                } catch (const UnableToConvertFieldTypeException &exc) {
                    if (errorHandlingMode == ignore_errors) {
                        setSingleEntryElementValue<int>(
                                _sourceFileVector[columnIndex][entryIndex], 0);
                        _sourceFileVector[columnIndex][entryIndex]->notSet();
                    } else {
                        throw exc;
                    }
                }
                break;
            }
            default:
            {
                setSingleEntryElementValue<std::string>(
                        _sourceFileVector[columnIndex][entryIndex], entryElement);
                break;
            }
        }
        ++columnIndex;
    }
}

void CsvHandler::printParsingErrorMessage(const std::string& type, int col, int entry) {
    std::cerr << "ERROR: Cannot parse value to " << type << "."
            << " Will leave column: " << col
            << " row: " << entry << " unset!\n";
}

template <class V>
void CsvHandler::setSingleEntryElementValue(
        CsvEntryElement* toBeSetted, const V value) {
    if (auto* te = dynamic_cast<CsvTypedEntryElement<V>*> (toBeSetted)) {
        te->setValue(value);
    } else {
        auto* tn = new csv_stringField();
        tn->setValue(_zeroChar);
        delete toBeSetted;
        toBeSetted = tn;
    }
}

void CsvHandler::splitEntryByDelimiter(std::string& lineToSplit,
        csv_entryLine & splittedLine, char delimiter) {
    int poss = 0;
    int pose = 0;

    while ((poss <= (int) lineToSplit.length()) && (pose != -1)) {
        if (poss == (int) lineToSplit.length()) {
            splittedLine.emplace_back(_emptyString);
            pose = -1;
        } else {
            poss = lineToSplit.find_first_not_of(_whitespace, poss);
            if (poss == -1) {
                splittedLine.emplace_back(_emptyString);
                pose = -1;
            } else if (lineToSplit[poss] == _quotationMark) {
                pose = getDoubleQuottedFieldEnding(lineToSplit, poss, delimiter);
                emplaceSpacesFreeFieldInVector(lineToSplit,
                        splittedLine, poss, pose);
            } else {
                pose = lineToSplit.find_first_of(delimiter, poss);
                emplaceSpacesFreeFieldInVector(lineToSplit,
                        splittedLine, poss, pose);
            }
            poss = pose + 1;
        }
    }
}

void CsvHandler::emplaceSpacesFreeFieldInVector(std::string& lineToSplit,
        csv_entryLine& splittedLine, int& poss, int& pose) {
    if (pose == -1) {
        int correctedPose = pose;
        if (lineToSplit[lineToSplit.length() - 1] == _space
                || lineToSplit[lineToSplit.length() - 1] == _tab) {
            int notSpaceFromLast =
                    lineToSplit.find_last_not_of(_whitespace, lineToSplit.length());
            correctedPose =
                    lineToSplit.find_first_not_of(_whitespace, notSpaceFromLast) + 1;
        }
        splittedLine.emplace_back(lineToSplit.substr(poss, correctedPose - poss));
    } else {
        int correctedPose = pose;
        if ((lineToSplit[pose - 1] == _space || lineToSplit[pose - 1] == _tab)
                && pose - poss > 1) {
            int notSpaceFromLast =
                    lineToSplit.find_last_not_of(_whitespace, pose - 1);
            correctedPose =
                    lineToSplit.find_first_not_of(_whitespace, notSpaceFromLast) + 1;
        }
        splittedLine.emplace_back(lineToSplit.substr(poss, correctedPose - poss));
    }
}

int CsvHandler::getDoubleQuottedFieldEnding(std::string& lineToSplit, int poss,
        char delimiter) {
    unsigned int nqMarks = 1;
    unsigned int posc = poss + 1;
    while (posc < lineToSplit.size() &&
            ((nqMarks % 2) || lineToSplit[posc] != delimiter)) {
        if (lineToSplit[posc] == _quotationMark) {
            ++nqMarks;
        }
        ++posc;
    }
    return posc;
}

void CsvHandler::printDataOnScreen() {
    if (!_sourceFileVector.empty()) {
        for (int currEntry = 0; currEntry < _entriesInCurrentChunk; ++currEntry) {
            for (int currCol = 0; currCol < (int) _sourceFileColumnTypes.size(); ++currCol) {
                std::cout << "|" << std::setw(15)
                        << _sourceFileVector[currCol][currEntry]->getStringValue();
            }
            std::cout << "|" << std::endl;
        }
    } else {
        std::cerr << "No data was loaded into memory!" << std::endl;
    }
}

void CsvHandler::printDataTypesOnScreen() {
    if (!_sourceFileColumnTypes.empty()) {
        for (auto it = _sourceFileColumnTypes.begin(); it != _sourceFileColumnTypes.end(); ++it) {
            std::cout << *it << " | ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "No data types was loaded into memory!" << std::endl;
    }
}

void CsvHandler::printHeaderOnScreen() {
    if (!_sourceFileHeader.empty()) {
        for (auto it = _sourceFileHeader.begin(); it != _sourceFileHeader.end(); ++it) {
            std::cout << *it << " | ";
        }
        std::cout << std::endl;
    }
}

void CsvHandler::storeDataInFile(const std::string& newCsvFileName,
        _fileFormat outFormat, char delimiter) {
    if (!_sourceFileVector.empty()) {
        if (outFormat == CSV) {
            storeHeaderInFile_CSV(newCsvFileName, delimiter, std::ios::ate | std::ios::binary);
            storeFieldsInFile_CSV(newCsvFileName, delimiter, std::ios::app | std::ios::binary);
        } else if (outFormat == JSON) {
            if (_chunksCount == 1) {
                storeFieldsInFile_JSON(newCsvFileName, std::ios::ate);
            } else {
                storeFieldsInFile_JSON(newCsvFileName, std::ios::app);

            }
        }
    } else {
        std::cerr << "No data was loaded into memory!" << std::endl;
    }
}

void CsvHandler::storeHeaderInFile_CSV(const std::string& newCsvFileName,
        char delimiter, std::ios::openmode openMode) {
    if (_chunksCount == 1) {
        std::ofstream file(newCsvFileName, openMode);
        if (!_sourceFileHeader.empty()) {
            auto hItem = _sourceFileHeader.begin();
            file << *hItem;
            ++hItem;
            for (; hItem != _sourceFileHeader.end(); ++hItem) {
                file << delimiter << *hItem;
            }
            if (_CRLF == true) file << _CR;
            file << _inFileLineEnding;
        }
        file.close();
    }
}

void CsvHandler::storeFieldsInFile_CSV(const std::string& newCsvFileName,
        char delimiter, std::ios::openmode openMode) {
    std::ofstream file(newCsvFileName, openMode);
    for (int currEntry = 0; currEntry < _entriesInCurrentChunk; ++currEntry) {
        if (_sourceFileVector.front().at(currEntry)->isSet()) {
            file << _sourceFileVector.front().at(currEntry)->getStringValue();
        }
        for (std::vector<std::vector < CsvEntryElement*>>::iterator columnIt =
                _sourceFileVector.begin() + 1;
                columnIt < _sourceFileVector.end(); ++columnIt) {
            file << delimiter;
            if ((*columnIt).at(currEntry)->isSet()) {
                file << (*columnIt).at(currEntry)->getStringValue();
            } else {
                file << "";
            }
        }
        if (_CRLF == true) file << _CR;
        file << _inFileLineEnding;
    }
    file.close();
}

void CsvHandler::storeFieldsInFile_JSON(const std::string& newCsvFileName,
        std::ios::openmode openMode) {
    if (_sourceFileHeader.empty()) throw HeaderNotAvailableException();

    surroundFieldsInVectorWithQuotationMarks(_sourceFileHeader);
    surroundStringFieldsWithQuotationMarks();

    std::ofstream file(newCsvFileName, openMode);
    if (_chunksCount == 1) file << _leftSquare;
    for (int cEntry = 0; cEntry < _entriesInCurrentChunk; ++cEntry) {
        if (_CRLF == true) file << _CR;
        file << _inFileLineEnding << _leftBrace;
        if (_CRLF == true) file << _CR;
        file << _inFileLineEnding;

        for (int colIndex = 0; colIndex < (int) _sourceFileColumnTypes.size(); ++colIndex) {
            file << _sourceFileHeader.at(colIndex) << " : ";
            if (_sourceFileVector.at(colIndex).at(cEntry)->isSet()) {
                file << _sourceFileVector.at(colIndex).at(cEntry)->getStringValue();
            } else file << _quotationMark << _quotationMark;
            if (colIndex < (int) _sourceFileColumnTypes.size() - 1) file << _comma;
            if (_CRLF == true) file << _CR;
            file << _inFileLineEnding;
        }
        file << _rightBrace;
        if (cEntry < _entriesInCurrentChunk - 1) file << _comma;
    }

    if (_eofFlag) {
        if (_CRLF == true) file << _CR;
        file << _inFileLineEnding << _rightSquare;
    }
    file.close();
}

void CsvHandler::initializeNewEntry(int newEntryPos) {
    csv_entryLine::iterator typeIt = _sourceFileColumnTypes.begin();

    for (int columnId = 0; columnId < (int) _sourceFileColumnTypes.size(); ++columnId) {
        _dataTypes dataColumnType = _dataTypesMap[*typeIt];

        switch (dataColumnType) {
            case type_double:
                _sourceFileVector[columnId].insert(
                        _sourceFileVector[columnId].begin() + newEntryPos,
                        new csv_doubleField);
                break;
            case type_int:
                _sourceFileVector[columnId].insert(
                        _sourceFileVector[columnId].begin() + newEntryPos,
                        new csv_intField);
                break;
            default:
                _sourceFileVector[columnId].insert(
                        _sourceFileVector[columnId].begin() + newEntryPos,
                        new csv_stringField);

                break;
        }
        ++typeIt;
    }
}

std::vector<CsvEntryElement*> CsvHandler::initializeNewColumn(_dataTypes type) {
    switch (type) {
        case type_double:
            return initializeEntriesForColumn<csv_doubleField>();
        case type_int:
            return initializeEntriesForColumn<csv_intField>();
        default:
            return initializeEntriesForColumn<csv_stringField>();
    }
}

std::string CsvHandler::getDataTypeAsString(_dataTypes dataType) {
    for (auto it = _dataTypesMap.begin(); it != _dataTypesMap.end(); ++it) {
        if (it->second == dataType) {
            return it->first;
        }
    }
    return _emptyString;
}

void CsvHandler::surroundStringFieldsWithQuotationMarks() {
    int currentColumnId = 0;

    for (std::vector<CsvEntryElement*> columnV : _sourceFileVector) {
        if (isColumnStringType(currentColumnId)) {
            surroundFieldsInVectorWithQuotationMarks(columnV);
        }
        currentColumnId++;
    }
}

void CsvHandler::surroundFieldsInVectorWithQuotationMarks(
        std::vector<CsvEntryElement*>& vect) {
    for (auto field : vect) {
        csv_stringField* typedField =
                dynamic_cast<csv_stringField*> (field);
        if (!isFieldDoubleQuotted(typedField->getStringValue())) {
            typedField->setValue(addQuotationMarks(typedField->getValue()));
        }
    }
}

void CsvHandler::surroundFieldsInVectorWithQuotationMarks(
        std::vector<std::string>& vect) {
    for (int pos = 0; pos < (int) vect.size(); ++pos) {
        if (!isFieldDoubleQuotted(vect.at(pos))) {
            vect.at(pos) = addQuotationMarks(vect.at(pos));
        }
    }
}

inline bool CsvHandler::isColumnStringType(int columnId) {
    return _sourceFileColumnTypes[columnId] == CsvHandler::_tString ? true : false;
}

inline bool CsvHandler::isFieldDoubleQuotted(const std::string & field) {
    if (field.size()) {
        if (field.at(0) == _quotationMark) {
            return true;
        }
    }
    return false;
}

inline std::string CsvHandler::addQuotationMarks(const std::string & field) {
    std::string quottedField = field;
    quottedField += '"';
    quottedField = '"' + quottedField;
    return quottedField;
}

csv_genericField CsvHandler::getField(int columnIndex, int rowIndex) {
    if (rowIndex >= _absoluteBeginningIndex && rowIndex < _absoluteEndingIndex
            && columnIndex < (int) _sourceFileColumnTypes.size()) {
        long long row = _entriesInCurrentChunk - (_absoluteEndingIndex - rowIndex);
        return _sourceFileVector[columnIndex][row];
    } else if (_eofFlag && rowIndex >= _absoluteEndingIndex) {
        throw std::out_of_range("Row or column index is out of range!");
    }
    return nullptr;
}

csv_genericField CsvHandler::getField(std::string columnCaption, int rowIndex) {
    int colID;
    try {
        colID = getColumnId(columnCaption);
    } catch (InvalidColumnCaptionException& e) {
        std::string msg = "Column caption " + columnCaption + " is not valid.";
        throw InvalidColumnCaptionException(msg.c_str());
    }

    try {
        return getField(colID, rowIndex);
    } catch (std::out_of_range& exc) {
        throw exc;
    }
}

csv_entryLine CsvHandler::getRow(int rowIndex) {
    csv_entryLine entry;

    if (rowIndex >= _absoluteBeginningIndex && rowIndex < _absoluteEndingIndex) {
        long long row = _entriesInCurrentChunk - (_absoluteEndingIndex - rowIndex);
        entry.reserve(_sourceFileColumnTypes.size());
        for (int colID = 0; colID < (int) _sourceFileColumnTypes.size(); ++colID) {
            entry.emplace_back(_sourceFileVector[colID][row]->getStringValue());
        }
        return entry;
    } else if (_eofFlag && rowIndex >= _absoluteEndingIndex) {
        throw std::out_of_range("Row index out of range!");
    }
    return entry;
}

csv_column CsvHandler::getColumn(int columnIndex) {
    if (columnIndex < (int) _sourceFileColumnTypes.size()) {

        return _sourceFileVector[columnIndex];
    }
    throw std::out_of_range("Column index is out of range!");
}

std::vector<CsvEntryElement*> CsvHandler::getColumn(std::string columnCaption) {
    int colID;
    try {
        colID = getColumnId(columnCaption);
    } catch (InvalidColumnCaptionException& e) {

        std::string msg = "Column caption " + columnCaption + " is not valid.";
        throw InvalidColumnCaptionException(msg.c_str());
    }
    return _sourceFileVector[colID];
}

int CsvHandler::getColumnId(std::string columnCaption) {
    if (_sourceFileHeader.empty()) {
        throw HeaderNotAvailableException();
    }
    for (int headerID = 0; headerID < (int) _sourceFileHeader.size(); ++headerID) {
        if (_sourceFileHeader[headerID] == columnCaption) {
            return headerID;
        }
    }
    throw InvalidColumnCaptionException();
}

void CsvHandler::removeColumn(int columnIndex) {
    _sourceFileColumnTypes.erase(_sourceFileColumnTypes.begin() + columnIndex);
    if (!_sourceFileHeader.empty()) {
        _sourceFileHeader.erase(_sourceFileHeader.begin() + columnIndex);
    }
    for (auto it = _sourceFileVector[columnIndex].begin(); it < _sourceFileVector[columnIndex].end(); it++) {
        delete *it;
    }
    _sourceFileVector.erase(_sourceFileVector.begin() + columnIndex);
}

void CsvHandler::removeColumn(std::string columnCaption) {
    int colId;
    try {
        colId = getColumnId(columnCaption);
    } catch (InvalidColumnCaptionException& exc) {

        std::string msg = "Column caption " + columnCaption + " is not valid.";
        throw InvalidColumnCaptionException(msg.c_str());
    }

    removeColumn(colId);
}

void CsvHandler::removeRow(int rowIndex) {
    long long pos = rowIndex;
    if (rowIndex >= _absoluteBeginningIndex && rowIndex < _absoluteEndingIndex) {
        pos = _entriesInCurrentChunk - (_absoluteEndingIndex - rowIndex);
        for (int colID = 0; colID < (int) _sourceFileColumnTypes.size(); ++colID) {
            delete _sourceFileVector[colID].at(pos);
            _sourceFileVector[colID].erase(_sourceFileVector[colID].begin() + pos);
        }
        _entriesInCurrentChunk--;
        _absoluteEndingIndex--;
    } else if (_eofFlag && pos > _absoluteEndingIndex) {
        throw std::out_of_range("Row index out of range!");
    }
}

void CsvHandler::insertRow(csv_entryLine entry, int pos,
        _errorHandlingMode errorHandlingMode) {
    int newEntryPos = pos;

    if (pos == -1 && _eofFlag) {
        newEntryPos = _entriesInCurrentChunk;
        initializeNewEntry(newEntryPos);
        setColumnsForEntry(entry, newEntryPos, errorHandlingMode);
        ++_entriesInCurrentChunk;
        ++_absoluteEndingIndex;
    } else if (pos >= _absoluteBeginningIndex && pos < _absoluteEndingIndex) {
        newEntryPos = _entriesInCurrentChunk - _absoluteEndingIndex - pos;
        initializeNewEntry(newEntryPos);
        setColumnsForEntry(entry, newEntryPos, errorHandlingMode);
        ++_entriesInCurrentChunk;
        ++_absoluteEndingIndex;
    } else if (_eofFlag && pos > _absoluteEndingIndex) {
        throw std::out_of_range("Provided pos is greater than amount of entries");
    }
}

void CsvHandler::insertRow(std::string entry, int pos,
        _errorHandlingMode errorHandlingMode) {
    csv_entryLine newEntry;
    newEntry.reserve(_sourceFileColumnTypes.size());
    splitEntryByDelimiter(entry, newEntry, _csvDelimiter);
    insertRow(newEntry, pos, errorHandlingMode);
}

void CsvHandler::insertColumn(std::vector<CsvEntryElement*>& columnVector,
        _dataTypes type, int pos) {
    int newColPos = pos;
    if (pos == -1) {
        newColPos = _sourceFileColumnTypes.size();
    } else if (pos > (int) _sourceFileColumnTypes.size()) {
        throw std::out_of_range("Provided pos is greater than amount of columns");
    }
    _sourceFileVector.insert(_sourceFileVector.begin() + newColPos, columnVector);
    _sourceFileColumnTypes.insert(_sourceFileColumnTypes.begin() + newColPos,
            getDataTypeAsString(type));
}

void CsvHandler::insertColumn(std::vector<CsvEntryElement*>& columnVector,
        const std::string& caption, _dataTypes type, int pos) {
    if (_sourceFileHeader.empty()) {
        throw HeaderNotAvailableException();
    } else {
        int newColPos = pos;
        if (pos == -1) {
            newColPos = _sourceFileColumnTypes.size();
        }
        insertColumn(columnVector, type, pos);
        _sourceFileHeader.insert(_sourceFileHeader.begin() + newColPos, caption);
    }
}

void CsvHandler::insertColumn(_dataTypes type, int pos) {
    if (pos > (int) _sourceFileColumnTypes.size()) {
        throw std::out_of_range("Provided pos is greater than amount of columns");
    }
    int newColPos = pos;
    if (pos == -1) {
        newColPos = _sourceFileColumnTypes.size();
    }
    _sourceFileColumnTypes.insert(_sourceFileColumnTypes.begin() + newColPos,
            getDataTypeAsString(type));
    _sourceFileVector.insert(_sourceFileVector.begin() + newColPos,
            initializeNewColumn(type));
}

void CsvHandler::insertColumn(const std::string& caption,
        _dataTypes type, int pos) {
    if (_sourceFileHeader.empty()) {
        throw HeaderNotAvailableException();
    } else {
        int newColPos = pos;
        if (pos == -1) {
            newColPos = _sourceFileColumnTypes.size();
        }
        insertColumn(type, pos);

        if (_chunksCount == 1) {
            _sourceFileHeader.insert(_sourceFileHeader.begin() + newColPos, caption);
        }
    }
}

long long CsvHandler::replaceAll(int columnPos,
        std::string regex, std::string replacement) {
    if (columnPos < (int) _sourceFileColumnTypes.size()
            && _sourceFileColumnTypes.at(columnPos) == _tString) {
        std::regex r(regex);
        std::smatch matches;
        long long replaced = 0;
        for (CsvEntryElement* field : _sourceFileVector.at(columnPos)) {
            std::string s = field->getStringValue();
            std::regex_search(s, matches, r);
            for (std::string ms : matches) {
                if (!ms.empty()) {
                    csv_stringField* sf = dynamic_cast<csv_stringField*> (field);
                    sf->setValue(std::regex_replace(s, r, replacement));
                    ++replaced;
                }
            }
        }
        return replaced;
    } else {
        throw std::out_of_range("Provided pos is greater than amount of columns");
    }
}

long long CsvHandler::replaceAll(std::string columnCaption,
        std::string regex, std::string replacement) {
    return replaceAll(getColumnId(columnCaption), regex, replacement);
}

csv_column CsvHandler::findAll(int columnPos, std::string regex) {
    if (columnPos < (int) _sourceFileColumnTypes.size()) {
        csv_column fFields;
        std::regex r(regex);
        std::smatch matches;
        for (CsvEntryElement* field : _sourceFileVector.at(columnPos)) {
            std::string s = field->getStringValue();
            std::regex_search(s, matches, r);
            for (std::string ms : matches) {
                if (!ms.empty()) {
                    fFields.push_back(field);
                }
            }
        }
        return fFields;
    } else {
        throw std::out_of_range("Provided pos is greater than amount of columns");
    }
}

csv_column CsvHandler::findAll(std::string columnCaption, std::string regex) {
    return findAll(getColumnId(columnCaption), regex);
}

csv_entryLines CsvHandler::findAllRows(int columnPos, std::string regex) {
    if (columnPos < (int) _sourceFileColumnTypes.size()) {
        csv_entryLines rows;
        std::regex r(regex);
        std::smatch matches;
        long long rowIndex = 0;
        for (CsvEntryElement* field : _sourceFileVector.at(columnPos)) {
            std::string s = field->getStringValue();
            std::regex_search(s, matches, r);
            for (std::string ms : matches) {
                if (!ms.empty()) {
                    rows.push_back(getRow(rowIndex));
                }
            }
            ++rowIndex;
        }
        return rows;
    } else {
        throw std::out_of_range("Provided pos is greater than amount of columns");
    }
}

csv_entryLines CsvHandler::findAllRows(std::string columnCaption, std::string regex) {
    return findAllRows(getColumnId(columnCaption), regex);
}


