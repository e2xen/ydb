#pragma once
#include "test_server.h"
#include <ydb/library/persqueue/topic_parser_public/topic_parser.h>
#include <library/cpp/logger/log.h>
#include <util/system/tempfile.h>

#define TEST_CASE_NAME (this->Name_)

namespace NPersQueue {

class SDKTestSetup {
protected:
    TString TestCaseName;

    THolder<TTempFileHandle> NetDataFile;
    THashMap<TString, NKikimr::NPersQueueTests::TPQTestClusterInfo> DataCenters;
    TString LocalDC = "dc1";
    TTestServer Server = TTestServer(false /* don't start */);

    TLog Log = TLog("cerr");

public:
    SDKTestSetup(const TString& testCaseName, bool start = true)
        : TestCaseName(testCaseName)
    {
        InitOptions();
        if (start) {
            Start();
        }
    }

    void InitOptions() {
        Log.SetFormatter([testCaseName = TestCaseName](ELogPriority priority, TStringBuf message) {
            return TStringBuilder() << TInstant::Now() << " :" << testCaseName << " " << priority << ": " << message << Endl;
        });
        Server.GrpcServerOptions.SetGRpcShutdownDeadline(TDuration::Max());
        // Default TTestServer value for 'MaxReadCookies' is 10. With this value the tests are flapping with two errors:
        // 1. 'got more than 10 unordered cookies to commit 12'
        // 2. 'got more than 10 uncommitted reads'
        Server.ServerSettings.PQConfig.Clear();
        Server.ServerSettings.PQConfig.SetEnabled(true);
        Server.ServerSettings.PQConfig.SetRemoteClusterEnabledDelaySec(1);
        Server.ServerSettings.PQConfig.SetCloseClientSessionWithEnabledRemotePreferredClusterDelaySec(1);
        Server.ServerSettings.PQClusterDiscoveryConfig.SetEnabled(true);
        // NOTE(shmel1k@): KIKIMR-14221
        Server.ServerSettings.PQConfig.SetCheckACL(false);
        Server.ServerSettings.PQConfig.SetRequireCredentialsInNewProtocol(false);
        Server.ServerSettings.PQConfig.SetClusterTablePath("/Root/PQ/Config/V2/Cluster");
        Server.ServerSettings.PQConfig.SetVersionTablePath("/Root/PQ/Config/V2/Versions");
        Server.ServerSettings.PQConfig.SetTopicsAreFirstClassCitizen(false);
        Server.ServerSettings.PQConfig.SetRoot("/Root/PQ");
        Server.ServerSettings.PQConfig.MutableQuotingConfig()->SetEnableQuoting(false);

        SetNetDataViaFile("::1/128\t" + GetLocalCluster());

        auto seed = TInstant::Now().MicroSeconds();
        // This makes failing randomized tests (for example with NUnitTest::RandomString(size, std::rand()) calls) reproducable
        Log << TLOG_INFO << "Random seed for debugging is " << seed;
        std::srand(seed);
    }

    void Start(bool waitInit = true, bool addBrokenDatacenter = false) {
        Server.StartServer(false);
        //Server.EnableLogs({NKikimrServices::PQ_WRITE_PROXY, NKikimrServices::PQ_READ_PROXY});
        Server.AnnoyingClient->InitRoot();
        if (DataCenters.empty()) {
            THashMap<TString, NKikimr::NPersQueueTests::TPQTestClusterInfo> dataCenters;
            dataCenters.emplace("dc1", NKikimr::NPersQueueTests::TPQTestClusterInfo{TStringBuilder() << "localhost:" << Server.GrpcPort, true});
            if (addBrokenDatacenter) {
                dataCenters.emplace("dc2", NKikimr::NPersQueueTests::TPQTestClusterInfo{"dc2.logbroker.yandex.net", false});
            }
            Server.AnnoyingClient->InitDCs(dataCenters);
        } else {
            Server.AnnoyingClient->InitDCs(DataCenters, LocalDC);
        }
        Server.AnnoyingClient->InitSourceIds();
        CreateTopic(GetTestTopic(), GetLocalCluster());
        if (waitInit) {
            Server.WaitInit(GetTestTopic());
        }
    }

    static TString GetTestTopic() {
        return "test-topic";
    }

    static TString GetTestClient() {
        return "test-reader";
    }

    static TString GetTestMessageGroupId() {
        return "test-message-group-id";
    }

    TString GetLocalCluster() const {
        return LocalDC;
    }

    ui16 GetGrpcPort() const {
        return Server.GrpcPort;
    }

    NGrpc::TServerOptions& GetGrpcServerOptions() {
        return Server.GrpcServerOptions;
    }

