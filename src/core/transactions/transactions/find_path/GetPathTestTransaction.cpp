#include "GetPathTestTransaction.h"

GetPathTestTransaction::GetPathTestTransaction(
    NodeUUID &nodeUUID,
    FindPathCommand::Shared command,
    ResourcesManager *resourcesManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GetPathTestTransactionType,
        nodeUUID,
        logger),

    mCommand(command),
    mResourcesManager(resourcesManager),
    mRequestCounter(0) {}

FindPathCommand::Shared GetPathTestTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst GetPathTestTransaction::run() {

    info() << "run\t" << UUID() << " I am " << mNodeUUID;
    info() << "run\tget paths to " << mCommand->contractorUUID();

    if (!mResources.empty()) {
        return checkResourcesContext();

    } else {
        if (mRequestCounter >= kMaxRequestsCount) {
            error() << "no resources";
            return make_shared<const TransactionResult>(
                TransactionState::exit());
        }
        if (mRequestCounter == 0) {
            info() << "run\t request resources";
            mResourcesManager->requestPaths(
                UUID(),
                mCommand->contractorUUID());
        }
        mRequestCounter++;
    }
    return waitingForResourcesState();
}

TransactionResult::SharedConst GetPathTestTransaction::waitingForResourcesState() {

    info() << "waitingForResourcesState";
    return transactionResultFromState(
        TransactionState::waitForResourcesTypes(
            {BaseResource::ResourceType::Paths},
            kConnectionTimeout));
}

TransactionResult::SharedConst GetPathTestTransaction::checkResourcesContext() {

    info() << "checkResourcesContext\tresources size: " << mResources.size();
    if (mResources.size() == 1) {
        auto responseResource = *mResources.begin();

        if (responseResource->type() == BaseResource::ResourceType::Paths) {
            PathsResource::Shared response = static_pointer_cast<PathsResource>(
                responseResource);
            info() << "checkResourcesContext\t receive paths count: " << response->pathCollection()->count();
            /*response->pathCollection()->resetCurrentPath();
            while (response->pathCollection()->hasNextPath()) {
                info() << response->pathCollection()->nextPath()->toString();
            }*/
            return make_shared<const TransactionResult>(
                TransactionState::exit());
        }

        error() << "checkResourcesContext\twrong resource type";
        return make_shared<const TransactionResult>(
            TransactionState::exit());

    } else {
        throw ConflictError("GetPathTestTransaction::checkTransactionContext: "
                                "Unexpected context size.");
    }
}

const string GetPathTestTransaction::logHeader() const
{
    stringstream s;
    s << "[GetPathTestTA]";

    return s.str();
}
