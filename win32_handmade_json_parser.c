// TODO(simone): to improve errors we want to track current state of parsing, based upon the state we are currently in, we can narrow down the error class
// and be more specific in error messages. But is it the goal?

#include <stdio.h>
#include <windows.h>
#include "win32_handmade_json_parser.h"

int x = 0;

internal inline uint64 find_comma(const char *json_value, const uint64 file_size, int i)
{
	if (x==0 || i >= file_size - 3)
	{
		return;
	}

	uint64 x = 0;
	while(json_value[x] == ' ' || json_value[x] == '\t' ||
		json_value[x] == '\r' || json_value[x] == '\n')
	{
		++x;
	}
	if(json_value[x] != ',' && json_value[x] != ']' && json_value[x] != '}') // Square bracket because nested elem.
	{
		putchar(json_value[x]);
		putchar("\n");
		puts("panic: malformed json");
		exit(1);
	}
	//++i;
	return x;
}

internal void debug_print_string(const char *debug_string, uint64 length)
{
	int i = 0;
	puts("***STRING DEBUGGING: ");
	while(i < length)
	{
		putchar(debug_string[i]);
		++i;
	}
	puts(" ***\n");
}

internal int parse_and_extract_key(const char *json_value, normalized_key *key)
{
	int i = 0;
	while(json_value[i] == ' ' || json_value[i] == '\t' ||
		json_value[i] == '\r' || json_value[i] == '\n')
	{
		++i;
	}
	char temp_debug = json_value[i];
	//TODO(simone): verify if this comma if condition is required still.
	if(json_value[i] == ',')
	{
		++i;
		while(json_value[i] == ' ' || json_value[i] == '\t' ||
			json_value[i] == '\r' || json_value[i] == '\n')
		{
			++i;
		}
		if(json_value[i] != '\"')
		{
			puts("panic: invalid key");
			exit(1);
		}
	}
	else if (json_value[i] != '\"')
	{
		puts("panic: invalid key");
		exit(1);
	}

	uint64 left_blanks = i;
	++i;
	uint64 right_blanks = 0;
	int8 end_quotes = 0;
	int8 end_colon = 0;
	while(end_quotes == 0 || end_colon == 0)
	{
		temp_debug = json_value[i];
		if((json_value[i] == ' ' || json_value[i] == '\t' ||
			json_value[i] == '\r' || json_value[i] == '\n') &&  end_quotes == 1)
		{
			++right_blanks;
			++i;
			continue;
		}
		else if(end_quotes && json_value[i] != ':')
		{
			puts("panic: trailing char found in key");
			exit(1);
		}
		if(!end_quotes && json_value[i] == '\"' && json_value[i-1] != '\\')
		{
			end_quotes = 1;
		}
		else if(json_value[i] == '\"' && json_value[i-1] == '\\')
		{
			puts("panic! invalid key format. offending char not properly escaped");
			exit(1);
		}
		if(end_quotes && json_value[i] == ':')
		{
			end_colon = 1;
			continue;
		}
		
		if((end_quotes == 0 || end_colon == 0) && (json_value[i] == '}' || json_value[i] == ']'))
		{
			puts("panic! invalid key");
			exit(1);
		}
		++i;	
	}
	if (right_blanks > i)
	{
		puts("panic!");
		exit(1);
	}
	key->length = i - right_blanks - left_blanks;
	key->string = json_value + left_blanks;
	debug_print_string(key->string, key->length);
	return i;
}

