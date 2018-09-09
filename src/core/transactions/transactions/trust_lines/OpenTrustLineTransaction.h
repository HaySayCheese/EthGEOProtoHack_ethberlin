#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"
#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineInitialMessage.h"

#include "../../../ethereum/connector.h"

class OpenTrustLineTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        const NodeUUID &nodeUUID,
        const string &ethereumAddress,
        SetOutgoingTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        bool iAmGateway,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
    noexcept;

    OpenTrustLineTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

    const string logHeader() const
    noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

    TransactionResult::SharedConst runRecoveryStage();

    TransactionResult::SharedConst runReceiveNextKeyStage();

    TransactionResult::SharedConst runChannelOpeningPending();

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    SetOutgoingTrustLineCommand::Shared mCommand;
    TrustLineAmount mAmount;
    SubsystemsController *mSubsystemsController;
    bool mIAmGateway;
    string mEthereumAddress;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
