LIBRARY()

SRCS(
    blob.cpp
    blob_cache.cpp
    blob_manager.cpp
    blob_manager_db.cpp
    blob_manager_txs.cpp
    columnshard__export.cpp
    columnshard__forget.cpp
    columnshard__init.cpp
    columnshard__notify_tx_completion.cpp
    columnshard__plan_step.cpp
    columnshard__progress_tx.cpp
    columnshard__propose_cancel.cpp
    columnshard__propose_transaction.cpp
    columnshard__read.cpp
    columnshard__read_base.cpp
    columnshard__read_blob_ranges.cpp
    columnshard__scan.cpp
    columnshard__index_scan.cpp
    columnshard__stats_scan.cpp
    columnshard__write.cpp
    columnshard__write_index.cpp
    columnshard.cpp
    columnshard_impl.cpp
    columnshard_common.cpp
    columnshard_private_events.cpp
    counters.cpp
    compaction_actor.cpp
    defs.cpp
    eviction_actor.cpp
    export_actor.cpp
    indexing_actor.cpp
    read_actor.cpp
    write_actor.cpp
    tables_manager.cpp
)

GENERATE_ENUM_SERIALIZATION(columnshard.h)

PEERDIR(
    library/cpp/actors/core
    ydb/core/actorlib_impl
    ydb/core/base
    ydb/core/blobstorage/dsproxy
    ydb/core/control
    ydb/core/formats
    ydb/core/kqp
    ydb/core/protos
    ydb/core/tablet
    ydb/core/tablet_flat
    ydb/core/tx/columnshard/engines
    ydb/core/tx/columnshard/counters
    ydb/core/tx/tiering
    ydb/core/tx/conveyor/usage
    ydb/core/tx/long_tx_service/public
    ydb/core/util
    ydb/public/api/protos
    ydb/library/yql/dq/actors/compute
    ydb/library/chunks_limiter
)

YQL_LAST_ABI_VERSION()

END()

RECURSE(
    engines
)

RECURSE_FOR_TESTS(
    ut_rw
    ut_schema
)
