LIBRARY()

SRCS(
    grpc_service.cpp
)

PEERDIR(
    library/cpp/grpc/server
    ydb/core/grpc_services
    ydb/core/protos
    ydb/public/api/grpc
    ydb/public/lib/operation_id
)

END()
