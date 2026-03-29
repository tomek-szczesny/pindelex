#!/bin/bash

# Exit if la is not installed
for prog in la_strip la_dedup la_bounds; do
    if ! command -v "$prog" &> /dev/null; then
        echo "Error: la is not installed (missing $prog)" >&2
        exit 1
    fi
done

# Preparing a temporary file and output file name
TEMP1=$(mktemp)
dirbase="$(dirname -- "$1")"
name="$(basename -- "$1")"
if [[ "$name" == *.* ]]; then
  ext="${name##*.}"                # text after last dot
  base="${name%.*}"                # everything before last dot
  outname="${base}.gcode"
else
  outname="${name}.processed"
fi
outfile="${dirbase}/${outname}"


# The processing chain of commands
cat $1 | la_svg2g -s 1000 -f 220 | la_rot -minx | la_pos -x 70 -y 70 | la_dup -n 3 | la_opt  > $TEMP1

# Header
echo "; Processed with LA tools" > $outfile
echo "; GRBL device profile, absolute coords" >> $outfile
cat $TEMP1 | la_bounds >> $outfile
cat >> $outfile<< EOF
G00         ; Rapid Travel
G17         ; XY plane
G40         ; Cancel Cutter compensation (?)
G21         ; Set units to mm
G54         ; Work Offset
G90         ; Absolute positioning
M4
M8
EOF

cat $TEMP1 >> $outfile

# Footer
cat >> $outfile<< EOF
G90
M9
G1S0
M5
G90
G0 X0 Y0
M2

EOF

cat $outfile | la_g2pdf -a -f > ${base}.preview.pdf

rm $TEMP1
