#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseCollectTopologyTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

#include "../../../network/messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"

#include "CollectTopologyTransaction.h"

#include <set>

class InitiateMaxFlowCalculationTransaction : public BaseCollectTopologyTransaction {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationTransaction> Shared;

public:
    InitiateMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationCommand::Shared command,
        TrustLinesManager *trustLinesManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        bool iAmGateway,
        Logger &logger);

    InitiateMaxFlowCalculationCommand::Shared command() const;

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst sendRequestForCollectingTopology();

    TransactionResult::SharedConst processCollectingTopology();

    TrustLineAmount calculateMaxFlow(
        const NodeUUID &contractorUUID);

    TrustLineAmount calculateMaxFlowUpdated(
        const NodeUUID &contractorUUID);

    void calculateMaxFlowOnOneLevel();

    void calculateMaxFlowOnOneLevelUpdated();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultOk(
        vector<pair<NodeUUID, TrustLineAmount>> &maxFlows);

    TransactionResult::SharedConst resultProtocolError();

private:
    static const byte kMaxPathLength = 6;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlow = 3000;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlowAgain = 500;
    static const uint32_t kMaxWaitMillisecondsForCalculatingMaxFlow = 10000;
    static const uint16_t kCountRunningProcessCollectingTopologyStage =
        (kMaxWaitMillisecondsForCalculatingMaxFlow - kWaitMillisecondsForCalculatingMaxFlow) /
            kWaitMillisecondsForCalculatingMaxFlowAgain;

private:
    InitiateMaxFlowCalculationCommand::Shared mCommand;
    vector<NodeUUID> mForbiddenNodeUUIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
    NodeUUID mCurrentContractor;
    size_t mCountProcessCollectingTopologyRun;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
