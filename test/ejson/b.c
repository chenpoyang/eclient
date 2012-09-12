#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "json.h"

json_t *new_entry(char *name, char *phone)
{
    json_t *entry, *label, *value;

    // create an entry node
    entry = json_new_object();

    // insert the first label-value pair
    label = json_new_string("name");
    value = json_new_string("Andew");
    json_insert_child(label, value);
    json_insert_child(entry, label);

    // insert the second label-value pair
    label = json_new_string("phone");
    value = json_new_string("555 123 456");
    json_insert_child(label, value);
    json_insert_child(entry, label);

    // inserts that object as a value in a label-value pair
    label = json_new_string("entry");
    json_insert_child(label, entry);

    return label;
}

int main (void)
{
    setlocale (LC_ALL, "");//设置为系统默认的地域信息

    json_t *root, *subtree;

    // creates the root node
    root = json_new_object();

    // creates the desired MJSON document subtree
    subtree = new_entry("Andrew", "555 123 456");

    // inserts the subtree into the root object
    json_insert_child(root, subtree);

    // print the result
    char *text;
    json_tree_to_string(root, &text);
    printf("%s\n",text);    //官方例子中为printf("%ls\n",text);去掉l才能打印出来。。

    // clean up
    free(text);
    json_free_value(&root);
    return EXIT_SUCCESS;
}
/*
【输出结果】
{"entry":{"name":"Andew","phone":"555 123 456"}}
*/
