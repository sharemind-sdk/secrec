#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

################################################################################
# Tests:
################################################################################

FUNCTION(add_test_secrec_execute testfile)
    ADD_TEST(NAME "${testfile}"
        COMMAND $<TARGET_FILE:sca> --eval "${CMAKE_CURRENT_SOURCE_DIR}/${testfile}.sc")
ENDFUNCTION()

# Tests for scalars:
add_test_secrec_execute("scalars/00-trivial")
add_test_secrec_execute("scalars/01-true")
add_test_secrec_execute("scalars/02-decl")
add_test_secrec_execute("scalars/03-scope")
add_test_secrec_execute("scalars/04-function")
add_test_secrec_execute("scalars/05-assert-fail")
add_test_secrec_execute("scalars/05-declassify")
add_test_secrec_execute("scalars/06-rec")
add_test_secrec_execute("scalars/07-while")
add_test_secrec_execute("scalars/08-func-ret")
add_test_secrec_execute("scalars/09-global")
add_test_secrec_execute("scalars/10-if")
add_test_secrec_execute("scalars/11-empty-if")
add_test_secrec_execute("scalars/12-return")
add_test_secrec_execute("scalars/13-loop-bench")
add_test_secrec_execute("scalars/14-expressions")
add_test_secrec_execute("scalars/15-continue")
add_test_secrec_execute("scalars/16-break")
add_test_secrec_execute("scalars/17-for")
add_test_secrec_execute("scalars/18-for-empty-body")
add_test_secrec_execute("scalars/19-logic-public")
add_test_secrec_execute("scalars/20-logic-bug1")
add_test_secrec_execute("scalars/21-logic-private")
add_test_secrec_execute("scalars/22-ternary")
add_test_secrec_execute("scalars/23-uops-public")
add_test_secrec_execute("scalars/24-uops-private")
add_test_secrec_execute("scalars/25-assignops")
add_test_secrec_execute("scalars/26-do-while")
add_test_secrec_execute("scalars/27-do-while-bug")
add_test_secrec_execute("scalars/28-scope-bug")
add_test_secrec_execute("scalars/29-scope-bug-2")
add_test_secrec_execute("scalars/30-rec-bug")
add_test_secrec_execute("scalars/31-fib")
add_test_secrec_execute("scalars/32-overloading")
add_test_secrec_execute("scalars/33-string")
add_test_secrec_execute("scalars/34-print")
add_test_secrec_execute("scalars/35-prefix-incdec")
add_test_secrec_execute("scalars/36-postfix-incdec")
add_test_secrec_execute("scalars/37-for-decl")
add_test_secrec_execute("scalars/38-postfix-fail")
add_test_secrec_execute("scalars/39-int-types")
add_test_secrec_execute("scalars/40-casts")
add_test_secrec_execute("scalars/41-param-order")
add_test_secrec_execute("scalars/42-ra-fail")
add_test_secrec_execute("scalars/43-domain-fail")
add_test_secrec_execute("scalars/44-classify")
add_test_secrec_execute("scalars/45-return-type-overloading")
add_test_secrec_execute("scalars/46-domain-id")
add_test_secrec_execute("scalars/47-operator-overloading")
add_test_secrec_execute("scalars/48-multiple-var-decl")
add_test_secrec_execute("scalars/49-multiple-var-decl-bug")
add_test_secrec_execute("scalars/50-integer-types")
add_test_secrec_execute("scalars/51-qualified-expressions")
add_test_secrec_execute("scalars/52-empty-false-branch")
add_test_secrec_execute("scalars/53-for-loop-bug")
add_test_secrec_execute("scalars/54-Bug1478")
add_test_secrec_execute("scalars/55-invalid-concat")
add_test_secrec_execute("scalars/56-overlapping-instances")
add_test_secrec_execute("scalars/57-strlen")
add_test_secrec_execute("scalars/58-float-cast-bug")
add_test_secrec_execute("scalars/59-float-cast-bug-2")
add_test_secrec_execute("scalars/60-float-cast-bug-3")
add_test_secrec_execute("scalars/61-float-cast-bug-4")
add_test_secrec_execute("scalars/62-float-cast-bug-5")
add_test_secrec_execute("scalars/63-operator-overloading-2")
add_test_secrec_execute("scalars/64-operator-overloading-3")
add_test_secrec_execute("scalars/65-operator-overloading-4")
add_test_secrec_execute("scalars/66-operator-overloading-5")
add_test_secrec_execute("scalars/67-operator-overloading-6")
add_test_secrec_execute("scalars/68-operator-overloading-7")
add_test_secrec_execute("scalars/69-operator-overloading-8")
add_test_secrec_execute("scalars/70-operator-overloading-9")
add_test_secrec_execute("scalars/71-user-defined-types-1")
add_test_secrec_execute("scalars/72-user-defined-types-2")
add_test_secrec_execute("scalars/73-cast-definitions-1")
add_test_secrec_execute("scalars/74-cast-definitions-2")
add_test_secrec_execute("scalars/75-cast-definitions-3")
add_test_secrec_execute("scalars/76-cast-definitions-4")
add_test_secrec_execute("scalars/77-cast-definitions-5")
add_test_secrec_execute("scalars/78-void-cast-bug")

