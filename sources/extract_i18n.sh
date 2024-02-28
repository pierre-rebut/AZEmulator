#!/bin/bash

grep -R 'I18N::Get("' | sed -n 's/.*I18N::Get("//p' | cut -d '"' -f 1 | sort | uniq > tmp.txt
cat tmp.txt | while read line;  do grep $line ../appData/i18n/fr_FR.yml > /dev/null || echo "$line:" ;  done
rm -f tmp.txt
