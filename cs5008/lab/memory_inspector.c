#include <stdio.h>

void inspect_memory(char *address, long byte_count)
{
    const int BYTES_PER_LINE = 16;  // ADDED

	for (long i = 0; i < byte_count; i += BYTES_PER_LINE)
	{
        // 1) line address
		printf("%p  ", (void*)(address + i));

        // 2) hex bytes
        for (int j = 0; j < BYTES_PER_LINE; j++)
        {
            long idx = i + j;
            if (idx < byte_count)
            {
                unsigned char b = (unsigned char)address[idx];
                printf("%02x ", b);
            }
            else
            {
                printf("   ");
            }
        }

        // 3) ASCII
        printf(" ");
        for (int j = 0; j < BYTES_PER_LINE; j++)
        {
            long idx = i + j;
            if (idx < byte_count)
            {
                unsigned char b = (unsigned char)address[idx];
                if (b >= 32 && b <= 126) printf("%c", b);
                else printf(".");
            }
            else printf(" ");
        }
    
        // 4) new line
        printf("\n");
	}    
}

typedef struct {
    int id;
    char grade;
    short year;
} Student;

int main()
{
    // 1) struct
    Student s = {123456, 'A', 25};

    // 2) array
    int arr[6] = {1, 2, 3, 1000, -1, 42};

    // 3) string
    char msg[] = "Hello, Memory Inspector!";

    printf("Inspect struct Student s (size=%zu)\n", sizeof(Student));
    inspect_memory((char *)&s, (long)sizeof(Student));
    printf("\n");

    printf("Inspect int array arr (size=%zu)\n", sizeof(arr));
    inspect_memory((char *)arr, (long)sizeof(arr));
    printf("\n");

    printf("Inspect string msg (size=%zu)\n", sizeof(msg));
    inspect_memory((char *)msg, (long)sizeof(msg));
    printf("\n");

    return 0;
}
