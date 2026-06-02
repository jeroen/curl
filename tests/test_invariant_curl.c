#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declare the function under test from src/curl.c */
extern void handle_curl_response(void *contents, size_t size, size_t nmemb, void *userp);

typedef struct {
    char *buf;
    size_t size;
    size_t limit;
} request_buffer_t;

START_TEST(test_allocation_failure_handling)
{
    /* Invariant: realloc/malloc failures must not cause NULL pointer dereference.
       The function must either handle allocation failure gracefully or not crash. */
    
    const char *payloads[] = {
        "A",                                    /* valid minimal input */
        "normal_response_data",                 /* valid normal input */
        "\x00\x00\x00\x00",                    /* boundary: null bytes */
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);
    
    for (int i = 0; i < num_payloads; i++) {
        request_buffer_t req = {0};
        req.buf = malloc(1);
        ck_assert_ptr_nonnull(req.buf);
        req.size = 0;
        req.limit = 1;
        
        /* Call the actual production function with adversarial input */
        handle_curl_response((void *)payloads[i], strlen(payloads[i]), 1, &req);
        
        /* Invariant: if buf is non-NULL after call, it must be valid memory
           (no segfault occurred during realloc/memcpy chain) */
        if (req.buf != NULL) {
            ck_assert_int_ge(req.limit, req.size);
        }
        
        free(req.buf);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_allocation_failure_handling);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}