#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/SetTrustLineMessage.h"
#include "../../../../network/messages/trust_lines/UpdateTrustLineMessage.h"

#include "UpdateTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class SetTrustLineTransaction: public TrustLineTransaction {
public:
    typedef shared_ptr<SetTrustLineTransaction> Shared;

private:
    enum Stages {
        CheckUnicity = 1,
        CheckOutgoingDirection,
        CheckContext
    };

public:
    SetTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    SetTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    SetTrustLineCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isOutgoingTrustLineDirectionExisting();

    TransactionResult::SharedConst checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    void setOutgoingTrustAmount();

    void logSetTrustLineOperation();

    TransactionResult::SharedConst resultOk();

    TransactionResult::SharedConst resultTrustLineIsAbsent();

    TransactionResult::SharedConst resultConflictWithOtherOperation();

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst resultCurrentIncomingDebtIsGreaterThanNewAmount();

    TransactionResult::SharedConst resultProtocolError();



private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    SetTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
