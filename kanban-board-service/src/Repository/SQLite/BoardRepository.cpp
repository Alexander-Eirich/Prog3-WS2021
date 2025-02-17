#include "BoardRepository.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string.h>

using namespace Prog3::Repository::SQLite;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace std;

#ifdef RELEASE_SERVICE
string const BoardRepository::databaseFile = "./data/kanban-board.db";
#else
string const BoardRepository::databaseFile = "../data/kanban-board.db";
#endif

BoardRepository::BoardRepository() : database(nullptr) {

    string databaseDirectory = filesystem::path(databaseFile).parent_path().string();

    if (filesystem::is_directory(databaseDirectory) == false) {
        filesystem::create_directory(databaseDirectory);
    }

    int result = sqlite3_open(databaseFile.c_str(), &database);

    if (SQLITE_OK != result) {
        cout << "Cannot open database: " << sqlite3_errmsg(database) << endl;
    }

    initialize();
}

BoardRepository::~BoardRepository() {
    sqlite3_close(database);
}

void BoardRepository::initialize() {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlCreateTableColumn =
        "create table if not exists column("
        "id integer not null primary key autoincrement,"
        "name text not null,"
        "position integer not null UNIQUE);";

    string sqlCreateTableItem =
        "create table if not exists item("
        "id integer not null primary key autoincrement,"
        "title text not null,"
        "date text not null,"
        "position integer not null,"
        "column_id integer not null,"
        "unique (position, column_id),"
        "foreign key (column_id) references column (id));";

    result = sqlite3_exec(database, sqlCreateTableColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    result = sqlite3_exec(database, sqlCreateTableItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // only if dummy data is needed ;)
    //createDummyData();
}

Board BoardRepository::getBoard() {
    Board board(boardTitle);
    vector<Column> vColumn = getColumns();
    board.setColumns(vColumn);
    return board;
}

std::vector<Column> BoardRepository::getColumns() {
    vector<Column> cVector;
    string sqlGetColumns = "SELECT * FROM Column ORDER BY position;";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlGetColumns.c_str(), queryCallback2, &cVector, &errorMessage);
    handleSQLError(result, errorMessage);

    if (SQLITE_OK == result && !cVector.empty()) {
        for (Column &column : cVector) {
            vector<Item> vecItem = getItems(column.getId());
            for (Item &item : vecItem) {
                column.addItem(item);
            }
        }
        return cVector;
    }
    return vector<Column>{};
}

std::optional<Column> BoardRepository::getColumn(int id) {
    vector<Column> cVector;
    string sqlGetColumn = "SELECT * FROM Column WHERE Id =" + to_string(id) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlGetColumn.c_str(), queryCallback2, &cVector, &errorMessage);
    handleSQLError(result, errorMessage);

    if (SQLITE_OK == result && !cVector.empty()) {
        Column c = cVector.back();
        vector<Item> vector = getItems(id);

        for (Item &item : vector) {
            c.addItem(item);
        }
        return c;
    }
    return nullopt;
}

std::optional<Column> BoardRepository::postColumn(std::string name, int position) {
    string sqlpostColumn = "INSERT INTO Column ('name', 'position') VALUES ('" + name + "', '" + to_string(position) + "');";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlpostColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    if (SQLITE_OK == result) {
        //The sqlite3_last_insert_rowid(D) interface usually returns the rowid of the most recent successful INSERT into a rowid table or virtual table on database connection D
        int columnId = sqlite3_last_insert_rowid(database);
        return Column(columnId, name, position);
    }
    return std::nullopt;
}

std::optional<Prog3::Core::Model::Column> BoardRepository::putColumn(int id, std::string name, int position) {

    string sqlPutColumn = "UPDATE Column SET name = '" + name + "', position = '" + to_string(position) + "' WHERE id = " + to_string(id) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlPutColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    int numberOfRowsModified = sqlite3_changes(database);

    if (SQLITE_OK == result && numberOfRowsModified != 0) {
        Column c(id, name, position);
        vector<Item> vector = getItems(id);

        for (Item item : vector) {
            c.addItem(item);
        }
        return c;
    }
    return std::nullopt;
}

void BoardRepository::deleteColumn(int id) {
    string sqlDeleteColumn = "DELETE FROM Column WHERE id=" + to_string(id) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlDeleteColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

std::vector<Item> BoardRepository::getItems(int columnId) {
    vector<Item> pVector;
    string sqlGetItems = "SELECT * FROM Item WHERE column_id = " + to_string(columnId) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlGetItems.c_str(), queryCallback, &pVector, &errorMessage);
    handleSQLError(result, errorMessage);
    return pVector;
}

std::optional<Item> BoardRepository::getItem(int columnId, int itemId) {

    vector<Item> pVector;
    string sqlGetItem = "SELECT * FROM Item WHERE id = " + to_string(itemId) + " AND column_id = " + to_string(columnId) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlGetItem.c_str(), queryCallback, &pVector, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result && !pVector.empty()) {
        Item item = pVector.back();
        return item;
    }
    return nullopt;
}

