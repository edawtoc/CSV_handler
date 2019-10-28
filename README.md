# CSV_handler
## C++ Library for CSV files handling.

The main purpose of this project was to write an easy to use and powerful library for CSVfiles handling.
The project contains src/main.cpp file with usage examples.

### FEATURES:
* It allows to process extra large files (the limit is the selected buffer size)
* Searches using regexp
* Add/remove columns and entries
* It can determine besic data types for columns such as integer, double, string and date.
* The column types can be specified manually for better fit to users needs.
* Columns separator is adjustable (by default it is comma).
* The library allows to perform bidirectional convertion between JSON and CSV.
      Supported JSON format:
```
      [
          {
              "name1" : value1,
              "name2" : value2,
              (...)
              "nameN" : valueN
          },
          (...)
          {
              "name1" : value1,
              "name2" : value2,
              (...)
              "nameN" : valueN
          }
     ]
```

### REQUIREMENTS:
Compiler that supports C++11 standard.
