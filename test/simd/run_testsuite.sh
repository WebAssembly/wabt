

mkdir -p errors
errcount=0

echo "[X] Checking for Error files"


##
# NOTE: These .wast files contain positive and negative
# examples, so we may encounter errors that are caused 
# from expected malformed SIMD files. In these cases, 
# no C files are generated, so we can just skip those
# errors. 
for f in ../../third_party/testsuite/simd_*.wast; do
    echo "Running $f"
    mkdir -p curtest/
    ../../build/wast2json $f -o curtest/test.json
    
    for t in curtest/*.wasm; do 
        #echo $t
        ../../build/wasm2c $t -o curtest/test.c &> /dev/null
        if [[ $? -eq 0 ]]; then 
            echo "[$t] All good"
            rm curtest/test.c 
            rm curtest/test.h
        else 
            if [[ -f "curtest/test.c" ]]; then
                echo "Error with C file generation for $t"
                echo $f >> errors/error.log
                echo $t >> errors/error.log
                echo err$errcount.wasm >> errors/error.log
                mv $t errors/err$errcount.wasm 
                errcount=$((errcount+1))

                rm curtest/test.c 
                rm curtest/test.h
            else 
                echo "[$t] Skippable error"
            fi
        fi
        #read -p "Press enter to continue"
    done 
    rm -rf curtest/*
done

echo "[X] Running Error Files"

for e in errors/*.wasm; do 
    echo $e
    ../../build/wasm2c $e -o $e.c &> $e.log
done