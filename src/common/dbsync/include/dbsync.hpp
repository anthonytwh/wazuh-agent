#pragma once

#include <idbsync.hpp>

#include "builder.hpp"
#include "commonDefs.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>

using ResultCallbackData = const std::function<void(ReturnTypeCallback, const nlohmann::json&)>;

class DBSync : public IDBSync
{
public:
    /// @brief Explicit DBSync Constructor.
    /// @param hostType          Dynamic library host type to be used.
    /// @param dbType            Database type to be used (currently only supported SQLITE3)
    /// @param path              Path where the local database will be created.
    /// @param sqlStatement      SQL sentence to create tables in a SQL engine.
    /// @param dbManagement      Database management type to be used at startup.
    /// @param upgradeStatements SQL sentences to be executed when upgrading the database.
    explicit DBSync(const HostType hostType,
                    const DbEngineType dbType,
                    const std::string& path,
                    const std::string& sqlStatement,
                    const DbManagement dbManagement = DbManagement::VOLATILE,
                    const std::vector<std::string>& upgradeStatements = {});

    /// @brief DBSync Constructor.
    /// @param handle     handle to point another dbsync instance.
    DBSync(const DBSYNC_HANDLE handle);

    /// @brief DBSync Destructor.
    ~DBSync();

    /// @brief Generates triggers that execute actions to maintain consistency between tables.
    /// @param jsInput      JSON information with tables relationship.
    void addTableRelationship(const nlohmann::json& jsInput) override;

    /// @brief Insert the \p jsInsert data in the database.
    /// @param jsInsert JSON information with values to be inserted.
    void insertData(const nlohmann::json& jsInsert) override;

    /// @brief Sets the max rows in the \p table table.
    /// @param table    Table name to apply the max rows configuration.
    /// @param maxRows  Max rows number to be applied in the table \p table table.
    /// @details The table will work as a queue if the limit is exceeded.
    void setTableMaxRow(const std::string& table, const long long maxRows) override;

    /// @brief Inserts (or modifies) a database record.
    /// @param jsInput        JSON information used to add/modified a database record.
    /// @param callbackData   Result callback(std::function) will be called for each result.
    void syncRow(const nlohmann::json& jsInput, ResultCallbackData& callbackData) override;

    /// @brief Select data, based in \p jsInput data, from the database table.
    /// @param jsInput         JSON with table name, fields and filters to apply in the query.
    /// @param callbackData    Result callback(std::function) will be called for each result.
    void selectRows(const nlohmann::json& jsInput, ResultCallbackData& callbackData) override;

    /// @brief Deletes a database table record and its relationships based on \p jsInput value.
    /// @param jsInput JSON information to be applied/deleted in the database.
    void deleteRows(const nlohmann::json& jsInput) override;

    /// @brief Updates data table with \p jsInput information. \p jsResult value will
    ///  hold/contain the results of this operation (rows insertion, modification and/or deletion).
    /// @param jsInput    JSON information with snapshot values.
    /// @param jsResult   JSON with deletes, creations and modifications (diffs) in rows.
    void updateWithSnapshot(const nlohmann::json& jsInput, nlohmann::json& jsResult) override;

    /// @brief Update data table, based on json_raw_snapshot bulk data based on json string.
    /// @param jsInput       JSON with snapshot values.
    /// @param callbackData  Result callback(std::function) will be called for each result.
    void updateWithSnapshot(const nlohmann::json& jsInput, ResultCallbackData& callbackData) override;

    /// @brief Get current dbsync handle in the instance.
    /// @return DBSYNC_HANDLE to be used in all internal calls.
    DBSYNC_HANDLE handle() override
    {
        return m_dbsyncHandle;
    }

    /// @brief Turns off the services provided by the shared library.
    static void teardown();

private:
    DBSYNC_HANDLE m_dbsyncHandle;
    bool m_shouldBeRemoved;
};

class DBSyncTxn
{
public:
    /// @brief DBSync Transaction constructor
    /// @param handle         Handle obtained from the \ref DBSync instance.
    /// @param tables         Tables to be created in the transaction.
    /// @param threadNumber   Number of worker threads for processing data. If 0 hardware concurrency
    ///                       value will be used.
    /// @param maxQueueSize   Max data number to hold/queue to be processed.
    /// @param callbackData   Result callback(std::function) will be called for each result.
    /// @details If the max queue size is reached then this will be processed synchronously.
    explicit DBSyncTxn(const DBSYNC_HANDLE handle,
                       const nlohmann::json& tables,
                       const unsigned int threadNumber,
                       const unsigned int maxQueueSize,
                       ResultCallbackData& callbackData);

