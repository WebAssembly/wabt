# https://stackoverflow.com/a/47801116
file(READ ${in} content)
set(content "const char* ${symbol} = R\"w2c_template(${content})w2c_template\";")
file(WRITE ${out} "${content}")
