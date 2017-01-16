#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H

#include "../../common/Types.h"
#include "../TransactionUUID.h"
#include "../../common/NodeUUID.h"

#include "../../network/messages/Message.h"

#include "result/TransactionResult.h"

#include "../../db/uuid_map_block_storage/UUIDMapBlockStorage.h"


namespace storage = db::uuid_map_block_storage;


class BaseTransaction {
public:
    typedef shared_ptr<BaseTransaction> Shared;

public:
    enum TransactionType {
        OpenTrustLineTransactionType,
        AcceptTrustLineTransactionType,
        UpdateTrustLineTransactionType,
        SetTrustLineTransactionType,
        CloseTrustLineTransactionType,
        RejectTrustLineTransactionType,
        MaximalAmountTransactionType,
        UseCreditTransactionType,
        TotalBalanceTransactionType,
        ContractorsListTransactionType
    };
    typedef uint8_t SerializedTransactionType;

public:
    virtual const TransactionType type() const;

    virtual const TransactionUUID uuid() const;

    virtual TransactionResult::Shared run() = 0;

    virtual void setContext(
        Message::Shared message) = 0;

    virtual pair<byte *, size_t> serializeContext() = 0;

protected:
    BaseTransaction(
        TransactionType type);

protected:
    TransactionType mType;
    TransactionUUID mTransactionUUID;
    Message::Shared mContext;

};


#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
