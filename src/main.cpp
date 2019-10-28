/*
 * Example main file for CsvHandler library.
 *
 * File:   main.cpp
 * Author: dawidtoczek
 */
#include "CsvHandler.hpp"

using namespace std;
using namespace csvh;
int main() {

    // EXAMPLE 0a: Validate CSV file structure
    {
        CsvHandler csvHandle("data/input/names_with_birthdate.csv", load_whole_file, CSV, ',', include_header);
        const int numberOfColumns = 4;

        // correct data types
        {
            _dataTypes colTypes[numberOfColumns] = {type_string, type_int, type_int, type_date};
            UserDefinedTypesValidationResult validationResult = csvHandle.validateTypesForColumns(numberOfColumns, colTypes);

            cout << "EXAMPLE 0a: Validate CSV file structure - correct" << endl;
            if (!validationResult.isValid()) {
                cout << validationResult.getMessage() << endl;
            } else {
                cout << "File structure seems to be correct." << endl;
            }
            cout << endl;
        }

        // incorrect data types
        {
            _dataTypes colTypes[numberOfColumns] = {type_double, type_int, type_int, type_date};
            UserDefinedTypesValidationResult validationResult = csvHandle.validateTypesForColumns(numberOfColumns, colTypes);

            cout << "EXAMPLE 0a: Validate CSV file structure - incorrect" << endl;
            if (!validationResult.isValid()) {
                cout << validationResult.getMessage() << endl;
            } else {
                cout << "File structure seems to be correct." << endl;
            }
            cout << endl;
        }
    }

    // EXAMPLE 0b: Provide user defined column types
    {
        CsvHandler csvHandle("data/input/names_with_birthdate.csv", load_whole_file, CSV, ',', include_header);
        const int numberOfColumns = 4;
        _dataTypes colTypes[numberOfColumns] = {type_string, type_int, type_int, type_date};

        csvHandle.provideTypesForColumns(numberOfColumns, colTypes);

        if (csvHandle.loadEntries()) {
            csvHandle.storeDataInFile("data/output/names_with_birthdate.csv");
        }
    }

    // EXAMPLE 1: Change currency from PLN to EUR.
    {
        CsvHandler csvHandle("data/input/products.csv", load_whole_file, CSV, ',', include_header);

        if (csvHandle.loadEntries()) {
            const double PLN_TO_EUR_RATIO = 4.23 ;

            csv_column priceCol = csvHandle.getColumn("Price");

            for (csv_genericField price : priceCol) {
                csv_doubleField* priceTyped =
                    dynamic_cast<csv_doubleField*>(price);
                priceTyped->setValue(priceTyped->getValue() / PLN_TO_EUR_RATIO);
            }

            csvHandle.replaceAll("Unit" , "^PLN$", "EUR");
            csvHandle.storeDataInFile("data/output/products.csv");
        }
    }

    // EXAMPLE 2: Mask last name with 'xxx'
    {
        CsvHandler csvHandle("data/input/names.csv", load_whole_file, CSV, ',', include_header);

        if (csvHandle.loadEntries()) {
            csvHandle.replaceAll("Full name", "^(\\S+) .*", "$1 xxx");
            csvHandle.storeDataInFile("data/output/names.csv");
        }
    }

    // EXAMPLE 3: Find all people >= 40 years old
    {
        CsvHandler csvHandle("data/input/names.csv", load_whole_file, CSV, ',', include_header);
        int phoneNoID;
        int fullNameID;

        if (csvHandle.loadEntries()) {
            try {
                phoneNoID = csvHandle.getColumnId("Phone number");
                fullNameID = csvHandle.getColumnId("Full name");
            } catch (const InvalidColumnCaptionException& exc) {
                cerr << exc.what();
                //DO SOMETHING ...
            } catch (const HeaderNotAvailableException& exc) {
                cerr << exc.what();
                //DO SOMETHING ...
            }

            csv_entryLines olderThan40 = csvHandle.findAllRows(
                "Age", "^[4-9][0-9]+|^[1-3][0-9]{2,}");

            for (csv_entryLine person : olderThan40) {
                cout << person.at(fullNameID) << '\t'
                     << person.at(phoneNoID) << endl;
            }
        }
    }

    // EXAMPLE 4: Convert CSV file to JSON
    {
        CsvHandler csvHandle("data/input/names.csv", load_whole_file, CSV, ',', include_header);

        if (csvHandle.loadEntries()) {
            csvHandle.storeDataInFile("data/output/names.json", JSON);
        }
    }

    // EXAMPLE 5: Add index column and change delimiter to '|'
    {
        CsvHandler csvHandle("data/input/building_consents.csv", load_whole_file, CSV, ',', include_header);

        if (csvHandle.loadEntries()) {
            csv_column indexColumn;
            indexColumn.reserve(csvHandle.getAmountOfEntries());

            for (int index = 0; index < csvHandle.getAmountOfEntries(); ++index) {
                csv_intField* newField = new csv_intField();
                newField->setValue(index);
                indexColumn.emplace_back(newField);
            }

            csvHandle.insertColumn(indexColumn, "Index", type_int, 0);
            csvHandle.storeDataInFile("data/output/building_consents.csv", CSV, '|');
        }
    }

    // EXAMPLE 6: Add quotation marks to string fields
    {
        CsvHandler csvHandle("data/input/building_consents.csv", load_whole_file, CSV, ',', include_header);
        csvHandle.provideTypesForColumns(6, type_string, type_string, type_int, type_string,type_string,type_string);

        if (csvHandle.loadEntries()) {
            csvHandle.surroundStringFieldsWithQuotationMarks();
            csvHandle.storeDataInFile("data/output/building_consents_strings_quotted.csv", CSV, ',');
        }
    }

    return 0;
}
