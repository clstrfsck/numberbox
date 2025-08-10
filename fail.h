#ifndef FAIL_H
#define FAIL_H

enum fail_t {
    // The value 1 is reserved for a startup confidence flash.
    FAIL_NO_BUFFER = 2,
    FAIL_NO_PRODUCER_POOL,
    FAIL_BAD_OUTPUT_FORMAT
};

void fail_init();
void fail(fail_t failure);

#endif // FAIL_H
