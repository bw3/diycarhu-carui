#!/bin/sh
QRC=./nav_icons.qrc
echo '<!DOCTYPE RCC>' > $QRC
echo '<RCC version="1.0">' >> $QRC
echo '  <qresource>' >> $QRC

# for each files/folder in the folder "theFokderName"
for a in $(find nav_icons/ -d)
do
    # if this is not a folder
    if [ ! -d "$a" ]; then
        echo '      <file>'$a'</file>' >> $QRC
    fi
done

echo '  </qresource>' >> $QRC
echo '</RCC>' >> $QRC
