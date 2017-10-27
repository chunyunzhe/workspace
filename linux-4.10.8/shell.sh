rm cscope* tags
find `pwd` -name '*.h' -o -name '*.c' -o -name '*.cpp' > cscope.files
cscope -Rbkq
ctags -R
