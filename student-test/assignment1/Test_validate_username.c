#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../../examples/autotest-validate/autotest-validate.h"
#include "../../assignment-autotest/test/assignment1/username-from-conf-file.h"

/**
* This function should:
*   1) Call the my_username() function in Test_assignment_validate.c to get your hard coded username.
*   2) Obtain the value returned from function malloc_username_from_conf_file() in username-from-conf-file.h within
*       the assignment autotest submodule at assignment-autotest/test/assignment1/
*   3) Use unity assertion TEST_ASSERT_EQUAL_STRING_MESSAGE the two strings are equal.  See
*       the [unity assertion reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md)
*/
void test_validate_my_username()
{
    // 1) autotest-validate.c에 있는 하드코딩된 내 유저네임 가져오기
    const char *hardcoded_username = my_username();

    // 2) conf/username.txt 파일에서 읽어온 유저네임 가져오기 (동적 할당됨)
    char *conf_username = malloc_username_from_conf_file();

    // 3) 두 문자열이 완벽히 일치하는지 확인하기
    TEST_ASSERT_EQUAL_STRING_MESSAGE(hardcoded_username, conf_username, "The usernames do not match! Please check conf/username.txt and my_username() in autotest-validate.c");

    // 할당된 메모리 해제
    free(conf_username);
}
