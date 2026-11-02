EXEC_NAME = json_parse

all: build

build:
	clang -O0 -g nix_handmade_json_parser.c -o $(EXEC_NAME)

debug:
	codium --open-url "vscodium://vadimcn.vscode-lldb/launch/command?$(CURDIR)/$(EXEC_NAME)"
