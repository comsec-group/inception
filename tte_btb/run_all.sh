#!/bin/bash

make clean all

echo [-] TTE_BTB-BTB
./ijmp_ijmp
echo [-] TTE_PHT-BTB
./jcc_ijmp
echo [-] TTE_RSB-BTB
./ret_ijmp
echo [-] TTE_OOO-BTB
./ooo_ijmp
