#ifndef GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../network/messages/trust_lines/PublicKeyMessage.h"
#include "../../../network/messages/trust_lines/PublicKeyCRCConfirmation.h"

using namespace crypto;

class PublicKeysSharingSourceTransaction : public BaseTransaction {

public:
    typedef shared_ptr<PublicKeysSharingSourceTransaction> Shared;

public:
    PublicKeysSharingSourceTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialisation = 1,
        SendNextKey = 2,
    };

protected: // log
    const string logHeader() const;

private:
    TransactionResult::SharedConst runInitialisationStage();

    TransactionResult::SharedConst runSendNextKeyStage();

private:
    static const uint32_t kKeysCount = 5;
    static const uint32_t kWaitMillisecondsForResponse = 3000;

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    KeyNumber mCurrentKeyNumber;
    lamport::PublicKey::Shared mCurrentPublicKey;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGSOURCETRANSACTION_H
