module test_report;

import stdlib;

void test_report(uint tests, uint passed) {
    publish("test_count", tests);
    publish("passed_count", passed);
}

void test_report(int64 tests, int64 passed) {
    // SecreCTestRunner expects uint64s
    test_report((uint) tests, (uint) passed);
}

void test_report(uint32 tests, uint32 passed) {
    // SecreCTestRunner expects uint64s
    test_report((uint) tests, (uint) passed);
}

void test_report_error(float64 error) {
    publish("relative_error", error);
}

void test_report_error(float32 error) {
    // SecreCTestRunner expects a float64
    test_report_error((float64) error);
}
