
#include "testing.hpp"

#include "ews_client.hpp"
#include "google/template.h"
#include <google/template_from_string.h>
#include <google/template_dictionary.h>

#include <boost/algorithm/string.hpp>

SUITE(CTmpl) {

// This is esp. useful for calling from within gdb.
// The gdb nice-ness is balanced by the need for the caller to delete the buf.
	static const char* ExpandIs(google::Template* tpl, google::TemplateDictionary *dict) {
		string outstring;
		tpl->Expand(&outstring, dict);
		char* buf = new char [outstring.size()+1];
		strcpy(buf, outstring.c_str());
		return buf;
	}

	static google::Template* StringToTemplate(const string& s, google::Strip strip) {
		static int filenum = 0;
		char buf[16];
		snprintf(buf, sizeof(buf), "%03d", ++filenum);   // unique name for each call
		return google::TemplateFromString::GetTemplate(buf, s, strip);
	}


TEST( All ){

	google::TemplateDictionary dict("dict");
	dict.SetValue("FOO", "foo");

	const char* tests[][4] = {  // 0: in, 1: do-not-strip, 2: blanklines, 3: ws
		{"hi!\n", "hi!\n", "hi!\n", "hi!"},
		{"hi!", "hi!", "hi!", "hi!"},
		// These test strip-blank-lines, primarily
		{"{{FOO}}\n\n{{FOO}}", "foo\n\nfoo", "foo\nfoo", "foofoo"},
		{"{{FOO}}\r\n\r\n{{FOO}}", "foo\r\n\r\nfoo", "foo\r\nfoo", "foofoo"},
		{"{{FOO}}\n   \n{{FOO}}\n", "foo\n   \nfoo\n", "foo\nfoo\n", "foofoo"},
		{"{{FOO}}\n{{BI_NEWLINE}}\nb", "foo\n\n\nb", "foo\n\n\nb", "foo\nb"},
		// These test strip-whitespace
		{"foo\nbar\n", "foo\nbar\n", "foo\nbar\n", "foobar"},
		{"{{FOO}}\nbar\n", "foo\nbar\n", "foo\nbar\n", "foobar"},
		{"  {{FOO}}  {{!comment}}\nb", "  foo  \nb", "  foo  \nb", "foo  b"},
		{"  {{FOO}}  {{BI_SPACE}}\n", "  foo   \n", "  foo   \n", "foo   "},
		{"  \t \f\v  \n\r\n  ", "  \t \f\v  \n\r\n  ", "", ""},
	};

	for (int i = 0; i < sizeof(tests)/sizeof(*tests); ++i) {
		google::Template* tpl1 = StringToTemplate(tests[i][0], google::DO_NOT_STRIP);
		google::Template* tpl2 = StringToTemplate(tests[i][0], google::STRIP_BLANK_LINES);
		google::Template* tpl3 = StringToTemplate(tests[i][0], google::STRIP_WHITESPACE);

		const char* buf = ExpandIs( tpl1, &dict);
		CHECK_EQUAL( tests[i][1],buf );
		delete [] buf;

		buf = ExpandIs( tpl2, &dict);
		CHECK_EQUAL( tests[i][2],buf );
		delete [] buf;

		buf = ExpandIs( tpl3, &dict);
		CHECK_EQUAL( tests[i][3],buf );
		delete [] buf;
	}
}






} // SUITE(CTmpl)

int
ctemplate( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("CTmpl");

}

