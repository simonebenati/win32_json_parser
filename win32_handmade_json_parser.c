// TODO(simone): to improve errors we want to track current state of parsing, based upon the state we are currently in, we can narrow down the error class
// and be more specific in error messages. But is it the goal?

#include <stdio.h>
#include <windows.h>
#include "win32_handmade_json_parser.h"

#define int8 char
#define uint64 long
#define internal static
#define true 1
#define false 0

// NOTE(simone): The length here is the index length.
// Therefore in loops we must use equal to len or we skip the last char.
typedef struct
{
	uint64 length;
	char *string;
} normalized_value;

typedef struct
{
	uint64 length;
	char *string;
} normalized_key;

internal void debug_print_string(char *debug_string, uint64 length)
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

internal int parse_and_extract_key(char *json_value, normalized_key *key)
{
	int i = 0;
	while(json_value[i] == ' ' || json_value[i] == '\t' ||
		json_value[i] == '\r' || json_value[i] == '\n')
	{
		++i;
	}
	char temp_debug = json_value[i];
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
		
		if((end_quotes == 0 || end_colon == 0) && json_value[i] == '}')
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

internal int parse_and_extract_value(char *json_value, normalized_value *value)
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
					continue;
				}
				else if(json_value[i] == '\"' && json_value[i-1] == '\\')
				{
					puts("panic! invalid value format. offending char not properly escaped");
					exit(1);
				}
				if(json_value[i] == '}')
				{
					puts("panic! reached end of file without parsing a valid value");
					exit(1);
				}
				++i;
			}
			value->length = i - left_blanks;
			value->string = json_value + left_blanks;
			debug_print_string(value->string, value->length);
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
			value->length = 4;
			value->string = json_value + left_blanks;
			debug_print_string(value->string, value->length);
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
			value->length = 5;
			value->string = json_value + left_blanks;
			debug_print_string(value->string, value->length);
		}
		break;
		case '[':
		{
		}
		break;
		case '{':
		{
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
					json_value[i] != '\r' && json_value[i] != '\n' && json_value[i] != ',' && json_value[i] != '}')
				{
					puts("panic: invalid number");
					exit(1);
				}
				value->length = i - left_blanks;
				value->string = json_value + left_blanks;
				debug_print_string(value->string, value->length);
			}
			else
			{
				puts("panic: invalid value");
				exit(1);
			}
		}
	}
	return i;
}

//NOTE(simone): not necessarily needed the first i = 0 part to be normalized.
internal void normalize_value(char *json_value, int count, normalized_value *norm_value)
{
	int i = 0;
	while(json_value[i] == ' ' || json_value[i] == '\t' ||
			json_value[i] == '\r' || json_value[i] == '\n')
	{
		++i;
	}
	int x = count - 1;
	while(json_value[x] == ' ' || json_value[x] == '\t' ||
			json_value[x] == '\r' || json_value[x] == '\n')
	{
		--x;
	}

	if(x - i < 0)
	{
		puts("panic in fn normalize_value()");
		exit(1);
	}
	norm_value->length = x - i;
	norm_value->string = json_value + i;

	int len = 0;
	//NOTE(simone): remove these debug steps;
	printf("debug norm_value->length: %d\n", norm_value->length);
	puts("debugging in normalize_value\n");
	while(len <= norm_value->length)
	{
		putchar(norm_value->string[len]);
		++len;
	}
	puts("\n");
	return;
}

internal int8 string_checker(int count, char *json_value)
{
	char json_val0 = json_value[0];
	char json_val_end = json_value[count];
	char json_val_test = json_value[count-1];
	if(json_value[0] == '\"' && json_value[count-1] == '\"')
	{
		return true;
	}
	else
	{
		return false;
	}
}

internal int8 num_checker(int count, char *json_value)
{
	char debug_num_checker = '\0';
	if(json_value[0] >= 48 && json_value[0] <= 57)
	{
		while(count > 0 && json_value[count] >= 48 && json_value[count] <= 57)
		{
			--count;
		}
		if(count == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

/* NOTE(simone): bool_value wants to be 0 or 1. On that case we will assign to count to 5 or 4
 * 5 for 'false' and 4 for 'true' and a constant true/false string will be initialized */
internal int8 bool_checker(int8 bool_value, char *json_value)
{
	//TODO(simone): rewrite this as an handmade assert
	if (bool_value < 0 || bool_value > 1)
	{
		exit(1);
	}

	int8 len = bool_value == 0 ? 5 : 4;
	const char *compare_value = bool_value == 0 ? "false" : "true";
	for(int i=0; i<len; ++i)
	{
		if (json_value[i] != compare_value[i])
		{
			return false;
		}
	}
	return true;
}

internal int8 array_checker(uint64 count, char *json_value)
{
	if(json_value[0] != '[' || json_value[count] != ']')
	{
		puts("invalid array format");
		return false;
	}
	for(int i=1; i<count; ++i)
	{
		 if(json_value[i] == ',')
		 {
			 eval_value_type(json_value + i, i);
		 }
	}
	return true;

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
	if(file_data[0] != '{')
	{
		printf("invalid json beginnig check your syntax");
		exit(1);
	}
	//NOTE(simone): it is -3 because windows is dogshit and adds other chars implicitly
	if(file_data[file_size_in_bytes-3] != '}')
	{
		printf("invalid end of json %d", file_data[file_size_in_bytes-3]);
		exit(1);
	}
	//NOTE(simone): verify if this int is suitable
	int keys_count = 0;
	int values_count = 0;
	char last_char = '\0';
	char debug_current_char = '\0';

	normalized_value string_to_print = {};
	normalized_key key_to_print = {};

	// TODO(simone): switch to a while loop;
	for(SIZE_T i = 1; i < file_size_in_bytes-3; ++i)
	{
		debug_current_char = file_data[i];
		if(file_data[i] == '\n' || file_data[i] == ' ' || file_data[i] == '\r' || file_data[i] == '\t')
		{
			continue;
		}
		i += (parse_and_extract_key(file_data + i, &key_to_print) + 1); //+1 indicates to start further after parsing ':'
		debug_current_char = file_data[i];
		i += (parse_and_extract_value(file_data + i, &string_to_print)); 
		debug_current_char = file_data[i];
		last_char = file_data[i];

	}
	printf("total scanned keys: %d total values scanned %d\n", keys_count, values_count);

}

