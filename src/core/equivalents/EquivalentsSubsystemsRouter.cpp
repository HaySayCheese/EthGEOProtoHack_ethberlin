#include "EquivalentsSubsystemsRouter.h"

EquivalentsSubsystemsRouter::EquivalentsSubsystemsRouter(
    NodeUUID &nodeUUID,
    StorageHandler *storageHandler,
    as::io_service &ioService,
    vector<SerializedEquivalent> &equivalentsIAmGateway,
    Logger &logger):

    mNodeUUID(nodeUUID),
    mStorageHandler(storageHandler),
    mIOService(ioService),
    mLogger(logger)
{
    {
        auto ioTransaction = storageHandler->beginTransaction();
        mEquivalents = ioTransaction->trustLinesHandler()->equivalents();
    }
    for (const auto &equivalent : mEquivalents) {
        info() << "Equivalent " << equivalent;

        mIAmGateways.insert(
            make_pair(
                equivalent,
                find(
                    equivalentsIAmGateway.begin(),
                    equivalentsIAmGateway.end(),
                    equivalent) != equivalentsIAmGateway.end()));

        mTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TrustLinesManager>(
                    equivalent,
                    mStorageHandler,
                    mLogger)));
        info() << "Trust Lines Manager is successfully initialized";

        mTopologyTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TopologyTrustLinesManager>(
                    equivalent,
                    mIAmGateways[equivalent],
                    mNodeUUID,
                    mLogger)));
        info() << "Topology Trust Lines Manager is successfully initialized";

        mTopologyCacheManagers.insert(
            make_pair(
                equivalent,
                make_unique<TopologyCacheManager>(
                    equivalent,
                    mLogger)));
        info() << "Topology Cache Manager is successfully initialized";

        mMaxFlowCacheManagers.insert(
            make_pair(
                equivalent,
                make_unique<MaxFlowCacheManager>(
                    equivalent,
                    mLogger)));
        info() << "Max Flow Cache Manager is successfully initialized";

        mTopologyCacheUpdateDelayedTasks.insert(
            make_pair(
                equivalent,
                make_unique<TopologyCacheUpdateDelayedTask>(
                    equivalent,
                    mIOService,
                    mTopologyCacheManagers[equivalent].get(),
                    mTopologyTrustLinesManagers[equivalent].get(),
                    mMaxFlowCacheManagers[equivalent].get(),
                    mLogger)));
        info() << "Topology Cache Update Delayed Task is successfully initialized";

        mPathsManagers.insert(
            make_pair(
                equivalent,
                make_unique<PathsManager>(
                    equivalent,
                    mNodeUUID,
                    mTrustLinesManagers[equivalent].get(),
                    mTopologyTrustLinesManagers[equivalent].get(),
                    mLogger)));
        info() << "Paths Manager is successfully initialized";

        mEthereumSignaturesManagers.insert(
            make_pair(
                equivalent,
                make_unique<EthereumSiganturesManager>(
                    equivalent,
                    mLogger)));
        info() << "Ethereum Signatures Manager is successfully initialized";
    }

    mGatewayNotificationAndRoutingTablesDelayedTask = make_unique<GatewayNotificationAndRoutingTablesDelayedTask>(
        mIOService,
        mLogger);
    subscribeForGatewayNotification(
        mGatewayNotificationAndRoutingTablesDelayedTask->gatewayNotificationSignal);
    info() << "Gateway Notification and Routing Tables Delayed Task is successfully initialized";
}

vector<SerializedEquivalent> EquivalentsSubsystemsRouter::equivalents() const
{
    return mEquivalents;
}

bool EquivalentsSubsystemsRouter::iAmGateway(
    const SerializedEquivalent equivalent) const
{
    if (mIAmGateways.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::iAmGateway: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mIAmGateways.at(equivalent);
}

TrustLinesManager* EquivalentsSubsystemsRouter::trustLinesManager(
    const SerializedEquivalent equivalent) const
{
    if (mTrustLinesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::trustLinesManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTrustLinesManagers.at(equivalent).get();
}

TopologyTrustLinesManager* EquivalentsSubsystemsRouter::topologyTrustLineManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyTrustLinesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::topologyTrustLineManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyTrustLinesManagers.at(equivalent).get();
}

TopologyCacheManager* EquivalentsSubsystemsRouter::topologyCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyCacheManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::topologyCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyCacheManagers.at(equivalent).get();
}

