#!/usr/bin/env bash
find include/ src/ test/ examples/ \
        -regextype egrep \
        -type f -regex '.*\.(hpp|cpp|hpp\.in)$' \
    | xargs clang-format-10 -i