SET_TESTS_PROPERTIES("scalars/05-assert-fail" PROPERTIES PASS_REGULAR_EXPRESSION "assert failed at .*\\(3,3\\)\\(3,18\\)")
SET_TESTS_PROPERTIES("scalars/43-domain-fail" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(11,5\\)\\(11,12\\)")
SET_TESTS_PROPERTIES("scalars/53-for-loop-bug" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(3,32\\)\\(3,37\\)")
SET_TESTS_PROPERTIES("scalars/55-invalid-concat" PROPERTIES
  PASS_REGULAR_EXPRESSION "[FATAL].*\\(4,26\\)\\(4,27\\)")
SET_TESTS_PROPERTIES("scalars/64-operator-overloading-3" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(1,1\\)\\(2,42\\)")
SET_TESTS_PROPERTIES("scalars/65-operator-overloading-4" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(1,1\\)\\(2,47\\)")
SET_TESTS_PROPERTIES("scalars/66-operator-overloading-5" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(1,1\\)\\(2,42\\)")
SET_TESTS_PROPERTIES("scalars/67-operator-overloading-6" PROPERTIES PASS_REGULAR_EXPRESSION "Mismatching shapes in arithmetic expression at.*\\(10,5\\)\\(10,10\\)")
SET_TESTS_PROPERTIES("scalars/68-operator-overloading-7" PROPERTIES PASS_REGULAR_EXPRESSION "returned a value with incorrect size.*\\(9,5\\)\\(9,10\\)")
SET_TESTS_PROPERTIES("scalars/69-operator-overloading-8" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(9,5\\)\\(9,10\\)")
SET_TESTS_PROPERTIES("scalars/71-user-defined-types-1" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(9,19\\)\\(9,20\\)")
SET_TESTS_PROPERTIES("scalars/74-cast-definitions-2" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(15,19\\)\\(15,29\\)")
SET_TESTS_PROPERTIES("scalars/75-cast-definitions-3" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(8,1\\)\\(11,2\\)")
SET_TESTS_PROPERTIES("scalars/76-cast-definitions-4" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(10,24\\)\\(10,34\\)")
SET_TESTS_PROPERTIES("scalars/77-cast-definitions-5" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(7,4\\)\\(7,10\\)")
SET_TESTS_PROPERTIES("scalars/78-void-cast-bug" PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(3,5\\)\\(3,15\\)")

# Tests for arrays:
add_test_secrec_execute("arrays/00-trivia")
add_test_secrec_execute("arrays/01-size")
add_test_secrec_execute("arrays/02-expression")
add_test_secrec_execute("arrays/03-index")
add_test_secrec_execute("arrays/04-slice")
add_test_secrec_execute("arrays/05-simplicit")
add_test_secrec_execute("arrays/06-functions")
add_test_secrec_execute("arrays/07-dynamic-size")
add_test_secrec_execute("arrays/08-rec")
add_test_secrec_execute("arrays/09-reshape")
add_test_secrec_execute("arrays/10-shape")
add_test_secrec_execute("arrays/11-shape-change")
add_test_secrec_execute("arrays/12-big-expr")
add_test_secrec_execute("arrays/13-index-assing")
add_test_secrec_execute("arrays/14-index-assing-slice")
add_test_secrec_execute("arrays/15-index-assing-slice-2d")
add_test_secrec_execute("arrays/16-scalar-reshape")
add_test_secrec_execute("arrays/17-assign-op-index")
add_test_secrec_execute("arrays/18-nat")
add_test_secrec_execute("arrays/19-fibo")
add_test_secrec_execute("arrays/20-order")
add_test_secrec_execute("arrays/21-reshape-bug")
add_test_secrec_execute("arrays/22-ternary-bug")
add_test_secrec_execute("arrays/23-assign-bug")
add_test_secrec_execute("arrays/24-ternary")
add_test_secrec_execute("arrays/25-uops")
add_test_secrec_execute("arrays/26-classify")
add_test_secrec_execute("arrays/27-assign-op")
add_test_secrec_execute("arrays/28-primes")
add_test_secrec_execute("arrays/29-slice-suggar")
add_test_secrec_execute("arrays/30-concat")
add_test_secrec_execute("arrays/31-concat-2d")
add_test_secrec_execute("arrays/32-slice-order")
add_test_secrec_execute("arrays/33-concat-suggar")
add_test_secrec_execute("arrays/34-reshape-classify")
add_test_secrec_execute("arrays/35-cat-bug")
add_test_secrec_execute("arrays/36-slice-empty-dim")
add_test_secrec_execute("arrays/37-cat-empty")
add_test_secrec_execute("arrays/38-concat-empty-matrix-bug")
add_test_secrec_execute("arrays/39-size-of-assignment-expr")
add_test_secrec_execute("arrays/40-prefix-postfix-incdec")
add_test_secrec_execute("arrays/41-for-decl")
add_test_secrec_execute("arrays/42-inc-bug")
add_test_secrec_execute("arrays/43-arith-assignment-bug")
add_test_secrec_execute("arrays/44-casts")
add_test_secrec_execute("arrays/45-inc-bug-2")
add_test_secrec_execute("arrays/46-assignment-bug")
add_test_secrec_execute("arrays/47-out-of-bounds-bug")
add_test_secrec_execute("arrays/48-private-index-bug")
add_test_secrec_execute("arrays/49-private-concat-bug")
add_test_secrec_execute("arrays/50-slice-assign-bug")
add_test_secrec_execute("arrays/51-seq-assign-bug")
add_test_secrec_execute("arrays/52-inc-bug-3")
add_test_secrec_execute("arrays/53-if-expression-bug")
add_test_secrec_execute("arrays/54-arith-assignment-bug")
add_test_secrec_execute("arrays/55-global-array-size-bug")

# Tests for templates:
add_test_secrec_execute("templates/00-trivial")
add_test_secrec_execute("templates/01-recursion")
add_test_secrec_execute("templates/02-kind-fail")
SET_TESTS_PROPERTIES("templates/02-kind-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(11,5\\)\\(11,19\\).")
add_test_secrec_execute("templates/03-kind")
add_test_secrec_execute("templates/04-multiparam")
add_test_secrec_execute("templates/05-multiparam-fail")
SET_TESTS_PROPERTIES("templates/05-multiparam-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(15,5\\)\\(15,15\\).")
add_test_secrec_execute("templates/06-instantiation-fail")
SET_TESTS_PROPERTIES("templates/06-instantiation-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(4,9\\)\\(4,10\\)")
add_test_secrec_execute("templates/07-strange-bug")
add_test_secrec_execute("templates/08-mutual-recursion")
add_test_secrec_execute("templates/09-recursive-call-of-different-instance")
add_test_secrec_execute("templates/10-overloading")
add_test_secrec_execute("templates/11-name-lookup")
add_test_secrec_execute("templates/12-return-type-overloading")
add_test_secrec_execute("templates/13-various")
add_test_secrec_execute("templates/14-ambiguous-lookup")
SET_TESTS_PROPERTIES("templates/14-ambiguous-lookup"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(16,5\\)\\(16,15\\)")
add_test_secrec_execute("templates/15-unify-fail-bug")
SET_TESTS_PROPERTIES("templates/15-unify-fail-bug"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(1,18\\)\\(1,19\\)")
add_test_secrec_execute("templates/16-is-scalar")
add_test_secrec_execute("templates/17-simple")
add_test_secrec_execute("templates/18-overloading-on-struct-bug")
add_test_secrec_execute("templates/19-operator-overloading-1")
add_test_secrec_execute("templates/20-operator-overloading-2")
SET_TESTS_PROPERTIES("templates/20-operator-overloading-2"
  PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(8,21\\)\\(8,30\\)")
add_test_secrec_execute("templates/21-cast-definitions")
add_test_secrec_execute("templates/22-protection-domain-bug")
add_test_secrec_execute("templates/23-template-instance-bug")
add_test_secrec_execute("templates/24-index-into-N-dimensional-array")
SET_TESTS_PROPERTIES("templates/22-protection-domain-bug"
  PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(7,10\\)\\(7,21\\)")
SET_TESTS_PROPERTIES("templates/23-template-instance-bug"
  PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(13,7\\)\\(13,10\\)")
SET_TESTS_PROPERTIES("templates/24-index-into-N-dimensional-array"
  PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(10,.*\\)\\(10,.*\\)")

# Tests for structures:
add_test_secrec_execute("structs/00-simple-declaration")
add_test_secrec_execute("structs/01-simple-definition")
add_test_secrec_execute("structs/02-set-get")
add_test_secrec_execute("structs/03-template")
add_test_secrec_execute("structs/04-redeclaration")
add_test_secrec_execute("structs/05-trivial-get")
add_test_secrec_execute("structs/06-return-struct")
add_test_secrec_execute("structs/07-param-struct")
add_test_secrec_execute("structs/08-double-free-bug")
add_test_secrec_execute("structs/09-inc-dec")
add_test_secrec_execute("structs/10-nested-template")
add_test_secrec_execute("structs/11-template-domain-failure")
add_test_secrec_execute("structs/12-template-argument")
add_test_secrec_execute("structs/13-some")
add_test_secrec_execute("structs/14-assign-bug")
add_test_secrec_execute("structs/15-recursive-bug")

SET_TESTS_PROPERTIES("structs/04-redeclaration"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(4,1\\)\\(4,15\\)")

SET_TESTS_PROPERTIES("structs/11-template-domain-failure"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(9,.*\\)\\(9,.*\\)")

# Regressions found by AFL (american fuzzy lop).
add_test_secrec_execute("afl/00-integer-literal-overflow")
SET_TESTS_PROPERTIES("afl/00-integer-literal-overflow"
    PROPERTIES PASS_REGULAR_EXPRESSION ".*\\(3,16\\)-\\(3,43\\).*")

add_test_secrec_execute("afl/01-index-scalar-bug")
SET_TESTS_PROPERTIES("afl/01-index-scalar-bug"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(4,.*\\)\\(4,.*\\)")

add_test_secrec_execute("afl/02-global-struct-init-bug")

add_test_secrec_execute("afl/03-compare-struct-and-int-bug")
SET_TESTS_PROPERTIES("afl/03-compare-struct-and-int-bug"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(6,.*\\)\\(6,.*\\)")
