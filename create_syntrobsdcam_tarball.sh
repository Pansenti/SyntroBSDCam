#!/bin/sh

if [ -f Makefile ]; then
        make clean
fi

rm -f *.gz *.log* *.ini
rm -f Makefile* *.so*
rm -rf release debug Output GeneratedFiles

tar czf SyntroBSDCam.tar.gz * 


echo "Created: SyntroBSDCam.tar.gz"
echo -n "Size: "
ls -l SyntroBSDCam.tar.gz | awk '{ print $5 }'

