#!/bin/bash
for i in {2..37}; do taskset -c $i ./memoptest.x 4 &  done
for i in {38..39}; do taskset -c $i ./memoptest.x 3 &  done


