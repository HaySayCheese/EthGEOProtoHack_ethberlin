#include "RoutingTableHandler.h"

RoutingTableHandler::RoutingTableHandler(
    sqlite3 *db,
    string tableName,
    Logger *logger) :

    mDataBase(db),
    mTableName(tableName),
    mLog(logger),
    isTransactionBegin(false) {

    string query = "CREATE TABLE IF NOT EXISTS " + mTableName +
                     "(source BLOB NOT NULL, "
                     "destination BLOB NOT NULL, "
                     "direction BLOB);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating table: " + mTableName +
                              " : Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "table" << mTableName << "created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating table: " + mTableName +
                              " : Run query");
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_source_idx on " + mTableName + "(source);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Source: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for source created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating index for Source: "
                              "Run query");
    }

    query = "CREATE INDEX IF NOT EXISTS " + mTableName
            + "_destination_idx on " + mTableName + "(destination);";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "index for destination created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating index for Destination: "
                              "Run query");
    }

    query = "CREATE UNIQUE INDEX IF NOT EXISTS " + mTableName
            + "_source_destination_unique_idx ON " + mTableName
            + " (source, destination)";
    rc = sqlite3_prepare_v2(mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "unique index for source and destination created successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::creating unique index for Source and Destination: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

void RoutingTableHandler::insert(
        const NodeUUID &source,
        const NodeUUID &destination,
        const TrustLineDirection direction) {

    if (!isTransactionBegin) {
        prepareInsertred();
    }

    string query = "INSERT INTO " + mTableName +
                     "(source, destination, direction) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad query");
    }

    rc = sqlite3_bind_blob(stmt, 1, source.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Source");
    }
    rc = sqlite3_bind_blob(stmt, 2, destination.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Desitnation");
    }
    switch (direction) {
        case TrustLineDirection::Incoming:
            rc = sqlite3_bind_blob(stmt, 3, "I", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Outgoing:
            rc = sqlite3_bind_blob(stmt, 3, "O", 1, SQLITE_STATIC);
            break;
        case TrustLineDirection::Both:
            rc = sqlite3_bind_blob(stmt, 3, "B", 1, SQLITE_STATIC);
            break;
        default:
            throw ValueError("RoutingTableHandler::insert: "
                                     "Direction should be Incomeng, Outgoing or Both");
        }
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::insert: "
                              "Bad binding of Direction");
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "prepare inserting (" << source << ", " << destination << ") " << "is completed successfully";
#endif
    } else {
        throw IOError("RoutingTableHandler::insert: "
                              "Run query");
    }
}

void RoutingTableHandler::commit() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call commit, but trunsaction wasn't started";
#endif
        return;
    }

    string query = "END TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::commit: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction commit";
#endif
    } else {
        throw IOError("RoutingTableHandler::commit: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void RoutingTableHandler::rollBack() {

    if (!isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call rollBack, but trunsaction wasn't started";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "ROLLBACK;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::rollback: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "rollBack done";
#endif
    } else {
        throw IOError("RoutingTableHandler::rollback: "
                              "Run query");
    }

    sqlite3_reset(stmt);
    isTransactionBegin = false;
}

void RoutingTableHandler::prepareInsertred() {

    if (isTransactionBegin) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        error() << "call prepareInsertred, but previous transaction isn't finished";
#endif
        return;
    }

    sqlite3_finalize(stmt);
    string query = "BEGIN TRANSACTION;";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::prepareInsertred: "
                              "Bad query");
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
#ifdef STORAGE_HANDLER_DEBUG_LOG
        info() << "transaction begin";
#endif
    } else {
        throw IOError("RoutingTableHandler::prepareInsertred: "
                              "Run query");
    }
    isTransactionBegin = true;

}

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> RoutingTableHandler::routeRecordsWithDirections() {

    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> result;
    string query = "SELECT source, destination, direction FROM " + mTableName;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirections: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
        char* direction = (char*)sqlite3_column_blob(stmt, 2);
        if (strcmp(direction, "I") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Incoming));
        } else if (strcmp(direction, "O") == 0) {
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Outgoing));
        } else if (strcmp(direction, "B") == 0){
            result.push_back(
                make_tuple(
                    source,
                    destination,
                    TrustLineDirection::Both));
        } else {
#ifdef STORAGE_HANDLER_DEBUG_LOG
            error() << "wrong direction during reading from DB";
#endif
            throw ValueError("RoutingTableHandler::routeRecordsWithDirections: "
                                     "Wrong Direction during reading from DB");
        }
    }
    sqlite3_reset(stmt);
    return result;
}

