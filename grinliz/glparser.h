#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace grinliz {

	/*	A simple CSV parser.
		Basic efficiency but nothing fancy. Single threaded.
		No enforcement of column/row consistency or interpretation of the CSV 
		strings. Just streams it all in.

		Returns pointers back to the data in `Parse` so make sure
		not to throw that away while using it.

		Can specify a delimiter on the constructor, it's really a SV parser.

		Basic use:
		```
		grinliz::CSVParser parser;
		parser.Parse(in);

		for (const grinliz::CSVParser::Row& row : parser.Rows()) {
			for (const std::string_view& value : row) {
				// do something with value
			}
		}
		```
	*/
	class CSVParser
	{
	public:
		using Row = std::vector<std::string_view>;

		CSVParser(char delim = ',') : m_delim(delim) {}

		// Read a null terminated string of data.
		void Parse(const char*);
		const std::vector<Row>& Rows() const { return m_rows; }
	
	private:
		char m_delim = 0;
		std::vector<Row> m_rows;

		void ParseLine(const char* start, const char* end);
	};

	bool TestCSV();
}