    void SetNetDataViaFile(const TString& netDataTsv) {
        NetDataFile = MakeHolder<TTempFileHandle>();
        NetDataFile->Write(netDataTsv.Data(), netDataTsv.Size());
        NetDataFile->FlushData();
        Server.ServerSettings.NetClassifierConfig.SetNetDataFilePath(NetDataFile->Name());
    }


    TLog& GetLog() {
        return Log;
    }

    template <class TConsumerOrProducer>
    void Start(const THolder<TConsumerOrProducer>& obj) {
        auto startFuture = obj->Start();
        const auto& initResponse = startFuture.GetValueSync();
        UNIT_ASSERT_C(!initResponse.Response.HasError(), "Failed to start: " << initResponse.Response);
    }

    void WriteToTopic(const TVector<TString>& data, bool compress = true) {

        auto client = NYdb::NPersQueue::TPersQueueClient(*(Server.AnnoyingClient->GetDriver()));
        NYdb::NPersQueue::TWriteSessionSettings settings;
        settings.Path(GetTestTopic()).MessageGroupId(GetTestMessageGroupId());
        if (!compress) settings.Codec(NYdb::NPersQueue::ECodec::RAW);
        auto writer = client.CreateSimpleBlockingWriteSession(settings);

        for (const TString& d : data) {
            Log << TLOG_INFO << "WriteToTopic: " << d;
            auto res = writer->Write(d);
            UNIT_ASSERT(res);
        }
        writer->Close();
    }

    void SetSingleDataCenter(const TString& name = "dc1") {
        UNIT_ASSERT(
                DataCenters.insert(std::make_pair(
                        name,
                        NKikimr::NPersQueueTests::TPQTestClusterInfo{TStringBuilder() << "localhost:" << Server.GrpcPort, true}
                )).second
        );
        LocalDC = name;
    }

    void AddDataCenter(const TString& name, const TString& address, bool enabled = true, bool setSelfAsDc = true) {
        if (DataCenters.empty() && setSelfAsDc) {
            SetSingleDataCenter();
        }
        NKikimr::NPersQueueTests::TPQTestClusterInfo info{
            address,
            enabled
        };
        UNIT_ASSERT(DataCenters.insert(std::make_pair(name, info)).second);
    }

    void AddDataCenter(const TString& name, const SDKTestSetup& cluster, bool enabled = true, bool setSelfAsDc = true) {
        AddDataCenter(name, TStringBuilder() << "localhost:" << cluster.Server.GrpcPort, enabled, setSelfAsDc);
    }

    void EnableDataCenter(const TString& name) {
        auto iter = DataCenters.find(name);
        UNIT_ASSERT(iter != DataCenters.end());
        Server.AnnoyingClient->UpdateDcEnabled(name, true);
    }
    void DisableDataCenter(const TString& name) {
        auto iter = DataCenters.find(name);
        UNIT_ASSERT(iter != DataCenters.end());
        Server.AnnoyingClient->UpdateDcEnabled(name, false);
    }

    void ShutdownGRpc() {
        Server.ShutdownGRpc();
    }

    void EnableGRpc() {
        Server.EnableGRpc();
        Server.WaitInit(GetTestTopic());
    }

    void KickTablets() {
        for (ui32 i = 0; i < Server.CleverServer->StaticNodes() + Server.CleverServer->DynamicNodes(); i++) {
            Server.AnnoyingClient->MarkNodeInHive(Server.CleverServer->GetRuntime(), i, false);
        }
        for (ui32 i = 0; i < Server.CleverServer->StaticNodes() + Server.CleverServer->DynamicNodes(); i++) {
            Server.AnnoyingClient->KickNodeInHive(Server.CleverServer->GetRuntime(), i);
        }
    }
    void AllowTablets() {
        for (ui32 i = 0; i < Server.CleverServer->StaticNodes() + Server.CleverServer->DynamicNodes(); i++) {
            Server.AnnoyingClient->MarkNodeInHive(Server.CleverServer->GetRuntime(), i, true);
        }
    }

    void CreateTopic(const TString& topic, const TString& cluster, size_t partitionsCount = 1) {
        Server.AnnoyingClient->CreateTopic(BuildFullTopicName(topic, cluster), partitionsCount);
    }

    void KillPqrb(const TString& topic, const TString& cluster) {
        auto describeResult = Server.AnnoyingClient->Ls(TStringBuilder() << "/Root/PQ/" << BuildFullTopicName(topic, cluster));
        UNIT_ASSERT_C(describeResult->Record.GetPathDescription().HasPersQueueGroup(), describeResult->Record);
        Server.AnnoyingClient->KillTablet(*Server.CleverServer, describeResult->Record.GetPathDescription().GetPersQueueGroup().GetBalancerTabletID());
    }
};
}
