rm -rf submission
mkdir submission
cp build/tgnews submission
cp -r src meta models submission/
touch submission/deb-packages.txt

rm -rf submission/src/bigartm/build
rm -rf submission/src/bigartm/python/build
rm -rf submission/src/lexbor/build
rm -rf submission/src/cld3/build
rm -rf submission/src/json/doc
rm -rf submission/meta/Lemmas.txt
rm -rf submission/models/{ru,en}/dictionary.txt

