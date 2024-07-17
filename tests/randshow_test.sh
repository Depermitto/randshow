#! /bin/bash

cd ..
build/gen_binary_data | tests/PractRand-pre0.95/RNG_test stdin64 -tlmax 1024MB | grep -q "FAIL"
if [ $? -eq 0 ]; then
    echo 1
fi

exit 0
