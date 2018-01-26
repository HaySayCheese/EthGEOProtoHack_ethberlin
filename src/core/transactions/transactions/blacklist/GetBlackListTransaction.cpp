#include "GetBlackListTransaction.h"

GetBlackListTransaction::GetBlackListTransaction(
    NodeUUID &nodeUUID,
    GetBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    Logger &logger)
noexcept :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler)
{}

GetBlackListCommand::Shared GetBlackListTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetBlackListTransaction::run() {
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto kBannedUsers = ioTransaction->blackListHandler()->allNodesUUIDS();
    stringstream ss;
    ss << to_string(kBannedUsers.size());

    for (const auto kNodeUUID: kBannedUsers) {
        ss << "\t";
        ss << kNodeUUID;
    }
    ss << "\n";
    string kResultInfo = ss.str();
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}