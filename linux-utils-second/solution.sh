#!/usr/bin/env bash

set -e

test out/
mkdir out/
cd out/
whoami > me.txt
cp me.txt metoo.txt
man wc > wchelp.txt
cat wchelp.txt
wc -l wchelp.txt | cut -d' ' -f1 > wchelp-lines.txt
tac wchelp.txt > wchelp-reversed.txt
cat wchelp.txt wchelp-reversed.txt me.txt metoo.txt wchelp-lines.txt > all.txt
tar -cf result.tar *.txt
gzip result.tar
rm *.txt
cd ..
test -f result.tar.gz && rm result.tar.gz
mv out/result.tar.gz .
rmdir out/