#ifndef GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../crypto/keychain.h"
#include "../../../crypto/lamportkeys.h"

#include "../../../network/messages/trust_lines/PublicKeyMessage.h"
#include "../../../network/messages/trust_lines/PublicKeyCRCConfirmation.h"

#include "PublicKeysSharingSourceTransaction.h"
#include "InitialAuditSourceTransaction.h"

using namespace crypto;

class PublicKeysSharingTargetTransaction : public BaseTransaction {

public:
    typedef shared_ptr<PublicKeysSharingTargetTransaction> Shared;

public:
    PublicKeysSharingTargetTransaction(
        const NodeUUID &nodeUUID,
        PublicKeyMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialisation = 1,
        ReceiveNextKey = 2,
    };

protected: // log
    const string logHeader() const;

private:
    TransactionResult::SharedConst runInitialisationStage();

    TransactionResult::SharedConst runReceiveNextKeyStage();

private:
    static const uint32_t kWaitMillisecondsForResponse = 3000;

protected:
    PublicKeyMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    Keystore *mKeysStore;
    KeysCount mReceivedKeysCount;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYSSHARINGTARGETTRANSACTION_H