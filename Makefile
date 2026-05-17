CC = gcc
WFlags = -Wall

TARGETS = main

# Define the path between the original file and the corresponding target object file
OBJDIR = obj
SRCS = main.c lex.c parser.c codeGen.c
OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGETS)

# Use the object file under OBJDIR when linking
main: $(OBJ)
	$(CC) $^ -o $@

# Compilation rules: Add directives to create folders
# $@ represents the target filename, $< represents the first source filename
# Use Order-only prerequisite (|) to ensure the folder exists
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(WFlags) -c $^ -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(OBJ)
	rm -rf $(OBJDIR)
