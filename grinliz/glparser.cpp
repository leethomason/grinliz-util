#include "glparser.h"
#include "gldebug.h"

bool CSVTest(const char* in, int expectedRow, int expectedCol, const char* first, const char* last)
{
	grinliz::CSVParser parser;
	parser.Parse(in);

	int r = 0;
	int c = 0;
	for (const grinliz::CSVParser::Row& row : parser.Rows()) {
		c = 0;
		for (const std::string_view& value : row) {
			std::string s(value);
			//printf("r%d c%d %s\n", r, c, s.c_str());
			if (c == 0 && r == 0) {
				GLASSERT(value == first);
			}
			++c;
		}
		++r;
	}
	GLASSERT(r == expectedRow);
	GLASSERT(c == expectedCol);
	if (r && c) {
		GLASSERT(parser.Rows()[r - 1][c - 1] == last);
	}
	return true;
}

bool grinliz::TestCSV()
{
	{
		const char* test0 =
			"aa,bb,cc,dd,\n"
			"e,f,g,h";
		CSVTest(test0, 2, 4, "aa", "h");

		const char* test1 =
			"aa,bb,cc,dd,\n\r"
			"e,f,g,h,\r\n"
			"i,j,k,lima";
		CSVTest(test1, 3, 4, "aa", "lima");

		const char* test2 = "";
		CSVTest(test2, 0, 0, "", "");

		const char* test3 = "a,,c,";
		CSVTest(test3, 1, 3, "a", "c");

		// note that this parser can't distinguish 
		// end of line comma vs. end of line data.
		// Can pass ill formed CSV
		const char* test4 = "a,b,c";
		CSVTest(test4, 1, 3, "a", "c");
	}
	return true;
}


void grinliz::CSVParser::Parse(const char* p)
{
	m_rows.clear();

	const char* token = p;
	while (*p) {
		if (*p == '\n' || *p == '\r') {
			if (p > token) {
				ParseLine(token, p);
			}
			while (*p && (*p == '\n' || *p == '\r')) {
				++p;
			}
			token = p;
		}
		else {
			++p;
		}
	}
	if (p > token) {
		ParseLine(token, p);
	}
}


void grinliz::CSVParser::ParseLine(const char* start, const char* end)
{
	GLASSERT(end > start);

	Row row;

	const char* token = start;
	const char* p = start;
	while (true) {
		if (p == end) {
			if (p > token) 
				row.emplace_back(std::string_view(token, p - token));
			break;
		}
		else if (*p == m_delim) {
			row.emplace_back(std::string_view(token, p - token));
			++p;
			token = p;
		}
		else {
			++p;
		}
	}
	m_rows.emplace_back(row);
}