std::optional<Item> BoardRepository::postItem(int columnId, std::string title, int position) {
    //Für Timestamp
    time_t timestamp = time(nullptr);
    char *s = ctime(&timestamp);

    string sqlpostItem = "INSERT INTO Item ('column_Id', 'title', 'position', 'date') VALUES ('" + to_string(columnId) + "', '" + title + "', '" + to_string(position) + "', '" + s + "');";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlpostItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) {
        return Item(sqlite3_last_insert_rowid(database), title, position, s);
    }
    return std::nullopt;
}

std::optional<Prog3::Core::Model::Item> BoardRepository::putItem(int columnId, int itemId, std::string title, int position) {
    time_t timestamp = time(nullptr);
    char *s = ctime(&timestamp);

    string sqlPutItem = "UPDATE Item SET title = '" + title + "', position = '" + to_string(position) + "', date = '" + s + "' WHERE column_id = " + to_string(columnId) + " AND id = " + to_string(itemId) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlPutItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    int numberOfModifiedRows = sqlite3_changes(database);
    if (SQLITE_OK == result && numberOfModifiedRows != 0) {
        return Item(itemId, title, position, s);
    }
    return std::nullopt;
}

void BoardRepository::deleteItem(int columnId, int itemId) {
    //columnId kann man sich sparen, weil itemId eindeutig ist
    string sqlDeleteItem = "DELETE FROM Item WHERE Id=" + to_string(itemId) + ";";
    int result = 0;
    char *errorMessage = nullptr;
    result = sqlite3_exec(database, sqlDeleteItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

void BoardRepository::handleSQLError(int statementResult, char *errorMessage) {

    if (statementResult != SQLITE_OK) {
        cout << "SQL error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

void BoardRepository::createDummyData() {

    cout << "creatingDummyData ..." << endl;

    int result = 0;
    char *errorMessage;
    string sqlInsertDummyColumns =
        "insert into column (name, position)"
        "VALUES"
        "(\"prepare\", 1),"
        "(\"running\", 2),"
        "(\"finished\", 3);";

    result = sqlite3_exec(database, sqlInsertDummyColumns.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    string sqlInserDummyItems =
        "insert into item (title, date, position, column_id)"
        "VALUES"
        "(\"in plan\", date('now'), 1, 1),"
        "(\"some running task\", date('now'), 1, 2),"
        "(\"finished task 1\", date('now'), 1, 3),"
        "(\"finished task 2\", date('now'), 2, 3);";

    result = sqlite3_exec(database, sqlInserDummyItems.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

/*
  I know source code comments are bad, but this one is to guide you through the use of sqlite3_exec() in case you want to use it.
  sqlite3_exec takes a "Callback function" as one of its arguments, and since there are many crazy approaches in the wild internet,
  I want to show you how the signature of this "callback function" may look like in order to work with sqlite3_exec()
*/
//Für jede Row aufgerufen

int BoardRepository::queryCallback(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    //cast void pointer into  vector<Item> type pointer
    vector<Item> *pV = static_cast<vector<Item> *>(data);

    string title = "";
    int position = -1;
    string date = "";
    int itemId = -1;
    int count = 0;

    for (int i = 0; i < numberOfColumns; i++) {
        //strcmp überprüft ganzen string und nicht nur den ersten char
        if (fieldValues[i] == NULL)
            break;
        if (!strcmp(columnNames[i], "id")) {
            itemId = stoi(fieldValues[i]);
            count++;
        }
        if (!strcmp(columnNames[i], "title")) {
            title.assign(fieldValues[i]);
            count++;
        }
        if (!strcmp(columnNames[i], "position")) {
            position = stoi(fieldValues[i]);
            count++;
        }
        if (!strcmp(columnNames[i], "date")) {
            date.assign(fieldValues[i]);
            count++;
        }
    }
    if (count == 4) {
        Item i(itemId, title, position, date);
        pV->push_back(i);
    } else {
        cerr << "Select-query passt nicht mit qeryCallback Argumenten überein" << endl;
    }
    return 0;
}
int BoardRepository::queryCallback2(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    vector<Column> *pC = static_cast<vector<Column> *>(data);
    int id = -1;
    string name = "";
    int positon = -1;
    int count = 0;
    for (int i = 0; i < numberOfColumns; i++) {
        // Falls fieldValues == Null, --> überspringen, sonst aufgrund dereferenzierung Programmabbruch
        if (fieldValues[i] == NULL)
            break;
        if (!strcmp(columnNames[i], "id")) {
            id = stoi(fieldValues[i]);
            count++;
        }
        if (!strcmp(columnNames[i], "name")) {
            name.assign(fieldValues[i]);
            count++;
        }
        if (!strcmp(columnNames[i], "position")) {
            positon = stoi(fieldValues[i]);
            count++;
        }
    }
    if (count == 3) {
        Column c(id, name, positon);
        pC->push_back(c);
    } else {
        cerr << "Select-query passt nicht mit qeryCallback Argumenten überein" << endl;
    }
    return 0;
}
