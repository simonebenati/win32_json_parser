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
	const char *string;
} normalized_elem;

typedef normalized_elem normalized_key;
typedef normalized_elem normalized_value;

void eval_value_type(char *value_end, int count);
internal int parse_and_extract_key(const char *json_value, normalized_key *key);
internal int parse_and_extract_value(const char *json_value, normalized_value *value);
