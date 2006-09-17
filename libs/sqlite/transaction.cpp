#include "sqlite.hpp"

namespace sqlite {

	transaction::transaction( connection &con, bool start) : con(con),intrans(false) {
		if(start) begin();
	}

	transaction::~transaction() {
		if(intrans) {
			try {
				rollback();
			}
			catch(...) {
				return;
			}
		}
	}

	void transaction::begin() {
		con.exec<none>("begin;");
		intrans=true;
	}

	void transaction::commit() {
		con.exec<none>("commit;");
		intrans=false;
	}

	void transaction::rollback() {
		con.exec<none>("rollback;");
		intrans=false;
	}

} // namespace sqlite
