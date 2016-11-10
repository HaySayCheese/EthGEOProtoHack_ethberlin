#ifndef GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"
#include "../messages/Message.h"

#include <boost/date_time.hpp>
#include <boost/asio.hpp>

#include <map>


using namespace std;
using namespace boost::asio::ip;


class MessagesParser {
public:
    pair<bool, shared_ptr<Message>> processMessage(const char* messagePart, const size_t receivedBytesCount);

private:
    pair<bool, shared_ptr<Message>> tryDeserializeMessage();

private:
    // Messages may arrive via network partially.
    // This buffer is needed to collect all the parts
    // and deserialize whole the message.
    //
    // This buffer is separated from the boost::asio buffer,
    // that is used for reading info from the socket.
    vector<char> mMessageBuffer;
};


// ToDo: add removing of obsolete parsers;
class IncomingMessagesHandler {
public:
    IncomingMessagesHandler();
    ~IncomingMessagesHandler();

    void processIncomingMessage(udp::endpoint &clientEndpoint, const char *messagePart,
                                const size_t receivedBytesCount);

private:
    const pair<bool, shared_ptr<MessagesParser>> endpointMessagesParser(udp::endpoint &) const;
    shared_ptr<MessagesParser> registerNewEndpointParser(udp::endpoint &clientEndpoint);

private:
    map<udp::endpoint, shared_ptr<MessagesParser>> mParsers;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H