internal int parse_and_extract_value(const char *json_value, normalized_value *value, uint64 file_size)
{
	int i = 0;
	char debug_value_tmp = json_value[i];
	while(json_value[i] == ' ' || json_value[i] == '\t' ||
		json_value[i] == '\r' || json_value[i] == '\n')
	{
		++i;
	}
	debug_value_tmp = json_value[i];
	uint64 left_blanks = i;
	switch(json_value[i])
	{
		case '\"':
		{
			++i;
			int8 end_quotes = 0;
			while(end_quotes == 0)
			{
				debug_value_tmp = json_value[i];
				if(json_value[i] == '\"' && json_value[i-1] != '\\')
				{
					end_quotes = 1;
					++i;
					continue;
				}
				else if(json_value[i] == '\"' && json_value[i-1] == '\\')
				{
					puts("panic! invalid value format. offending char not properly escaped");
					exit(1);
				}
				if(json_value[i] == '}' || json_value[i] == ']')
				{
					puts("panic! reached end of file without parsing a valid value");
					exit(1);
				}
				++i;
			}
			value->length = i - left_blanks;
			value->string = json_value + left_blanks;
			//debug_print_string(value->string, value->length);
			++x;
		}
		break;
		case 't':
		{
			++i;
			if(json_value[i] == 'r')
			{
				++i;
			}
			if(json_value[i] == 'u')
			{
				++i;
			}
			if(json_value[i] != 'e')
			{
				puts("panic: invalid boolean");
				exit(1);
			}
			++i;
			value->length = 4;
			value->string = json_value + left_blanks;
			//debug_print_string(value->string, value->length);
			++x;
		}
		break;
		case 'f':
		{
			++i;
			if(json_value[i] == 'a')
			{
				++i;
			}
			if(json_value[i] == 'l')
			{
				++i;
			}
			if(json_value[i] == 's')
			{
				++i;
			}
			if(json_value[i] != 'e')
			{
				puts("panic: invalid boolean");
				exit(1);
			}
			++i;
			value->length = 5;
			value->string = json_value + left_blanks;
			//debug_print_string(value->string, value->length);
			++x;
		}
		break;
		case '[':
		{
			++i;
			char tmp_debug = '\0';
			while(json_value[i] != ']')
			{
				i+=parse_and_extract_value(json_value + i, value, file_size);
				tmp_debug = json_value[i];
				while(json_value[i] == ' ' || json_value[i] == '\t' ||
					json_value[i] == '\r' || json_value[i] == '\n')
				{
					++i;
				}
				if(json_value[i] == ',')
				{
					++i;
					continue;
				}
			}
			++i;
			value->length = i - left_blanks;
			value->string = json_value + left_blanks;
			//debug_print_string(value->string, value->length);
			++x;
		}
		break;
		case '{':
		{

			++i;
			char tmp_debug = json_value[i];
			while(json_value[i] != '}')
			{
				while(json_value[i] == ' ' || json_value[i] == '\t' ||
						json_value[i] == '\r' || json_value[i] == '\n')
				{
					++i;
				}
				tmp_debug = json_value[i];
				if(json_value[i] == '}')
				{
					tmp_debug = json_value[i];
					continue;
				}
				i += (parse_and_extract_key(json_value + i, value) + 1);
				tmp_debug = json_value[i];
				i += (parse_and_extract_value(json_value + i, value, file_size));
				tmp_debug = json_value[i];
			}
			++i;
			tmp_debug = json_value[i];
			value->length = i - left_blanks;
			value->string = json_value + left_blanks;
			++x;
		}
		break;
		default:
		{
			if(json_value[i] >= 48 && json_value[i] <= 57)
			{
				char debug_char = json_value[i];
				++i;
				debug_char = json_value[i];
				while(json_value[i] >= 48 && json_value[i] <= 57)
				{
					++i;
					debug_char = json_value[i];
				}
				if(json_value[i] != ' ' && json_value[i] != '\t' &&
					json_value[i] != '\r' && json_value[i] != '\n' && 
						json_value[i] != ',' && json_value[i] != '}' && json_value[i] != ']')
				{
					puts("panic: invalid number");
					exit(1);
				}
				value->length = i - left_blanks;
				value->string = json_value + left_blanks;
				//debug_print_string(value->string, value->length);
				++x;
			}
			else
			{
				puts("panic: invalid value");
				exit(1);
			}
		}
	}
	i += find_comma(json_value + i, file_size, i);
	return i;
}

int main(int argc, char **argv)
{
	HANDLE file = CreateFileA("file.json", GENERIC_READ, 0, 
			0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (file == INVALID_HANDLE_VALUE)
	{
		exit(1);
	}
	LARGE_INTEGER file_size;
	if(GetFileSizeEx(file, &file_size) == 0)
	{
		exit(1);
	}
	LONGLONG file_size_in_bytes = file_size.QuadPart;
	HANDLE file_map = CreateFileMappingA(file, 0, PAGE_READONLY, 0, 0, 0);
	if (file_map == 0)
	{
		exit(1);
	}
	const char* file_data = MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 0);
	if (file_data == 0)
	{
		exit(1);
	}
	
	//NOTE(simone): verify if this int is suitable
	int keys_count = 0;
	int values_count = 0;
	char last_char = '\0';
	char debug_current_char = '\0';

	normalized_key key_to_print = {};
	normalized_value string_to_print = {};
	normalized_object object = {};

	int i = 0;
	int elem = 0;

	while(i < file_size_in_bytes-3)
	{
		debug_current_char = file_data[i];
		if(file_data[i] == '\n' || file_data[i] == ' ' || file_data[i] == '\r' || file_data[i] == '\t')
		{
			++i;
			continue;
		}

		i += (parse_and_extract_value(file_data + i, &string_to_print,
			file_size_in_bytes - 3)); 
		debug_current_char = file_data[i];
		//+1 indicates to start further after parsing ':'
		// i += (parse_and_extract_key(file_data + i, &key_to_print) + 1);
		// debug_current_char = file_data[i];
		last_char = file_data[i];
	}
	printf("total scanned keys: %d total values scanned %d\n", keys_count, values_count);

}