vector<pair<NodeUUID, NodeUUID>> RoutingTableHandler::routeRecords() {

    vector<pair<NodeUUID, NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecords: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
        result.push_back(
            make_pair(
                source,
                destination));
    }
    sqlite3_reset(stmt);
    return result;
}

unordered_map<NodeUUID, vector<NodeUUID>> RoutingTableHandler::routeRecordsMapDestinationKey() {

    DateTime startTime = utc_now();
    unordered_map<NodeUUID, vector<NodeUUID>> result;
    string query = "SELECT source, destination FROM " + mTableName;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsMapDestinationKey: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
        auto element = result.find(destination);
        if (element == result.end()) {
            vector<NodeUUID> newVect;
            newVect.push_back(source);
            result.insert(
                make_pair(
                    destination,
                    newVect));
        } else {
            element->second.push_back(source);
        }

    }
    sqlite3_reset(stmt);
    Duration methodTime = utc_now() - startTime;
    info() << "RoutingTableHandler::routeRecordsMapDestinationKey finished with time: " << methodTime;
    return result;
}

vector<NodeUUID> RoutingTableHandler::allDestinationsForSource(
    const NodeUUID &sourceUUID) {

    vector<NodeUUID> result;
    string query = "SELECT destination FROM " + mTableName + " WHERE source = ?";
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                              "Bad query");
    }
    rc = sqlite3_bind_blob(stmt, 1, sourceUUID.data, NodeUUID::kBytesSize, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::allDestinationsForSource: "
                              "Bad Source binding");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW ) {
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        result.push_back(destination);
    }
    sqlite3_reset(stmt);
    return result;
}

unordered_map<NodeUUID, vector<pair<NodeUUID, TrustLineDirection>>> RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey() {

    unordered_map<NodeUUID, vector<pair<NodeUUID, TrustLineDirection>>> result;
    string query = "SELECT source, destination, direction FROM " + mTableName;
#ifdef STORAGE_HANDLER_DEBUG_LOG
    info() << "select: " << query;
#endif
    int rc = sqlite3_prepare_v2( mDataBase, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        throw IOError("RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey: "
                              "Bad query");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NodeUUID source;
        memcpy(
            source.data,
            sqlite3_column_blob(stmt, 0),
            NodeUUID::kBytesSize);
        NodeUUID destination;
        memcpy(
            destination.data,
            sqlite3_column_blob(stmt, 1),
            NodeUUID::kBytesSize);
        char* directionChr = (char*)sqlite3_column_blob(stmt, 2);
        TrustLineDirection direction;
        if (strcmp(directionChr, "I") == 0) {
            direction = TrustLineDirection::Incoming;
        } else if (strcmp(directionChr, "O") == 0) {
            direction = TrustLineDirection::Outgoing;
        } else if (strcmp(directionChr, "B") == 0) {
            direction = TrustLineDirection::Both;
        } else {
#ifdef STORAGE_HANDLER_DEBUG_LOG
            error() << "wrong direction during reading from DB";
#endif
            throw ValueError("RoutingTableHandler::routeRecordsWithDirectionsMapSourceKey: "
                                     "Wrong Direction during reading from DB");
        }
        auto mapElement = result.find(source);
        if (mapElement == result.end()) {
            vector<pair<NodeUUID, TrustLineDirection>> newValue;
            newValue.push_back(
                make_pair(
                    destination,
                    direction));
            result.insert(
                make_pair(
                    source,
                    newValue));
        } else {
            mapElement->second.push_back(
                make_pair(
                    destination,
                    direction));
        }
    }
    return result;
}

const string& RoutingTableHandler::tableName() const {

    return mTableName;
}

LoggerStream RoutingTableHandler::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream RoutingTableHandler::error() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string RoutingTableHandler::logHeader() const {

    stringstream s;
    s << "[RoutingTableHandler]";
    return s.str();
}