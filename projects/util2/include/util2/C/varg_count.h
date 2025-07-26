#ifndef __UTIL2_COUNT_VA_ARGS__
#define __UTIL2_COUNT_VA_ARGS__
#include "static_assert.h"
/*
    Thanks to:
    https://stackoverflow.com/questions/66556552/a-way-to-count-the-number-of-va-args-arguments-including-0-without-compile
*/


#define __UTIL2_INTERNAL_EXPAND(x) x
#define __UTIL2_GLUE_INTERNAL(X,Y) X##Y
#define __UTIL2_GLUE(X,Y) __UTIL2_GLUE_INTERNAL(X,Y)


/* Returns the 100th argument. */
#define __UTIL2_INTERNAL_ARG_100(_,\
    _100,_99,_98,_97,_96,_95,_94,_93,_92,_91,_90,_89,_88,_87,_86,_85,_84,_83,_82,_81, \
    _80,_79,_78,_77,_76,_75,_74,_73,_72,_71,_70,_69,_68,_67,_66,_65,_64,_63,_62,_61, \
    _60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41, \
    _40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21, \
    _20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,X_,...) X_


/* Returns whether __VA_ARGS__ has a comma (up to 100 arguments). */
#define __UTIL2_INTERNAL_HAS_COMMA(...) __UTIL2_INTERNAL_EXPAND(__UTIL2_INTERNAL_ARG_100(__VA_ARGS__, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, \
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0))


/* Produces a comma if followed by a parenthesis. */
#define __UTIL2_INTERNAL_TRIGGER_PARENTHESIS(...) ,
#define __UTIL2_INTERNAL_PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define __UTIL2_INTERNAL_IS_EMPTY_CASE_0001 ,


/* Returns true if inputs expand to (false, false, false, true) */
#define __UTIL2_INTERNAL_IS_EMPTY(_0, _1, _2, _3) __UTIL2_INTERNAL_HAS_COMMA(__UTIL2_INTERNAL_PASTE5(__UTIL2_INTERNAL_IS_EMPTY_CASE_, _0, _1, _2, _3))


/* Returns whether __VA_ARGS__ is empty. */
#define __UTIL2_IS_EMPTY(...) __UTIL2_INTERNAL_IS_EMPTY(\
    /* \
        Testing for an argument with a comma \
        e.g. "ARG1, ARG2", "ARG1, ...", or "," \
    */ \
    __UTIL2_INTERNAL_HAS_COMMA(__VA_ARGS__), \
    /* \
        Testing for an argument around parenthesis \
        e.g. "(ARG1)", "(...)", or "()" \
    */ \
    __UTIL2_INTERNAL_HAS_COMMA(__UTIL2_INTERNAL_TRIGGER_PARENTHESIS __VA_ARGS__), \
    /* \
        Testing for a macro as an argument, \
        which will expand the parenthesis, \
        possibly generating a comma. \
    */ \
    __UTIL2_INTERNAL_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
    /* \
        If all previous checks are false, __VA_ARGS__ does not     \
        generate a comma by itself, nor with __UTIL2_INTERNAL_TRIGGER_PARENTHESIS \
        behind it, nor with () after it.                           \
        Therefore, "__UTIL2_INTERNAL_TRIGGER_PARENTHESIS __VA_ARGS__ ()"          \
        only generates a comma if __VA_ARGS__ is empty.            \
        So, this tests for an empty __VA_ARGS__                    \
        (given the previous conditionals are false). \
    */ \
    __UTIL2_INTERNAL_HAS_COMMA(__UTIL2_INTERNAL_TRIGGER_PARENTHESIS __VA_ARGS__ (/*empty*/)) \
)


#define __UTIL2_INTERNAL_VAR_COUNT_EMPTY_1(...) 0
#define __UTIL2_INTERNAL_VAR_COUNT_EMPTY_0(...) __UTIL2_INTERNAL_EXPAND(__UTIL2_INTERNAL_ARG_100(__VA_ARGS__, \
    100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81, \
    80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61, \
    60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41, \
    40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21, \
    20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1))
