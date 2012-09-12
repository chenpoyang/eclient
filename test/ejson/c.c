#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "json.h"

int main (void)
{
    setlocale (LC_ALL, "");
    char *document = 
        "{\"entry\":{\"name\":\"Andew\",\"phone\":\"555 123 456\"}}";

    json_t *root;

    printf("Parsing the document…\n");
    json_parse_document(&root, document);

    printf("Printing the document tree…\n");
    json_tree_to_string(root, &document);
    printf("%s\n", document);

    // clean up
    json_free_value(&root);
    return EXIT_SUCCESS;
}
