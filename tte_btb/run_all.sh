#!/bin/bash

make clean all

echo [-] TTE_BTB-BTB
./out/ijmp_ijmp
echo [-] TTE_PHT-BTB
./out/jcc_ijmp
echo [-] TTE_RSB-BTB
./out/ret_ijmp
echo [-] TTE_OOO-BTB
./out/ooo_ijmp
