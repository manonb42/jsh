echo "---------- TEST [2>] ----------"
./redirectionner 'cat tata' '2>' error
echo "result :" && cat error && rm error
#################################
./redirectionner pwd '2>' error
echo "result :" && cat error
#################################
./redirectionner cat '2>' error
echo "result :" && cat error
# echo "------------------------------"

echo "---------- TEST [2>|] ----------"
./redirectionner 'cat tata' '2>|' error
echo "result :" && cat error
#################################
./redirectionner pwd '2>|' error
echo "result :" && cat error && rm error
#################################
# ./redirectionner cat '2>|' error && rm error
echo "------------------------------"

echo "---------- TEST [2>>] ----------"
./redirectionner baba '2>>' error
echo "result :" && cat error
#################################
./redirectionner pwd '2>>' error
echo "result :" && cat error
#################################
# ./redirectionner cat '2>>' error
# echo "result :" && cat error
