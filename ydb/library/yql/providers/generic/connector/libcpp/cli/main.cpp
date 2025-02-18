#include <ydb/library/yql/utils/log/log.h>
#include <ydb/library/yql/providers/generic/connector/libcpp/client.h>
#include <ydb/library/yql/providers/generic/connector/libcpp/error.h>

const TString TableName = "example_3";

void SetDatabaseSourceInstance(NYql::Connector::API::DataSourceInstance* dsi) {
    dsi->set_database("dqrun");
    dsi->set_endpoint("localhost:9000");
    dsi->mutable_credentials()->mutable_basic()->set_username("crab");
    dsi->mutable_credentials()->mutable_basic()->set_password("qwerty12345");
}

std::shared_ptr<NYql::Connector::DescribeTableResult> describeTable(NYql::Connector::IClient::TPtr client) {
    NYql::Connector::API::DescribeTableRequest request;
    request.set_table(TableName);
    SetDatabaseSourceInstance(request.mutable_data_source_instance());

    auto result = client->DescribeTable(request);

    if (NYql::Connector::ErrorIsSuccess(result->Error)) {
        YQL_LOG(INFO) << "DescribeTable: succeded: ";
        YQL_LOG(INFO) << "\n"
                      << result->Schema.ShortDebugString();
    } else {
        ythrow yexception() << "DescribeTable: failure: " << result->Error.ShortDebugString();
    }

    return result;
}

std::shared_ptr<NYql::Connector::ListSplitsResult>
listSplits(NYql::Connector::IClient::TPtr client, const google::protobuf::RepeatedPtrField<Ydb::Column>& columns) {
    NYql::Connector::API::ListSplitsRequest request;
    SetDatabaseSourceInstance(request.mutable_data_source_instance());

    auto select = request.add_selects();

    // SELECT *
    std::for_each(columns.begin(), columns.end(), [&](const auto& c) {
        auto item = select->mutable_what()->mutable_items()->Add();
        item->mutable_column()->CopyFrom(c);
    });

    // SELECT * FROM $tableName
    select->mutable_from()->set_table(TableName);

    auto result = client->ListSplits(request);

    if (NYql::Connector::ErrorIsSuccess(result->Error)) {
        YQL_LOG(INFO) << "ListSplits: succeded: splits=" << result->Splits.size();
        std::for_each(result->Splits.cbegin(), result->Splits.cend(), [](const auto& split) {
            YQL_LOG(INFO) << "\n"
                          << split.ShortDebugString();
        });
    } else {
        ythrow yexception() << "ListSplits: failure: " << result->Error.ShortDebugString();
    }

    return result;
}

std::shared_ptr<NYql::Connector::ReadSplitsResult> readSplits(NYql::Connector::IClient::TPtr client,
                                                              const std::vector<NYql::Connector::API::Split>& splits) {
    NYql::Connector::API::ReadSplitsRequest request;
    SetDatabaseSourceInstance(request.mutable_data_source_instance());
    request.set_format(NYql::Connector::API::ReadSplitsRequest_Format::ReadSplitsRequest_Format_ARROW_IPC_STREAMING);

    std::for_each(splits.begin(), splits.end(), [&](const auto& c) { request.mutable_splits()->Add()->CopyFrom(c); });

    auto result = client->ReadSplits(request);
    // client->ReadSplits(splits, );

    if (NYql::Connector::ErrorIsSuccess(result->Error)) {
        YQL_LOG(INFO) << "ReadSplits: succeded: record_batches=" << result->RecordBatches.size();

        for (const auto& recordBatch : result->RecordBatches) {
            YQL_LOG(INFO) << "\n"
                          << recordBatch->ToString();
        }
    } else {
        ythrow yexception() << "ReadSplits: failure: " << result->Error.ShortDebugString();
    }

    return result;
}

int main() {
    NYql::NLog::InitLogger("console", false);
    auto client = NYql::Connector::MakeClientGRPC("localhost:50051");

    try {
        auto columns = describeTable(client)->Schema.columns();
        auto splits = listSplits(client, columns)->Splits;
        readSplits(client, splits);
    } catch (const std::runtime_error& e) {
        YQL_LOG(ERROR) << e.what();
    }

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
