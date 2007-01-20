



#include "spinny/settings.hpp"

namespace Spinny {

class settings_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "settings";
	};
	virtual int num_fields() const {
		return 2;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"name",
			"value",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"string",
			"string",
		};
		return field_types;
	};
};


void
Settings::link_up(){
	static settings_desc table_desc;
};


} // namespace Spinny
