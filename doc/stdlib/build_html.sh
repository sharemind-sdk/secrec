doxygen Doxyfile
for file in $(grep -il "." html/*.html)
do
	echo "repairing:" $file
	sed '/.*<td class=\"paramtype\">.*<\/td>/ {
		N
		s/\[\[\(.\)\]/\[\[\1\]\]/
		s/\(.*\)\(\]\)\(.*\)\(\[\[.\]\]\)\(.*\)/\1\4\3\5/
	}'<$file >html/tempfile.txt
	mv html/tempfile.txt $file
done
