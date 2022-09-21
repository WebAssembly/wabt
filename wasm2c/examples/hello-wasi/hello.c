#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {

    printf("printing args\n");
    for (int i = 0; i < argc ; i++) {
	printf("[%d] %s \n", i, argv[i]);
    }

    printf("Writing to stdout\n");
    char * hello_str = strdup("hello world!");
    puts(hello_str);

    printf("writing test.out\n");
    FILE * fp = fopen("test.out", "w+");
    assert(fp != NULL);
    fprintf(fp,"hello filesystem\n");
    fclose(fp);
    printf("removing test.out\n");
    assert(!unlink("test.out"));

    printf("writing /tmp/test.out\n");
    fp = fopen("/tmp/test.out", "w+");
    assert(fp != NULL);
    fprintf(fp,"hello filesystem\n");
    fclose(fp);
    printf("removing /tmp/test.out\n");
    assert(!unlink("/tmp/test.out"));

    return 0;
}
