#ifndef GEO_NETWORK_CLIENT_TRUSTLINE_H
#define GEO_NETWORK_CLIENT_TRUSTLINE_H

#include "../common/Types.h"
#include "../common/NodeUUID.h"
#include "../common/memory/MemoryUtils.h"
#include "../common/multiprecision/MultiprecisionUtils.h"

#include "../common/exceptions/RuntimeError.h"
#include "../common/exceptions/MemoryError.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/OverflowError.h"

#include <vector>


using namespace std;

// todo: tests?
// todo: hsc: review the tests.
class TrustLine {
public:
    typedef shared_ptr<TrustLine> Shared;
    typedef shared_ptr<const TrustLine> ConstShared;
    typedef uint16_t SerializedTrustLineState;

public:
    enum TrustLineState {
        Init = 1,
        Modify = 2,
        KeysPending = 3,
        AuditPending = 4,
        Active = 5,
        Archived = 6,
        Conflict = 7,
        ConflictResolving = 8,
        EthereumPending = 9,
    };

public:
    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineID trustLineID,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &nodeBalance,
        bool isContractorGateway,
        TrustLineState state = Init,
        AuditNumber auditNumber = kInitialAuditNumber);

    TrustLine(
        const NodeUUID &nodeUUID,
        const TrustLineID trustLineID,
        bool isContractorGateway,
        TrustLineState state);

    void setIncomingTrustAmount(
        const TrustLineAmount &amount);

    void setOutgoingTrustAmount(
        const TrustLineAmount &amount);

    void setBalance(
        const TrustLineBalance &balance);

    void pay(
        const TrustLineAmount &amount);

    void acceptPayment(
        const TrustLineAmount &amount);

    const NodeUUID& contractorNodeUUID() const;

    const TrustLineAmount& incomingTrustAmount() const;

    const TrustLineAmount& outgoingTrustAmount() const;

    const TrustLineBalance& balance() const;

    const TrustLineID trustLineID() const;

    const TrustLineState state() const;

    const AuditNumber currentAuditNumber() const;

    void setState(
        TrustLineState newState);

    void setAuditNumber(
        AuditNumber newAuditNumber);

    bool isStateChannel() const;

    const string contractorEthereumAddress() const;

    void setCotntractorEthereumAddress(
        const string contractorEthereumAddress);

    const string ethereumChannelId() const;

    void setEthereumChannelId(
        const string ethereumChannelId);

    void setAlice();

    bool isAlice() const;

    const uint64_t currentEpochId() const;

    void incrementEpochId();

    const uint64_t currentPaymentReceiptId() const;

    void incrementPaymentReceiptId();

    void resetPaymentReceiptId();

    ConstSharedTrustLineAmount availableOutgoingAmount() const;

    ConstSharedTrustLineAmount availableIncomingAmount() const;

    ConstSharedTrustLineAmount usedAmountByContractor() const;

    ConstSharedTrustLineAmount usedAmountBySelf() const;

    void setTotalOutgoingReceiptsAmount(
        const TrustLineAmount &amount);

    void setTotalIncomingReceiptsAmount(
        const TrustLineAmount &amount);

    bool isTrustLineOverflowed() const;

    void resetTotalReceiptsAmounts();

    bool isContractorGateway() const;

    void setContractorAsGateway(
        bool contractorAsGateway);

    static const TrustLineBalance& kZeroBalance();

    static const TrustLineAmount& kZeroAmount();

    friend bool operator== (
        const TrustLine::Shared contractor1,
        const TrustLine::Shared contractor2);

    friend bool operator== (
        const TrustLine &contractor1,
        const TrustLine &contractor2);

public:
    static const AuditNumber kInitialAuditNumber = 0;

private:
    NodeUUID mContractorNodeUUID;
    TrustLineID mID;
    TrustLineAmount mIncomingTrustAmount;
    TrustLineAmount mOutgoingTrustAmount;
    TrustLineBalance mBalance;
    TrustLineAmount mTotalOutgoingReceiptsAmount;
    TrustLineAmount mTotalIncomingReceiptsAmount;
    bool mIsContractorGateway;
    TrustLineState mState;
    AuditNumber mCurrentAudit;
    string mContractorEthereumAddress;
    string mEthereumChannelId;
    bool mIsAlice;
    uint64_t mCurrentEpochId;
    uint64_t mCurrentPaymentReceiptId;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINE_H
