#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (* TestFunc)();

struct TestItem
{
    const char * name;
    TestFunc func;
};

int emptyTest();

#define TEST_ITEM(name) { #name, &name##Test }

TestItem tests[] = {
    TEST_ITEM(empty),
    { nullptr, nullptr }
};

struct test_item * current_test;

static void printTests()
{
    TestItem * current = tests;
    for (; current->name != NULL; ++current) {
        printf("%s\n", current->name);
    }
}

static void runTestItem(const TestItem * item)
{
    printf("Run test “%s”.\n", item->name);
    int code = (*item->func)();
    if (code) {
        exit(code);
    }
}

static void runTest(const char * name)
{
    for (TestItem * currentTest = tests; currentTest->name != nullptr; ++currentTest) {
        if (strcmp(name, currentTest->name) == 0) {
            return runTestItem(currentTest);
        }
    }

    fprintf(stderr, "Test “%s” is not found.\n", name);
    exit(1);
}

int emptyTest()
{
    return 0;
}



int main(int argc, char * argv[])
{
    if (argc == 1) {
        printTests();
        return 0;
    }

    for (int i=1; i < argc; ++i) {
        runTest(argv[i]);
    }

    return 0;
}
