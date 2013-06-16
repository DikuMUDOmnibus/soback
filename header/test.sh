#!/bin/csh

set tmp = /tmp/chext.tmp.out

foreach f( *.h )
        echo $f > $tmp
        set out = `sed -e s/\.h\$/\.hx/g < $tmp`
        echo "$f ===> $out"
		sed -f sed.in < $f > $out
end
