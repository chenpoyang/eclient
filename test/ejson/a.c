#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "json.h"

int main (void)
{
    char *text;
    json_t *root, *entry, *label, *value;
    setlocale (LC_ALL, "");//设为系统默认地域信息

    // creates the root node
    root = json_new_object();
    // create an entry node
    entry = json_new_object();

    //    第一部分，打印结果：
    //    {"entry":{"name":"Andew","phone":"555 123 456"}}

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

    // inserts that label-value pair into the root object
    json_insert_child(root, label);

    // print the result
    json_tree_to_string(root, &text);
    printf("%s\n",text);

    // clean up
    free(text);
    json_free_value(&root);

    //打印第二部分，数组示例，
    //结果：
    //    ["test1","test2",109]

    root = json_new_array();
    label = json_new_string("test1");
    json_insert_child(root,label);
    value = json_new_string("test2");
    json_insert_child(root,value);
    value = json_new_number("109");
    json_insert_child(root,value);

    json_tree_to_string(root,&text);
    printf("%s\n",text);

    // clean up
    free(text);
    json_free_value(&root);

    return EXIT_SUCCESS;
}
