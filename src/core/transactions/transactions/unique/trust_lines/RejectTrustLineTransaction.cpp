#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    const NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}


RejectTrustLineTransaction::RejectTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst RejectTrustLineTransaction::run() {

    try {
        if (isIncomingTrustLineDirectionExisting()) {

            if (checkDebt()) {
                suspendTrustLineDirectionFromContractor();
                sendResponseCodeToContractor(
                        RejectTrustLineMessage::kResultCodeRejectDelayed);

                return transactionResultFromMessage(
                        mMessage->resultRejectDelayed());

            } else {
                rejectTrustLine();
                logRejectingTrustLineOperation();
                sendResponseCodeToContractor(
                        RejectTrustLineMessage::kResultCodeRejected);

                return transactionResultFromMessage(
                        mMessage->resultRejected());
            }

        } else {
            sendResponseCodeToContractor(
                    RejectTrustLineMessage::kResultCodeTrustLineAbsent);
            return transactionResultFromMessage(
                    mMessage->resultRejected());
        }

    } catch (exception &e) {
        throw RuntimeError("RejectTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + string(e.what()));
    }
}

bool RejectTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool RejectTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Both);
}

void RejectTrustLineTransaction::suspendTrustLineDirectionFromContractor() {

    return mTrustLinesManager->suspendDirection(
        mMessage->contractorUUID(),
        TrustLineDirection::Incoming);

}

void RejectTrustLineTransaction::rejectTrustLine() {

    mTrustLinesManager->reject(
        mMessage->contractorUUID());
}

void RejectTrustLineTransaction::logRejectingTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Rejecting,
        mMessage->senderUUID);

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}

bool RejectTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mMessage->contractorUUID()) == BalanceRange::Negative;
}

void RejectTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}