    /// @brief DBSync transaction Constructor.
    /// @param handle     handle to point another dbsync transaction instance.
    DBSyncTxn(const TXN_HANDLE handle);

    /// @brief Destructor closes the database transaction.
    virtual ~DBSyncTxn();

    /// @brief Synchronizes the \p jsInput data.
    /// @param jsInput JSON information to be synchronized.
    virtual void syncTxnRow(const nlohmann::json& jsInput);

    /// @brief Gets the deleted rows (diff) from the database.
    /// @param callbackData    Result callback(std::function) will be called for each result.
    virtual void getDeletedRows(ResultCallbackData& callbackData);

    /// @brief Get current dbsync transaction handle in the instance.
    /// @return TXN_HANDLE to be used in all internal calls.
    TXN_HANDLE handle()
    {
        return m_txn;
    }

private:
    TXN_HANDLE m_txn;
    bool m_shouldBeRemoved;
};

template<typename T>
class Query : public Builder<T>
{
protected:
    nlohmann::json m_jsQuery;

public:
    /// @brief Default constructor.
    Query() = default;

    /// @brief Default destructor.
    virtual ~Query() = default;

    /// @brief Returns the query object.
    /// @return Query object.
    nlohmann::json& query()
    {
        return m_jsQuery;
    }

    /// @brief Set table name.
    /// @param table Table name to be queried.
    T& table(const std::string& table)
    {
        m_jsQuery["table"] = table;
        return static_cast<T&>(*this); // Return reference to self
    }
};

class SelectQuery final : public Query<SelectQuery>
{
public:
    /// @brief Default constructor.
    SelectQuery() = default;

    /// @brief Default destructor.
    virtual ~SelectQuery() = default;

    /// @brief Set fields to be queried.
    /// @param fields Fields to be queried.
    SelectQuery& columnList(const std::vector<std::string>& fields);

    /// @brief Set filter to be applied in the query.
    /// @param filter Filter to be applied in the query.
    SelectQuery& rowFilter(const std::string& filter);

    /// @brief Set distinct flag to be applied in the query.
    /// @param distinct Distinct flag.
    SelectQuery& distinctOpt(const bool distinct);

    /// @brief Set order by field to be applied in the query.
    /// @param orderBy Order by field.
    SelectQuery& orderByOpt(const std::string& orderBy);

    /// @brief Set count/limit to be applied in the query.
    /// @param count Count/limit flag.
    SelectQuery& countOpt(const uint32_t count);
};

class DeleteQuery final : public Query<DeleteQuery>
{
public:
    /// @brief Default constructor.
    DeleteQuery() = default;

    /// @brief Default destructor.
    virtual ~DeleteQuery() = default;

    /// @brief Set data to be deleted.
    /// @param data Data to be deleted.
    DeleteQuery& data(const nlohmann::json& data);

    /// @brief Set filter to be applied in the query.
    /// @param filter Filter to be applied in the query.
    DeleteQuery& rowFilter(const std::string& filter);

    /// @brief Reset all data to be deleted.
    DeleteQuery& reset();
};

class InsertQuery final : public Query<InsertQuery>
{
public:
    /// @brief Default constructor.
    InsertQuery() = default;

    /// @brief Default destructor.
    virtual ~InsertQuery() = default;

    /// @brief Set data to be inserted.
    /// @param data Data to be inserted.
    InsertQuery& data(const nlohmann::json& data);

    /// @brief Reset all data to be inserted.
    InsertQuery& reset();
};

class SyncRowQuery final : public Query<SyncRowQuery>
{
public:
    /// @brief Default constructor.
    SyncRowQuery() = default;

    /// @brief Default destructor.
    virtual ~SyncRowQuery() = default;

    /// @brief Set data to be updated.
    /// @param data Data to be updated.
    SyncRowQuery& data(const nlohmann::json& data);

    /// @brief Set column to be ignored when comparing row values.
    /// @param column Name of the column to be ignored.
    SyncRowQuery& ignoreColumn(const std::string& column);

    /// @brief Make this query return the old data as well.
    SyncRowQuery& returnOldData();

    /// @brief Reset all data to be inserted.
    SyncRowQuery& reset();
};
