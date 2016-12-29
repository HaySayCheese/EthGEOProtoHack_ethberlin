#include "../../../core/db/fields/tl_direction_column/TrustLineDirectionColumn.h"
#include <boost/date_time.hpp>


using namespace db;
using namespace db::fields;
namespace f = boost::filesystem;


class TrustLineDirectionColumnTests {
public:
    void run() {
        checkFileCreation();
        checkSetOperation();
        checkSetOperationSpeed();
        checkSyncSetOperationSpeed();
        checkRemoveOperation();
        clean();
    }

private:
    f::path path() {
        return f::path("tests/db/trust_line_direction_column/");
    }

    void clean() {
        f::remove_all(path());
    }

    void checkFileCreation(){
        clean();

        TrustLineDirectionColumn column(path());
        assert(f::exists(path()));
    };

    void checkSetOperation(){
        clean();

        TrustLineDirectionColumn column(path());

        for (AbstractRecordsHandler::RecordNumber i=0; i<10; i++) {
            column.set(i, Incoming);
            assert(column.direction(i) == Incoming);
        }

        for (AbstractRecordsHandler::RecordNumber i=11; i<20; i++) {
            column.set(i, Outgoing);
            assert(column.direction(i) == Outgoing);
        }

        for (AbstractRecordsHandler::RecordNumber i=21; i<30; i++) {
            column.set(i, Both);
            assert(column.direction(i) == Both);
        }
    };

    void checkSetOperationSpeed(){
        clean();

        TrustLineDirectionColumn column(path());
        boost::posix_time::ptime started = boost::posix_time::microsec_clock::universal_time();

        for (AbstractRecordsHandler::RecordNumber i=0; i<300; i++) {
            column.set(i, Incoming);
        }
        for (AbstractRecordsHandler::RecordNumber i=301; i<600; i++) {
            column.set(i, Outgoing);
        }
        for (AbstractRecordsHandler::RecordNumber i=601; i<900; i++) {
            column.set(i, Both);
        }
        boost::posix_time::ptime finished = boost::posix_time::microsec_clock::universal_time();

        std::cout << "checkSetOperationSpeed(): 900 records in "
                  << (finished - started).total_milliseconds()
                  << "msecs in total."
                  << std::endl;
    };

    void checkSyncSetOperationSpeed(){
        clean();

        TrustLineDirectionColumn column(path());
        boost::posix_time::ptime started = boost::posix_time::microsec_clock::universal_time();

        for (AbstractRecordsHandler::RecordNumber i=0; i<300; i++) {
            column.set(i, Incoming, true);
        }
        for (AbstractRecordsHandler::RecordNumber i=301; i<600; i++) {
            column.set(i, Outgoing, true);
        }
        for (AbstractRecordsHandler::RecordNumber i=601; i<900; i++) {
            column.set(i, Both, true);
        }
        boost::posix_time::ptime finished = boost::posix_time::microsec_clock::universal_time();

        std::cout << "checkSyncSetOperationSpeed(): 900 records in "
                  << (finished - started).total_milliseconds()
                  << "msecs in total."
                  << std::endl;
    };

    void checkRemoveOperation(){
        clean();

        TrustLineDirectionColumn column(path());

        for (AbstractRecordsHandler::RecordNumber i=0; i<10; i++) {
            column.set(i, Incoming);
            column.remove(i);

            try {
                auto d = column.direction(i);
                assert(false);
            } catch (NotFoundError &) {
                // ok
            } catch (...) {
                assert(false);
            }
        }

        for (AbstractRecordsHandler::RecordNumber i=11; i<20; i++) {
            column.set(i, Outgoing);
            column.remove(i);

            try {
                auto d = column.direction(i);
                assert(false);
            } catch (NotFoundError &) {
                // ok
            } catch (...) {
                assert(false);
            }
        }

        for (AbstractRecordsHandler::RecordNumber i=21; i<30; i++) {
            column.set(i, Both);
            column.remove(i);

            try {
                auto d = column.direction(i);
                assert(false);
            } catch (NotFoundError &) {
                // ok
            } catch (...) {
                assert(false);
            }
        }
    };
};