MaxFlowCacheManager* EquivalentsSubsystemsRouter::maxFlowCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mMaxFlowCacheManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::maxFlowCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mMaxFlowCacheManagers.at(equivalent).get();
}

PathsManager* EquivalentsSubsystemsRouter::pathsManager(
    const SerializedEquivalent equivalent) const
{
    if (mPathsManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::pathsManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mPathsManagers.at(equivalent).get();
}

EthereumSiganturesManager* EquivalentsSubsystemsRouter::ethereumSiganturesManager(
    const SerializedEquivalent equivalent)
{
    if (mEthereumSignaturesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::ethereumSiganturesManager: "
                "wrong equivalent " + to_string(equivalent));
    }
    return mEthereumSignaturesManagers.at(equivalent).get();
}

void EquivalentsSubsystemsRouter::initNewEquivalent(
    const SerializedEquivalent equivalent)
{
    if (mTrustLinesManagers.count(equivalent) != 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::initNewEquivalent: "
                    "try init equivalent " + to_string(equivalent) + " which is already exists");
    }

    mIAmGateways.insert(
        make_pair(
            equivalent,
            false));

    mTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TrustLinesManager>(
                equivalent,
                mStorageHandler,
                mLogger)));
    info() << "Trust Lines Manager is successfully initialized";

    mTopologyTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TopologyTrustLinesManager>(
                equivalent,
                false,
                mNodeUUID,
                mLogger)));
    info() << "Topology Trust Lines Manager is successfully initialized";

    mTopologyCacheManagers.insert(
        make_pair(
            equivalent,
            make_unique<TopologyCacheManager>(
                equivalent,
                mLogger)));
    info() << "Topology Cache Manager is successfully initialized";

    mMaxFlowCacheManagers.insert(
        make_pair(
            equivalent,
            make_unique<MaxFlowCacheManager>(
                equivalent,
                mLogger)));
    info() << "Max Flow Cache Manager is successfully initialized";

    mTopologyCacheUpdateDelayedTasks.insert(
        make_pair(
            equivalent,
            make_unique<TopologyCacheUpdateDelayedTask>(
                equivalent,
                mIOService,
                mTopologyCacheManagers[equivalent].get(),
                mTopologyTrustLinesManagers[equivalent].get(),
                mMaxFlowCacheManagers[equivalent].get(),
                mLogger)));
    info() << "Topology Cache Update Delayed Task is successfully initialized";

    mPathsManagers.insert(
        make_pair(
             equivalent,
             make_unique<PathsManager>(
                 equivalent,
                 mNodeUUID,
                 mTrustLinesManagers[equivalent].get(),
                 mTopologyTrustLinesManagers[equivalent].get(),
                 mLogger)));
    info() << "Paths Manager is successfully initialized";

    mEthereumSignaturesManagers.insert(
        make_pair(
            equivalent,
            make_unique<EthereumSiganturesManager>(
                equivalent,
                mLogger)));
    info() << "Ethereum Signatures Manager is successfully initialized";

    mEquivalents.push_back(equivalent);
}

void EquivalentsSubsystemsRouter::subscribeForGatewayNotification(
    GatewayNotificationAndRoutingTablesDelayedTask::GatewayNotificationSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsSubsystemsRouter::onGatewayNotificationSlot,
            this));
}

void EquivalentsSubsystemsRouter::onGatewayNotificationSlot()
{
    gatewayNotificationSignal();
}

#ifdef TESTS
void EquivalentsSubsystemsRouter::setMeAsGateway()
{
    for (auto iAmGateway : mIAmGateways) {
        iAmGateway.second = true;
    }
}
#endif

string EquivalentsSubsystemsRouter::logHeader() const
{
    return "[EquivalentsSubsystemsRouter]";
}

LoggerStream EquivalentsSubsystemsRouter::error() const
{
    return mLogger.error(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::warning() const
{
    return mLogger.warning(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::debug() const
{
    return mLogger.debug(logHeader());
}