#define UTIL2_ARG_COUNT(...) __UTIL2_GLUE(__UTIL2_INTERNAL_VAR_COUNT_EMPTY_, __UTIL2_IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)


#define __UTIL2_INTERNAL_TEST_EATER0(...)
#define __UTIL2_INTERNAL_TEST_EATER1(...) ,
#define __UTIL2_INTERNAL_TEST_EATER2(...) (/*empty*/)
#define __UTIL2_INTERNAL_TEST_EATER3(...) (/*empty*/),
#define __UTIL2_INTERNAL_TEST_EATER4(...) EATER1
#define __UTIL2_INTERNAL_TEST_EATER5(...) EATER2
#define __UTIL2_INTERNAL_TEST_MAC0() ()
#define __UTIL2_INTERNAL_TEST_MAC1(x) ()
#define __UTIL2_INTERNAL_TEST_MACV(...) ()
#define __UTIL2_INTERNAL_TEST_MAC2(x,y) whatever

UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT()                             == 0, UTIL2_STATIC_ASSERT_VAR_COUNT_EMPTY,                 "UTIL2_ARG_COUNT(...) == 0 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(/*comment*/)                  == 0, UTIL2_STATIC_ASSERT_VAR_COUNT_COMMENT,               "UTIL2_ARG_COUNT(...) == 0 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(a)                            == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_A,                     "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(a, b)                         == 2, UTIL2_STATIC_ASSERT_VAR_COUNT_AB,                    "UTIL2_ARG_COUNT(...) == 2 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(a, b, c)                      == 3, UTIL2_STATIC_ASSERT_VAR_COUNT_ABC,                   "UTIL2_ARG_COUNT(...) == 3 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(a, b, c, d)                   == 4, UTIL2_STATIC_ASSERT_VAR_COUNT_ABCD,                  "UTIL2_ARG_COUNT(...) == 4 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(a, b, c, d, e)                == 5, UTIL2_STATIC_ASSERT_VAR_COUNT_ABCDE,                 "UTIL2_ARG_COUNT(...) == 5 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT((a, b, c, d, e))              == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_ARGS_IN_PARENTHESIS,   "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT((void))                       == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_VOID_PARENTHESIS,      "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT((void), c, d)                 == 3, UTIL2_STATIC_ASSERT_VAR_COUNT_VOID_PARENTHESIS_CD,   "UTIL2_ARG_COUNT(...) == 3 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT((a, b), c, d)                 == 3, UTIL2_STATIC_ASSERT_VAR_COUNT_PARENTHESIS_AB_CD,     "UTIL2_ARG_COUNT(...) == 3 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(_TRIGGER_PARENTHESIS_)        == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_PARENTHESIS_TRIGGER,   "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_EATER0) == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_EATER0,                "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_EATER1) == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_EATER1,                "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_EATER2) == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_EATER2,                "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_EATER3) == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_EATER3,                "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_EATER4) == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_EATER4,                "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_MAC0)   == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_MAC0,                  "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_MAC1)   == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_MAC1,                  "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(__UTIL2_INTERNAL_TEST_MACV)   == 1, UTIL2_STATIC_ASSERT_VAR_COUNT_MACV,                  "UTIL2_ARG_COUNT(...) == 1 Failed ")
UTIL2_STATIC_ASSERT(UTIL2_ARG_COUNT(
        __UTIL2_INTERNAL_TEST_MAC2, \
        __UTIL2_INTERNAL_TEST_MAC1, \
        __UTIL2_INTERNAL_TEST_MACV \
        ) == 3, \
    STATIC_ASSERT_VAR_COUNT_MAC2_MAC1_MACV, \
    "UTIL2_ARG_COUNT(...) == 3 Failed " \
)


#endif /* __UTIL2_COUNT_VA_ARGS__ */